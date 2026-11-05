"""r45 trend majors L/S: once-daily signal (live, 00 UTC) vs re-evaluating the daily-bar signal every 6h / every hour,
vs averaging the 4 six-hour-offset books (staggered). Costs: 0.045% + 0.05% slip, min_trade 0.02, real funding."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
P = load_panel()[MAJORS]; F = load_binance_funding_panel(load_panel())[MAJORS].fillna(0.0001 / 8)
def W(h): return S.trend_breakout(P, long_only=False, port_vol_target=0.25, hour=h)
def latest_of(hours):
    out = None
    for h in hours:
        w = W(h)
        out = w if out is None else out.combine_first(w)
    return out
def st(label, w, mt=0.02):
    res = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=mt)
    r = res.returns; a, b = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":])
    to = res.turnover["2024":].sum() / (len(r["2024":]) / 8760)
    print(f"{label:34s} IS sh {a['sharpe']:5.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} dd {b['max_dd']*100:4.0f}% | OOS turnover {to:5.1f}x/yr", flush=True)
st("daily @00 UTC (live)", W(0))
st("re-evaluate every 6h", latest_of([0, 6, 12, 18]))
st("re-evaluate every hour", latest_of(range(24)))
st("re-evaluate every hour, min_trade 0.05", latest_of(range(24)), mt=0.05)
stag = sum(W(h).ffill().fillna(0) for h in (0, 6, 12, 18)) / 4
st("average of 4 offset books (stagger)", stag.where(stag.diff().abs().sum(axis=1) > 0))
