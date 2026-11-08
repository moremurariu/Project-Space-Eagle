"""r52 time-series trend on alt/BTC ratios: Donchian breakout on each alt/BTC price ratio, long/short the ratio
(alt leg w, BTC leg -w), vol-targeted, daily. Real Binance funding, 0.045% + 5bp. Compared with xs_momentum."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
alts = [c for c in P.columns if c != "BTCUSDT"]
R = P[alts].div(P["BTCUSDT"], axis=0)
def to_legs(wr):
    w = wr.copy(); w["BTCUSDT"] = -wr.sum(axis=1, min_count=1)
    return w.reindex(columns=P.columns)
res = {}
for pvt in (0.25, 0.40):
    for lo in (False, True):
        wr = S.trend_breakout(R, long_only=lo, port_vol_target=pvt)
        w = to_legs(wr)
        x = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.02).returns
        lab = f"ratio trend pvt{pvt} {'LO(alt>BTC only)' if lo else 'L/S'}"
        res[lab] = x
        o = []
        for a, b in (("2020", "2023"), ("2024", None)):
            st = engine.perf_stats(x[a:b]); o.append(f"{a}: sh {st['sharpe']:5.2f} cagr {st['cagr']*100:5.1f} dd {st['max_dd']*100:6.1f}")
        g = w.ffill().fillna(0).abs().sum(axis=1)["2020":].mean()
        print(f"{lab:34s}", " | ".join(o), f"| avg gross {g:.2f}", flush=True)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
res["xs_mom"] = engine.run(P, wm, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
D = pd.DataFrame({k: (1 + v).groupby(v.index.floor("D")).prod() - 1 for k, v in res.items()})["2020":]
print("daily corr vs xs_mom:", D.corr()["xs_mom"].round(2).to_dict())
