"""Does the Binance-funding XS edge survive a 1-2 day signal delay (archive publication lag)?"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); FB = load_binance_funding_panel(P, default_8h=np.nan)
for start in ("2021-01-01", "2024-01-01"):
    Pu = P[P.index >= start]; Fpay = FB.reindex(Pu.index).fillna(0)
    for lag_h in (0, 24, 36, 48, 72):
        sig = FB.shift(lag_h).reindex(Pu.index)
        w = S.xs_funding_carry(Pu, sig, lookback_h=168, rebalance_h=24)
        r = engine.run(Pu, w, fee=0.00045, slippage=0.0005, funding=Fpay).returns
        st = engine.perf_stats(r)
        yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
        print(f"from {start} signal lag {lag_h:2d}h: sharpe {st['sharpe']:.2f} cagr {st['cagr']*100:5.1f}% dd {st['max_dd']*100:5.1f}% {yr}")
