"""r38 XS momentum lookback grid and a blend with XS funding (live book), real Binance funding paid."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel()
F = load_binance_funding_panel(P)
def run(w):
    return engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
def st(label, r):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    print(f"{label:38s} IS sh {a['sharpe']:5.2f} cagr {a['cagr']*100:5.1f}% dd {a['max_dd']*100:5.1f}% | OOS sh {b['sharpe']:5.2f} cagr {b['cagr']*100:5.1f}% dd {b['max_dd']*100:5.1f}%", flush=True)
rets = {}
for lb in (7, 14, 21, 30, 60):
    for reb in (7,):
        w = S.xs_momentum(P, lookback_d=lb, rebalance_d=reb)
        rets[f"mom{lb}"] = r = run(w); st(f"XS mom {lb}d / weekly", r)
# staggered weekly rebalance (7 offsets averaged) for 14d to reduce timing luck
w14 = sum(S.xs_momentum(P, lookback_d=14, rebalance_d=7, offset_d=o).ffill().fillna(0) for o in range(7)) / 7
rets["mom14s"] = r = run(w14); st("XS mom 14d weekly, 7 staggered offsets", r)
wf = S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24)
rets["fund"] = rf = run(wf); st("XS funding (lag 48h, live-like)", rf)
D = pd.DataFrame(rets).resample("D").sum()
print("daily corr with XS funding:", D.corr()["fund"].round(2).to_dict())
for k in ("mom14", "mom14s", "mom30"):
    st(f"50/50 funding + {k}", 0.5 * rf + 0.5 * rets[k])
