"""r54 xs_momentum with a funding-crowding filter: drop longs whose trailing-7d Binance funding (lag 48h, live-like) is in
the top quantile and shorts in the bottom quantile; renormalise legs to the original gross. Real funding paid."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
f7 = F.shift(48).rolling(168, min_periods=120).mean()
rk = f7.rank(axis=1, pct=True)
def filt(w, q):
    L = w.clip(lower=0).where(~(rk > 1 - q), 0.0); Sh = w.clip(upper=0).where(~(rk < q), 0.0)
    gl, gs = w.clip(lower=0).sum(axis=1), -w.clip(upper=0).sum(axis=1)
    L = L.mul((gl / L.sum(axis=1)).replace([np.inf, -np.inf], 0).fillna(0), axis=0)
    Sh = Sh.mul((gs / -Sh.sum(axis=1)).replace([np.inf, -np.inf], 0).fillna(0), axis=0)
    return L + Sh
# rebalance only when the daily grid row changes (avoid hourly drift from funding ranks)
grid = P.index[P.index.hour == 23]
for lab, q in (("base", None), ("filter q0.25", 0.25), ("filter q0.15", 0.15), ("filter q0.10", 0.10)):
    w = wm if q is None else filt(wm, q)
    w = w.reindex(grid).reindex(P.index).ffill().fillna(0) if q is not None else w
    x = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
    o = []
    for a, b in (("2020", "2023"), ("2024", None)):
        st = engine.perf_stats(x[a:b]); o.append(f"{a}: sh {st['sharpe']:5.2f} cagr {st['cagr']*100:5.1f} dd {st['max_dd']*100:6.1f}")
    print(f"{lab:14s}", " | ".join(o), flush=True)
