"""Live strategy adapters. Each strategy owns one paper Book and decides targets every cycle.

`full` cycles run hourly (right after the hourly bar closes); `fast` cycles run every few minutes and only
event-driven strategies (e.g. crash rebound) act in them.
"""
from __future__ import annotations

import json
import math

import numpy as np
import pandas as pd

from ..config import STATE
from ..data import hyperliquid as hl
from ..data.http import get
from ..strategies import signals as S
from ..universe import HL_MULT, MAJORS, UNIVERSE
from .book import Book, coin_of

MIN_ORDER_USD = 10.0


# ------------------------------------------------------------------------------------------- helpers

def rebalance(book: Book, mkt, targets: dict[str, float], now, reason: str, buffer: float = 0.0,
              close_others: bool = True) -> int:
    """Trade `book` towards target weights (fraction of equity). With close_others=True, held keys missing
    from `targets` are closed; otherwise only the keys in `targets` are touched."""
    keys = set(targets) | (set(book.positions) if close_others else set())
    prices = mkt.prices(set(book.positions) | set(targets))
    eq = book.equity(prices)
    if eq <= 0:
        return 0
    cur = book.weights(prices)
    n = 0
    for key in sorted(keys, key=lambda k: targets.get(k, 0.0) - cur.get(k, 0.0)):  # sells first
        if key not in prices:
            continue
        tgt = float(targets.get(key, 0.0))
        if not math.isfinite(tgt):
            continue
        c = cur.get(key, 0.0)
        dw = tgt - c
        closing = tgt == 0 and key in book.positions
        if abs(dw) <= buffer and not closing:
            continue
        notional = abs(dw) * eq
        if notional < MIN_ORDER_USD and not closing:
            continue
        px = prices[key]
        dqty = -book.positions[key]["qty"] if closing else dw * eq / px
        side = "buy" if dqty > 0 else "sell"
        slip = mkt.slippage(key, notional, side)
        book.trade(key, dqty, px, slip, mkt.fee(key), now, reason)
        n += 1
    return n


def binance_to_hl_weights(row: pd.Series) -> dict[str, float]:
    out = {}
    for sym, w in row.items():
        if sym in UNIVERSE and pd.notna(w):
            out[UNIVERSE[sym]] = float(w)
    return out


def hl_funding_panel(coins: list[str], hours: int = 24 * 8) -> pd.DataFrame:
    start = int((pd.Timestamp.now(tz="UTC") - pd.Timedelta(hours=hours)).timestamp() * 1000)
    cols = {}
    for c in coins:
        try:
            df = hl.funding_history(c, start)
            if len(df):
                cols[c] = df["funding"]
        except Exception as e:  # noqa: BLE001
            print(f"[funding panel] {c}: {e}")
    return pd.DataFrame(cols).sort_index()


# ------------------------------------------------------------------------------------------- base

class Strategy:
    name = "base"
    capital = 100_000.0
    description = ""
    fast = False  # acts in fast cycles

    def book(self, now) -> Book:
        return Book.load_or_create(self.name, self.capital, now, self.description)

    def run(self, book: Book, mkt, data, now, fast: bool):  # pragma: no cover
        raise NotImplementedError


class DailySignal(Strategy):
    """Rebalance once per new daily signal row (computed from the Binance hourly panel)."""
    symbols: list[str] | None = None
    buffer = 0.02

    def weights(self, P: pd.DataFrame) -> pd.DataFrame:  # pragma: no cover
        raise NotImplementedError

    paused = False
    pause_reason = ""

    def run(self, book, mkt, data, now, fast):
        if fast:
            return
        if self.paused:
            if book.positions:
                n = rebalance(book, mkt, {}, now, "retired: " + self.pause_reason[:80])
                print(f"[{self.name}] retired -> closed {n} positions")
            book.meta["paused"] = self.pause_reason
            return
        P = data.panel if self.symbols is None else data.panel[self.symbols]
        w = self.weights(P).dropna(how="all")
        if not len(w):
            return
        ts = w.index[-1]
        if book.meta.get("last_signal") == ts.isoformat():
            return
        targets = binance_to_hl_weights(w.iloc[-1])
        targets = {k: v for k, v in targets.items() if mkt.has(k)}
        n = rebalance(book, mkt, targets, now, f"signal {ts:%Y-%m-%d %H}h", buffer=self.buffer)
        book.meta["last_signal"] = ts.isoformat()
        book.meta["last_targets"] = {k: round(v, 4) for k, v in targets.items() if abs(v) > 1e-4}
        print(f"[{self.name}] new signal {ts}: {n} trades")


# ------------------------------------------------------------------------------------------- strategies

class TrendBreakoutMajors(DailySignal):
    name = "trend_breakout_majors"
    description = ("Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily "
                   "(cut from 40% on 2026-10-31 to balance risk across books, r15).")
    symbols = MAJORS

    def weights(self, P):
        return S.trend_breakout(P, long_only=False, port_vol_target=0.25)


class TrendBreakoutAltsLO(DailySignal):
    name = "trend_breakout_universe_lo"
    description = ("Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions "
                   "scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD).")

    def weights(self, P):
        w = S.trend_breakout(P, long_only=True, port_vol_target=0.4)
        return w * S.efficiency_gate(P, 30, 0.1, 0.3).reindex(columns=w.columns).where(w.notna())


class TrendEWMACMajorsLO(DailySignal):
    name = "trend_ewmac_majors_lo"
    description = "RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily."
    symbols = MAJORS
    paused = True
    pause_reason = ("RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend "
                    "books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41.")

    def weights(self, P):
        return S.trend_ewmac(P, long_only=True, port_vol_target=0.3)


class XSFunding(Strategy):
    name = "xs_funding"
    description = ("Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), "
                   "inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index "
                   "archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: "
                   "Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag.")
    paused = False
    pause_reason = ""
    max_staleness = pd.Timedelta(days=4)

    def _binance_funding_7d(self, now) -> tuple[pd.Series, pd.Timestamp | None]:
        from ..data import funding_recon
        start = (now - pd.Timedelta(days=45)).strftime("%Y-%m")
        sums, last = {}, []
        for sym in UNIVERSE:
            nm = "1000PEPEUSDT" if sym == "PEPEUSDT" else sym
            try:
                r = funding_recon.build(nm, start=start, workers=4)
            except Exception as e:  # noqa: BLE001
                print(f"[{self.name}] recon failed {nm}: {e}")
                continue
            if not len(r):
                continue
            t_last = r.index[-1]
            if now - t_last > self.max_staleness:
                continue
            sums[sym] = float(r[r.index > t_last - pd.Timedelta(hours=168)].sum())
            last.append(t_last)
        return pd.Series(sums), (min(last) if last else None)

    def run(self, book, mkt, data, now, fast):
        if fast:
            return
        if self.paused:
            if book.positions:
                n = rebalance(book, mkt, {}, now, "paused: live funding signal failed validation")
                print(f"[{self.name}] paused -> closed {n} positions")
            book.meta["paused"] = self.pause_reason
            return
        book.meta.pop("paused", None)
        day = now.strftime("%Y-%m-%d")
        if book.meta.get("last_day") == day:
            return
        trail, asof = self._binance_funding_7d(now)
        P = data.panel.iloc[-24 * 45:]
        trail = trail[[c for c in trail.index if c in P.columns and pd.notna(P[c].iloc[-1])]]
        if len(trail) < 20:
            print(f"[{self.name}] only {len(trail)} coins with fresh funding data -> holding positions")
            book.meta["warning"] = f"{now.isoformat()} stale/insufficient funding data ({len(trail)} coins)"
            return
        vol = P.pct_change(fill_method=None).iloc[-24 * 30:].std()
        k = max(1, int(round(len(trail) * 0.25)))
        r = trail.sort_values()
        longs, shorts = r.index[:k], r.index[-k:]
        iv = 1 / vol
        w = pd.Series(0.0, index=P.columns)
        w[longs] = iv[longs] / iv[longs].sum() * 0.5
        w[shorts] = -iv[shorts] / iv[shorts].sum() * 0.5
        targets = {k_: v for k_, v in binance_to_hl_weights(w).items() if mkt.has(k_)}
        n = rebalance(book, mkt, targets, now, "daily xs-funding (Binance recon)", buffer=0.01)
        book.meta.pop("warning", None)
        book.meta["last_day"] = day
        book.meta["signal_asof"] = str(asof)
        book.meta["last_targets"] = {k_: round(v, 4) for k_, v in targets.items() if abs(v) > 1e-4}
        book.meta["trail_funding_apr"] = {UNIVERSE[s]: round(float(v) * 365 / 7 * 100, 1) for s, v in r.items()}
        print(f"[{self.name}] rebalanced: {n} trades; asof {asof}; longs={[UNIVERSE[x] for x in longs]} "
              f"shorts={[UNIVERSE[x] for x in shorts]}")


class DeltaNeutralCarry(Strategy):
    name = "dn_funding_carry"
    description = ("Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d "
                   "funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): "
                   "13.7% CAGR, 1.4% vol, max DD -1.4%.")
    entry, exit_, top_k, max_per, max_tot = 0.15, 0.05, 6, 0.15, 0.90

    def run(self, book, mkt, data, now, fast):
        if fast:
            return
        day = now.strftime("%Y-%m-%d")
        if book.meta.get("last_day") == day:
            return
        cands = sorted(set(UNIVERSE.values()) | {"HYPE"})
        cands = [c for c in cands if mkt.has(c)]
        F = hl_funding_panel(cands, hours=24 * 7)
        apr = (F.iloc[-168:].mean() * 8760).dropna()
        held = {coin_of(k) for k in book.positions}
        keep = [c for c in apr.index if (c in held and apr[c] > self.exit_) or (c not in held and apr[c] > self.entry)]
        keep = sorted(keep, key=lambda c: -apr[c])[: self.top_k]
        n_each = min(self.max_per, self.max_tot / len(keep)) if keep else 0.0
        targets = {}
        for c in keep:
            targets[c] = -n_each
            targets[c + "/spot"] = n_each
        for k in book.positions:
            targets.setdefault(k, 0.0)
        n = rebalance(book, mkt, targets, now, "daily dn-carry", buffer=0.02)
        book.meta["last_day"] = day
        book.meta["holdings_apr"] = {c: round(float(apr[c]) * 100, 1) for c in keep}
        book.meta["top_apr"] = {c: round(float(v) * 100, 1) for c, v in apr.sort_values(ascending=False).head(10).items()}
        print(f"[{self.name}] {n} trades; holding {keep}")


class CrashRebound(Strategy):
    name = "crash_rebound"
    description = ("Buy coins after a >10% drop over a rolling 4h window (liquidation-cascade overshoot), "
                   "15% per position, max gross 100%, hold 24h (re-trigger extends). Checked every few minutes.")
    fast = True
    thr, hold_h, per_w, max_gross = -0.10, 24, 0.15, 1.0

    def _rolling_4h(self) -> dict[str, float]:
        syms = list(UNIVERSE)
        out = {}
        for i in range(0, len(syms), 50):
            chunk = syms[i:i + 50]
            r = get("https://data-api.binance.vision/api/v3/ticker",
                    params={"symbols": json.dumps(chunk, separators=(",", ":")), "windowSize": "4h", "type": "MINI"})
            for d in r.json():
                o, last = float(d["openPrice"]), float(d["lastPrice"])
                if o > 0:
                    out[d["symbol"]] = last / o - 1
        return out

    def _missed_events(self, data, since: pd.Timestamp, now: pd.Timestamp) -> dict:
        """4h drops at hourly closes since the last scan (catch-up when scans were missed, e.g. container asleep)."""
        if data is None or getattr(data, "panel", None) is None:
            return {}
        P = data.panel.iloc[-36:]
        r4 = P / P.shift(4) - 1
        out = {}
        for t, row in r4.iterrows():
            close_t = t + pd.Timedelta(hours=1)
            if close_t <= since or close_t <= now - pd.Timedelta(hours=self.hold_h):
                continue
            btc = float(row.get("BTCUSDT", float("nan")))
            for sym, v in row.items():
                if pd.notna(v) and v < self.thr:
                    out[sym] = (float(v), close_t, btc)
        return out

    def _accept(self, btc_4h: float) -> bool:
        """Event filter hook (base strategy accepts every crash)."""
        return True

    def run(self, book, mkt, data, now, fast):
        exits = book.meta.setdefault("exit_at", {})
        prev_scan = pd.Timestamp(book.meta.get("last_scan") or (now - pd.Timedelta(hours=6)).isoformat())
        # 1) exits
        for coin, ts in list(exits.items()):
            if pd.Timestamp(ts) <= now:
                if coin in book.positions:
                    rebalance(book, mkt, {coin: 0.0}, now, "crash-rebound exit (24h)", close_others=False)
                exits.pop(coin, None)
        # 2) events: real-time rolling 4h window + catch-up on hourly closes since the previous scan
        try:
            chg = self._rolling_4h()
        except Exception as e:  # noqa: BLE001
            print(f"[{self.name}] ticker failed: {e}")
            chg = {}
        btc_now = chg.get("BTCUSDT", float("nan"))
        events = {sym: (r, now, btc_now) for sym, r in chg.items() if r < self.thr}
        if (now - prev_scan) > pd.Timedelta(minutes=10):
            for sym, ev in self._missed_events(data, prev_scan, now).items():
                events.setdefault(sym, ev)
        events = {k: v for k, v in events.items() if self._accept(v[2])}
        book.meta["last_scan"] = now.isoformat()
        if chg:
            book.meta["worst_4h"] = {UNIVERSE[s]: round(v * 100, 2) for s, v in sorted(chg.items(), key=lambda x: x[1])[:5]}
        prices = mkt.prices(set(book.positions))
        gross, _ = book.exposure(prices)
        for sym, (r, t_ev, _btc) in sorted(events.items(), key=lambda x: x[1][0]):
            coin = UNIVERSE[sym]
            if not mkt.has(coin):
                continue
            exit_t = t_ev + pd.Timedelta(hours=self.hold_h)
            if coin in exits:
                exit_t = max(exit_t, pd.Timestamp(exits[coin]))
            if exit_t - now < pd.Timedelta(hours=12) and coin not in book.positions:
                continue  # too late to enter
            exits[coin] = exit_t.isoformat()
            if coin in book.positions:
                continue
            w = min(self.per_w, self.max_gross - gross)
            if w < 0.02:
                continue
            late = "" if t_ev == now else f" (late, event {t_ev:%H:%M})"
            rebalance(book, mkt, {coin: w}, now, f"crash-rebound entry 4h={r*100:.1f}%{late}", close_others=False)
            gross += w
            print(f"[{self.name}] ENTER {coin} after {r*100:.1f}% 4h drop{late}")


class CrashReboundMarketWide(CrashRebound):
    name = "crash_rebound_mkt"
    description = ("Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the "
                   "same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS "
                   "Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.")
    btc_thr = -0.03

    def _accept(self, btc_4h: float) -> bool:
        return btc_4h == btc_4h and btc_4h < self.btc_thr  # NaN-safe


class BTCHold(Strategy):
    name = "benchmark_btc_hold"
    description = "Benchmark: 100% BTC spot buy & hold (priced at HL oracle)."

    def run(self, book, mkt, data, now, fast):
        if fast or book.positions:
            return
        rebalance(book, mkt, {"BTC/spot": 1.0}, now, "benchmark buy")


class Discretionary(Strategy):
    name = "claude_discretionary"
    description = ("Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing "
                   "news, funding, positioning and charts. Measures whether judgment adds value.")
    capital = 100_000.0

    def run(self, book, mkt, data, now, fast):
        if fast:
            return
        p = STATE / "discretionary.json"
        if not p.exists():
            return
        cfg = json.loads(p.read_text())
        ver = cfg.get("updated")
        if book.meta.get("applied") == ver:
            return
        targets = {k: float(v) for k, v in cfg.get("targets", {}).items() if mkt.has(coin_of(k))}
        for k in book.positions:
            targets.setdefault(k, 0.0)
        n = rebalance(book, mkt, targets, now, "discretionary: " + cfg.get("rationale", "")[:120], buffer=0.005)
        book.meta["applied"] = ver
        book.meta["rationale"] = cfg.get("rationale", "")
        print(f"[{self.name}] applied targets version {ver}: {n} trades")


def all_strategies() -> list[Strategy]:
    return [TrendBreakoutMajors(), TrendBreakoutAltsLO(), TrendEWMACMajorsLO(), XSFunding(), DeltaNeutralCarry(),
            CrashRebound(), CrashReboundMarketWide(), Discretionary(), BTCHold()]


_ = (np, HL_MULT)  # kept for future use
