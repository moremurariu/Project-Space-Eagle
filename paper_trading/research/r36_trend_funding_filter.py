"""r36 universe long-only trend (live: breakout + ER30 gate) filtered by funding crowding: do breakouts with very high
perp funding (crowded longs) underperform? Real Binance funding paid in all variants."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P)
apr7 = F.rolling(168, min_periods=48).sum() * 365 / 7          # trailing 7d funding, annualised
base = S.trend_breakout(P, long_only=True, port_vol_target=0.4)
base = base * S.efficiency_gate(P, 30, 0.1, 0.3).reindex(columns=base.columns).where(base.notna())
def show(label, w):
    res = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.02)
    r = res.returns
    a, b = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":])
    print(f"{label:40s} IS sh {a['sharpe']:.2f} cagr {a['cagr']*100:5.1f}% dd {a['max_dd']*100:5.1f}% | "
          f"OOS sh {b['sharpe']:.2f} cagr {b['cagr']*100:5.1f}% dd {b['max_dd']*100:5.1f}%", flush=True)
show("live universe LO (ER30 gate)", base)
f = apr7.shift(1)   # no look-ahead
for lo, hi in ((0.2, 0.5), (0.3, 0.8), (0.5, 1.0)):
    g = ((hi - f) / (hi - lo)).clip(0, 1).fillna(1)
    show(f"x funding gate: 1 below {int(lo*100)}% -> 0 at {int(hi*100)}% APR", base * g.where(base.notna()))
rk = f.rank(axis=1, pct=True)
show("drop coins in top funding tercile", base * (rk <= 2/3).astype(float).where(base.notna()))
show("only coins below median funding", base * (rk <= 0.5).astype(float).where(base.notna()))
# how often is the gate binding?
held = base.fillna(0) > 0
print("share of held coin-hours with 7d funding > 50% APR:", round(float((f[held] > 0.5).sum().sum() / held.sum().sum()), 3))
