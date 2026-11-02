"""Markdown status report: reports/STATUS.md (regenerated every full cycle)."""
from __future__ import annotations

import json

import numpy as np
import pandas as pd

from ..config import REPORTS
from .book import BOOKS_DIR, EQUITY_CSV, TRADES_CSV, Book


def _load_books() -> list[Book]:
    out = []
    for p in sorted(BOOKS_DIR.glob("*.json")):
        out.append(Book(**json.loads(p.read_text())))
    return out


def book_stats(eq: pd.Series, initial: float) -> dict:
    eq = eq.sort_index()
    last = eq.iloc[-1]
    t_last = eq.index[-1]

    def ago(h):
        s = eq[eq.index <= t_last - pd.Timedelta(hours=h)]
        return s.iloc[-1] if len(s) else (initial if (t_last - eq.index[0]) < pd.Timedelta(hours=h) else np.nan)

    dd = (eq / eq.cummax().clip(lower=initial) - 1).min()
    hourly = eq.resample("h").last().dropna().pct_change().dropna()
    sharpe = hourly.mean() / hourly.std() * np.sqrt(8760) if len(hourly) > 48 and hourly.std() > 0 else np.nan
    return {"equity": last, "ret_total": last / initial - 1, "ret_24h": last / ago(24) - 1,
            "ret_7d": last / ago(24 * 7) - 1, "max_dd": min(dd, 0.0), "sharpe": sharpe,
            "days": (t_last - eq.index[0]).total_seconds() / 86400}


def pct(x, d=2):
    return "n/a" if x is None or (isinstance(x, float) and not np.isfinite(x)) else f"{x*100:+.{d}f}%"


def write_report(mkt, now: pd.Timestamp):
    books = _load_books()
    eqdf = pd.read_csv(EQUITY_CSV, parse_dates=["ts"]) if EQUITY_CSV.exists() else pd.DataFrame()
    lines = [f"# Paper trading status — {now:%Y-%m-%d %H:%M} UTC", ""]
    try:
        btc = mkt.mid("BTC")
        eth = mkt.mid("ETH")
        lines.append(f"BTC {btc:,.0f} · ETH {eth:,.1f} · venue: Hyperliquid (paper) · capital per book $100k")
        lines.append("")
    except Exception:  # noqa: BLE001
        pass
    lines += ["| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann.) | Gross | Net | Pos | Fees | Funding |",
              "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|"]
    tot_eq, tot_init = 0.0, 0.0
    for b in books:
        prices = mkt.prices(b.positions)
        e = b.equity(prices)
        g, n = b.exposure(prices)
        st = {}
        if len(eqdf):
            s = eqdf[eqdf.book == b.name].set_index("ts")["equity"]
            if len(s):
                st = book_stats(s, b.initial)
        lines.append(
            f"| {b.name} | ${e:,.0f} | {pct(e / b.initial - 1)} | {pct(st.get('ret_24h'))} | {pct(st.get('ret_7d'))} | "
            f"{pct(st.get('max_dd'))} | {st.get('sharpe', float('nan')):.2f} | {g:.2f} | {n:+.2f} | {len(b.positions)} | "
            f"${b.fees + b.slippage:,.0f} | ${b.funding:,.0f} |")
        if not b.name.startswith("benchmark"):
            tot_eq += e
            tot_init += b.initial
    if tot_init:
        lines += ["", f"**Combined (ex-benchmark): ${tot_eq:,.0f} on ${tot_init:,.0f} → {pct(tot_eq / tot_init - 1)}**", ""]

    lines += ["## Positions", ""]
    for b in books:
        if not b.positions:
            continue
        prices = mkt.prices(b.positions)
        e = b.equity(prices)
        lines += [f"**{b.name}**", "", "| Key | Qty | Avg px | Px | uPnL | Weight |", "|---|---:|---:|---:|---:|---:|"]
        for k, p in sorted(b.positions.items(), key=lambda kv: -abs(kv[1]["qty"] * prices.get(kv[0], kv[1]["avg_px"]))):
            px = prices.get(k, p["avg_px"])
            upnl = p["qty"] * (px - p["avg_px"])
            lines.append(f"| {k} | {p['qty']:.6g} | {p['avg_px']:.6g} | {px:.6g} | ${upnl:,.0f} | {p['qty'] * px / e:+.3f} |")
        lines.append("")

    lines += ["## Strategy notes", ""]
    for b in books:
        m = {k: v for k, v in b.meta.items() if k in ("last_signal", "last_day", "holdings_apr", "worst_4h", "last_error",
                                                     "rationale", "tranches", "watch", "top_apr", "paused")}
        lines.append(f"- **{b.name}** — {b.description}")
        if m:
            lines.append(f"  - `{json.dumps(m, default=str)[:600]}`")
    lines.append("")

    if TRADES_CSV.exists():
        tr = pd.read_csv(TRADES_CSV).tail(20)
        lines += ["## Last 20 trades", "", "| Time | Book | Key | Side | Notional | Fill | Reason |", "|---|---|---|---|---:|---:|---|"]
        for _, r in tr.iloc[::-1].iterrows():
            lines.append(f"| {str(r.ts)[:16]} | {r.book} | {r.key} | {r.side} | ${float(r.notional):,.0f} | {float(r.fill_px):.6g} | {r.reason} |")
    (REPORTS / "STATUS.md").write_text("\n".join(lines) + "\n")
