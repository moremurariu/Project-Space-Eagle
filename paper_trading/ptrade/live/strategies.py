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
                   "Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: "
                   "Sharpe IS 1.31→1.37, OOS 1.46→1.95).")
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

    @staticmethod
    def _beta_neutral(w: pd.Series, P: pd.DataFrame, cap: float = 2.0) -> pd.Series:
        """r44: scale the long/short legs so sum(w*beta)=0 (beta = 60d hourly beta to the equal-weight market),
        keeping gross 1. Dollar-neutral alone left the book long alt beta (low-funding coins are often high-beta)."""
        rh = P.pct_change(fill_method=None).iloc[-24 * 60:]
        mk = rh.mean(axis=1)
        beta = (rh.apply(lambda c: c.cov(mk)) / mk.var()).reindex(w.index).fillna(1.0).clip(0.2, 3.0)
        L, S_ = w.clip(lower=0), w.clip(upper=0)
        bl, bs = float((L * beta).sum()), float((-S_ * beta).sum())
        if bl <= 0 or bs <= 0:
            return w
        kl, ks = min(bs / (bl + bs) * 2, cap), min(bl / (bl + bs) * 2, cap)
        y = L * kl + S_ * ks
        return y / y.abs().sum()

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
        P = data.panel.iloc[-24 * 65:]
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
        w = self._beta_neutral(w, P)
        targets = {k_: v for k_, v in binance_to_hl_weights(w).items() if mkt.has(k_)}
        n = rebalance(book, mkt, targets, now, "daily xs-funding (Binance recon)", buffer=0.01)
        book.meta.pop("warning", None)
        book.meta["last_day"] = day
        book.meta["signal_asof"] = str(asof)
        book.meta["last_targets"] = {k_: round(v, 4) for k_, v in targets.items() if abs(v) > 1e-4}
        book.meta["trail_funding_apr"] = {UNIVERSE[s]: round(float(v) * 365 / 7 * 100, 1) for s, v in r.items()}
        print(f"[{self.name}] rebalanced: {n} trades; asof {asof}; longs={[UNIVERSE[x] for x in longs]} "
              f"shorts={[UNIVERSE[x] for x in shorts]}")


class XSMomentum(DailySignal):
    name = "xs_momentum"
    description = ("Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile "
                   "by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, "
                   "weekly rebalance staggered over "
                   "7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with "
                   "xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.")
    lookbacks = (7, 14, 21)
    buffer = 0.01

    def weights(self, P):
        P = P.iloc[-24 * 150:]
        grid = P.index[P.index.hour == 23]        # daily rows live on the 23:00 bar (close = 00 UTC)
        ws = [S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).reindex(grid).ffill().fillna(0.0)
              for lb in self.lookbacks for o in range(7)]
        return sum(ws) / len(ws)


class XSBAB(DailySignal):
    name = "xs_bab"
    description = ("EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest "
                   "quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero "
                   "realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, "
                   "robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if "
                   "live Sharpe < 0 after 3 months.")
    buffer = 0.01

    def weights(self, P):
        P = P.iloc[-24 * 150:]
        grid = P.index[P.index.hour == 23]
        ws = [S.xs_bab(P, rebalance_d=7, offset_d=o).reindex(grid).ffill().fillna(0.0) for o in range(7)]
        return sum(ws) / len(ws)


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
    description = ("Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade "
                   "overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly "
                   "close within the next 12h is a further 5% below the signal close; each tranche held 24h "
                   "(re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling "
                   "knives (MaxDD −60% vs −28%).")
    fast = False
    thr, hold_h, max_gross = -0.10, 24, 1.0
    per_w, add_w, add_drop, add_win_h = 0.075, 0.075, 0.05, 12   # r30 scale-in

    def _bars(self, data, since: pd.Timestamp, now: pd.Timestamp):
        """Closed hourly bars (Binance spot panel) with close time in (since, now], within the hold window."""
        if data is None or getattr(data, "panel", None) is None:
            return None, None, []
        P = data.panel.iloc[-40:]
        r4 = P / P.shift(4) - 1
        bars = []
        for t in P.index:
            close_t = t + pd.Timedelta(hours=1)
            if since < close_t <= now and close_t > now - pd.Timedelta(hours=self.hold_h):
                bars.append((t, close_t))
        return P, r4, bars

    def _accept(self, btc_4h: float) -> bool:
        """Event filter hook (base strategy accepts every crash)."""
        return True

    def _buy(self, book, mkt, coin, w, now, reason) -> float:
        before = book.positions.get(coin, {}).get("qty", 0.0)
        cur = book.weights(mkt.prices(set(book.positions) | {coin})).get(coin, 0.0)
        rebalance(book, mkt, {coin: cur + w}, now, reason, close_others=False)
        return book.positions.get(coin, {}).get("qty", 0.0) - before

    def run(self, book, mkt, data, now, fast):
        tr = book.meta.setdefault("tranches", {})   # coin -> {"t1"|"t2": {"qty", "exit"}}
        watch = book.meta.setdefault("watch", {})   # coin -> {"sym", "ref", "until"}: scale-in window
        book.meta.pop("exit_at", None)              # legacy (pre-r30) field
        # 1) exits, per tranche
        for coin, d in list(tr.items()):
            for k, t in sorted(d.items(), key=lambda x: x[1]["exit"]):
                if pd.Timestamp(t["exit"]) > now:
                    continue
                pos = book.positions.get(coin)
                if pos:
                    last = len(d) == 1
                    q = pos["qty"] if last else min(t["qty"], pos["qty"])
                    px = mkt.price(coin)
                    book.trade(coin, -q, px, mkt.slippage(coin, abs(q) * px, "sell"), mkt.fee(coin), now,
                               f"crash-rebound exit {k} (24h)")
                d.pop(k)
            if not d or coin not in book.positions:
                tr.pop(coin, None)
        for coin, w_ in list(watch.items()):
            if pd.Timestamp(w_["until"]) <= now - pd.Timedelta(hours=1):
                watch.pop(coin)
        if fast:
            return
        # 2) closed hourly bars since the previous scan, in time order (catch-up if the container slept)
        prev_scan = pd.Timestamp(book.meta.get("last_scan") or (now - pd.Timedelta(hours=6)).isoformat())
        P, r4, bars = self._bars(data, prev_scan, now)
        book.meta["last_scan"] = now.isoformat()
        if P is None:
            return
        chg = (P.iloc[-1] / P.iloc[-5] - 1).dropna().to_dict()
        if chg:
            book.meta["worst_4h"] = {UNIVERSE[s]: round(v * 100, 2) for s, v in sorted(chg.items(), key=lambda x: x[1])[:5]}
        gross, _ = book.exposure(mkt.prices(set(book.positions)))
        for t, close_t in bars:
            row = r4.loc[t]
            btc = float(row.get("BTCUSDT", float("nan")))
            late = "" if close_t >= now - pd.Timedelta(minutes=59) else f" (late, bar {close_t:%H:%M})"
            sigs = {s for s, v in row.items() if pd.notna(v) and v < self.thr and self._accept(btc)}
            # 2a) tranche 1 on new crash signals (re-trigger extends the hold and resets the scale-in reference)
            for sym in sorted(sigs, key=lambda s: row[s]):
                coin = UNIVERSE[sym]
                if not mkt.has(coin):
                    continue
                exit_t = close_t + pd.Timedelta(hours=self.hold_h)
                watch[coin] = {"sym": sym, "ref": float(P.loc[t, sym]),
                               "until": (close_t + pd.Timedelta(hours=self.add_win_h)).isoformat()}
                d = tr.setdefault(coin, {})
                if "t1" in d:
                    d["t1"]["exit"] = max(pd.Timestamp(d["t1"]["exit"]), exit_t).isoformat()
                    continue
                w = min(self.per_w, self.max_gross - gross)
                if exit_t - now < pd.Timedelta(hours=12) or w < 0.02:
                    if not d:
                        tr.pop(coin)
                    continue  # too late / no room
                q = self._buy(book, mkt, coin, w, now, f"crash-rebound t1 4h={row[sym]*100:.1f}%{late}")
                if q > 0:
                    d["t1"] = {"qty": q, "exit": exit_t.isoformat()}
                    gross += w
                    print(f"[{self.name}] ENTER t1 {coin} after {row[sym]*100:.1f}% 4h drop{late}")
                elif not d:
                    tr.pop(coin)
            # 2b) tranche 2: a further drop below the signal close within the scale-in window
            for coin, w_ in list(watch.items()):
                sym = w_["sym"]
                if sym in sigs or not (pd.Timestamp(w_["until"]) >= close_t) or sym not in P:
                    continue
                if pd.Timestamp(w_["until"]) - pd.Timedelta(hours=self.add_win_h) >= close_t:
                    continue  # bar not after the signal
                px = float(P.loc[t, sym])
                if not (px < w_["ref"] * (1 - self.add_drop)):
                    continue
                exit_t = close_t + pd.Timedelta(hours=self.hold_h)
                d = tr.setdefault(coin, {})
                if "t2" in d:
                    d["t2"]["exit"] = max(pd.Timestamp(d["t2"]["exit"]), exit_t).isoformat()
                    continue
                w = min(self.add_w, self.max_gross - gross)
                if exit_t - now < pd.Timedelta(hours=12) or w < 0.02:
                    if not d:
                        tr.pop(coin)
                    continue
                q = self._buy(book, mkt, coin, w, now,
                              f"crash-rebound t2 {px / w_['ref'] * 100 - 100:.1f}% below signal close{late}")
                if q > 0:
                    d["t2"] = {"qty": q, "exit": exit_t.isoformat()}
                    gross += w
                    print(f"[{self.name}] ADD t2 {coin} ({px / w_['ref'] * 100 - 100:.1f}% below signal){late}")
                elif not d:
                    tr.pop(coin)


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


class XYZWeekendFade(Strategy):
    name = "xyz_weekend_fade"
    description = ("EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday "
                   "reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, "
                   "short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.")
    NON_EQUITY = {"GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR",
                  "URANIUM", "ALUMINIUM", "DXY", "VIX", "CORN", "WHEAT", "TTF"}
    top_n, min_names, cap = 45, 10, 0.10

    @staticmethod
    def _ny_close(date) -> pd.Timestamp:
        """16:00 New York (DST-aware) on calendar date `date`, in UTC."""
        return pd.Timestamp(date.year, date.month, date.day, 16, tz="America/New_York").tz_convert("UTC")

    def run(self, book, mkt, data, now, fast):
        if fast:
            return
        # exit
        if book.positions:
            exit_at = book.meta.get("exit_at")
            if exit_at and now >= pd.Timestamp(exit_at):
                n = rebalance(book, mkt, {}, now, "weekend fade exit (Mon US close)")
                book.meta.pop("exit_at", None)
                print(f"[{self.name}] exit: {n} trades")
            return
        # entry window: Sunday 23:00 UTC .. Monday 02:59 UTC (catch-up if the container slept)
        wd, h = now.weekday(), now.hour
        if not ((wd == 6 and h == 23) or (wd == 0 and h < 3)):
            return
        sunday = (now - pd.Timedelta(days=0 if wd == 6 else 1)).date()
        if book.meta.get("last_entry_week") == str(sunday):
            return
        book.meta["last_entry_week"] = str(sunday)
        fri_close = self._ny_close(sunday - pd.Timedelta(days=2))
        meta = hl.meta_and_ctxs("xyz")
        meta = meta[~meta["delisted"]].sort_values("oi_usd", ascending=False).head(self.top_n)
        coins = [c for c in meta.index if c.split(":")[1] not in self.NON_EQUITY and mkt.has(c)]
        wk = {}
        for c in coins:
            try:
                cd = hl.candles(c, "1h", start_ms=int((fri_close - pd.Timedelta(hours=3)).timestamp() * 1000))
                bar = fri_close - pd.Timedelta(hours=1)
                if bar in cd.index:
                    wk[c] = mkt.price(c) / float(cd.at[bar, "close"]) - 1
            except Exception as e:  # noqa: BLE001
                print(f"[{self.name}] {c}: {e}")
        if len(wk) < self.min_names:
            book.meta["last_signal"] = f"{now:%Y-%m-%d %H:%M} skipped: only {len(wk)} names"
            return
        x = pd.Series(wk); x = x - x.mean()
        w = (-x / x.abs().sum()).clip(-self.cap, self.cap)       # linear in the (demeaned) weekend move
        w = w / w.abs().sum()                                     # gross 1, ~dollar-neutral
        targets = w[w.abs() > 0.002].to_dict()
        n = rebalance(book, mkt, targets, now, "weekend fade entry")
        mon_close = self._ny_close(sunday + pd.Timedelta(days=1))
        book.meta["exit_at"] = mon_close.isoformat()
        book.meta["fri_close"] = fri_close.isoformat()
        book.meta["last_signal"] = (f"{now:%Y-%m-%d %H:%M}: {n} trades, {len(x)} names, weekend xs-dispersion "
                                    f"{x.std() * 100:.2f}%, exit {mon_close:%a %H:%M} UTC")
        print(f"[{self.name}] {book.meta['last_signal']}")


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
    return [TrendBreakoutMajors(), TrendBreakoutAltsLO(), TrendEWMACMajorsLO(), XSFunding(), XSMomentum(), XSBAB(), DeltaNeutralCarry(),
            CrashRebound(), CrashReboundMarketWide(), XYZWeekendFade(), Discretionary(), BTCHold()]


_ = (np, HL_MULT)  # kept for future use
