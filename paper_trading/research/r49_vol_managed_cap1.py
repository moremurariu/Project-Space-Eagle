"""r49 de-risk-only vol management (cap 1.0, r48 method) on trend_majors / xs_funding / xs_momentum, full hourly engine."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
from ptrade.live.strategies import TrendBreakoutAltsLO as T
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
books = {"trend_majors": (S.trend_breakout(P[MAJORS], long_only=False, port_vol_target=0.25), MAJORS, 0.02),
         "xs_funding": (S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24), None, 0.01),
         "xs_momentum": (wm, None, 0.01)}
for name, (w, cols, mt) in books.items():
    Pu = P if cols is None else P[cols]; Fu = F if cols is None else F[cols]
    for lab, m in (("base", None), ("cap1 win20", 20), ("cap1 win60", 60)):
        ww = w if m is None else w.mul(T.vol_scale(Pu, w, m, 1.0, None), axis=0)
        x = engine.run(Pu, ww, fee=0.00045, slippage=0.0005, funding=Fu, min_trade=mt).returns
        o = []
        for a, b in (("2020", "2023"), ("2024", None)):
            st = engine.perf_stats(x[a:b]); o.append(f"{a}: sh {st['sharpe']:5.2f} cagr {st['cagr']*100:5.1f} dd {st['max_dd']*100:6.1f}")
        print(f"{name:13s} {lab:11s}", " | ".join(o), flush=True)
