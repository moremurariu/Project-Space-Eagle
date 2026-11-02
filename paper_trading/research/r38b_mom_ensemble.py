"""r38b robustness: XS momentum lookback ensemble (staggered weekly), alone and blended with XS funding."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P)
run = lambda w: engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
def st(label, r):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    yr = {str(k)[2:]: round(v * 100) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{label:34s} IS sh {a['sharpe']:5.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} dd {b['max_dd']*100:4.0f}% {yr}", flush=True)
def stag(lb):
    return sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o).ffill().fillna(0) for o in range(7)) / 7
W = {lb: stag(lb) for lb in (7, 14, 21, 30, 60)}
for lb, w in W.items():
    st(f"mom {lb}d staggered", run(w))
ens_all = sum(W.values()) / len(W); ens_short = (W[7] + W[14] + W[21]) / 3
r_all, r_short = run(ens_all), run(ens_short)
st("ensemble 7/14/21/30/60", r_all); st("ensemble 7/14/21", r_short)
rf = run(S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24))
st("XS funding (live-like)", rf)
for lab, r in (("ens all", r_all), ("ens 7/14/21", r_short)):
    for wm in (0.33, 0.5):
        st(f"funding {1-wm:.2f} + {lab} {wm:.2f}", (1 - wm) * rf + wm * r)
