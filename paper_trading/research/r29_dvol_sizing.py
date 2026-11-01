"""Trend majors: scale exposure by implied vol (Deribit DVOL) relative to its 1y median (forward-looking vol target)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
from ptrade.data import deribit
P = load_panel()[MAJORS]; P = P[P.index >= "2021-03-24"]
F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
dv = deribit.dvol_history("BTC")["close"].reindex(P.index).ffill()
base = S.trend_breakout(P, long_only=False, port_vol_target=0.25)
def show(label, w):
    res = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.02)
    r = res.returns
    a, b = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":])
    print(f"{label:38s} 2021-23 sh {a['sharpe']:.2f} dd {a['max_dd']*100:5.1f}% | 2024-26 sh {b['sharpe']:.2f} cagr {b['cagr']*100:5.1f}% dd {b['max_dd']*100:5.1f}%")
show("baseline (realised-vol target)", base)
for pw in (0.5, 1.0):
    scale = (dv.rolling(24 * 365, min_periods=24 * 90).median() / dv) ** pw
    scale = scale.clip(0.4, 1.6)
    show(f"x (median DVOL / DVOL)^{pw}", base.mul(scale, axis=0))
