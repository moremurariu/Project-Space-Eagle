"""Trend books gated by trendiness (Kaufman efficiency ratio) — does it help IS and OOS?"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
P = load_panel(); F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
d = S.to_daily(P)
def er(n):
    num = (d - d.shift(n)).abs(); den = d.diff().abs().rolling(n).sum()
    return (num / den)
def gate(w, n, lo, hi):
    e = er(n)
    g = ((e - lo) / (hi - lo)).clip(0, 1)
    gh = S.daily_to_hourly(g, P.index)
    return w * gh.reindex(columns=w.columns).where(w.notna())
def show(label, w, cols):
    res = engine.run(P[cols], w[cols], fee=0.00045, slippage=0.0005, funding=F[cols], min_trade=0.02)
    r = res.returns
    a, b = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":])
    print(f"{label:45s} IS sh {a['sharpe']:.2f} cagr {a['cagr']*100:5.1f}% dd {a['max_dd']*100:5.1f}% | OOS sh {b['sharpe']:.2f} cagr {b['cagr']*100:5.1f}% dd {b['max_dd']*100:5.1f}%")
for name, cols, lo in [("majors L/S", MAJORS, False), ("universe LO", list(P.columns), True)]:
    base = S.trend_breakout(P[cols], long_only=lo, port_vol_target=0.25 if not lo else 0.4)
    show(f"{name} baseline", base, cols)
    for n, l, h in [(30, 0.1, 0.3), (30, 0.15, 0.4), (60, 0.1, 0.3)]:
        show(f"{name} ER{n} gate [{l},{h}]", gate(base, n, l, h), cols)
