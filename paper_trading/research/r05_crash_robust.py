import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
P = load_panel()
F = pd.DataFrame(0.0001/8, index=P.index, columns=P.columns)
def crash_strategy(P, thr, hold, per_w=0.15, max_gross=1.0, hedge_btc=False, win=4):
    sig = (P/P.shift(win)-1) < thr
    pos = sig.astype(float).rolling(hold, min_periods=1).max().fillna(0)
    w = pos*per_w
    if hedge_btc:
        alt = w.drop(columns="BTCUSDT").sum(axis=1)
        w["BTCUSDT"] = -alt
    g = w.abs().sum(axis=1); w = w.mul((max_gross/g).clip(upper=1).fillna(1), axis=0)
    chg = pos.diff().abs().sum(axis=1) > 0
    return w.where(chg, np.nan)
for thr, hold in [(-0.08,24),(-0.10,24),(-0.08,48)]:
    for lag in (0,1,2):
        for hedge in (False, True):
            w = crash_strategy(P, thr, hold, hedge_btc=hedge)
            res = engine.run(P, w, fee=0.00045, slippage=0.001, funding=F, lag=lag)
            yr = engine.yearly_returns(res.returns)
            st = res.stats()
            print(f"thr={thr} hold={hold} lag={lag} hedge={hedge}: {engine.fmt_stats(st)}")
            print("     yearly:", " ".join(f"{y}:{v*100:+.0f}%" for y,v in yr.items()))
# window sensitivity
for win in (1,2,8,12):
    w = crash_strategy(P, -0.08 if win>2 else -0.05, 24, win=win)
    res = engine.run(P, w, fee=0.00045, slippage=0.001, funding=F, lag=1)
    print(f"win={win}h lag1: {engine.fmt_stats(res.stats())}")
