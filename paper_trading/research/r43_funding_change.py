"""r43 XS on the CHANGE in 7d funding (this week minus last week) vs the level (live xs_funding), lag 48h."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P)
run = lambda w: engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
def st(label, r):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    print(f"{label:40s} IS sh {a['sharpe']:5.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} dd {b['max_dd']*100:4.0f}%", flush=True)
    return r
Fl = F.shift(48)
lvl = st("level (live xs_funding)", run(S.xs_funding_carry(P, Fl, lookback_h=168, rebalance_h=24)))
for k in (72, 168):
    d = Fl - Fl.shift(k)
    st(f"change vs {k}h ago (7d window)", run(S.xs_funding_carry(P, d, lookback_h=168, rebalance_h=24)))
chg = run(S.xs_funding_carry(P, Fl - Fl.shift(168), lookback_h=168, rebalance_h=24))
print("daily corr level vs change:", round(lvl.resample("D").sum().corr(chg.resample("D").sum()), 2))
st("1/2 level + 1/2 change(168h)", 0.5 * lvl + 0.5 * chg)
