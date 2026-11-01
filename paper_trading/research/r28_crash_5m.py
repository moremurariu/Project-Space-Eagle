"""Crash-rebound with continuous 5m scanning vs hourly scanning (2024-01 → 2026-09), lag 5 minutes."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.config import CACHE
from ptrade.universe import UNIVERSE
cols = {}
for s in UNIVERSE:
    fs = sorted(glob.glob(str(CACHE / "k5m" / f"{s}_*.parquet")))
    if fs:
        x = pd.concat([pd.read_parquet(f)["c"] for f in fs]); cols[s] = x[~x.index.duplicated()]
C = pd.DataFrame(cols).sort_index()
C = C.reindex(pd.date_range(C.index[0], C.index[-1], freq="5min")).ffill(limit=3)
print("5m panel", C.shape, C.index[0], C.index[-1])
F = pd.DataFrame(0.0001 / 8 / 12, index=C.index, columns=C.columns)
r4 = C / C.shift(48) - 1
on_hour = (C.index.minute == 0)
def run(scan, market_wide, thr=-0.10, hold=288, per_w=0.15):
    sig = r4 < thr
    if market_wide:
        sig = sig & (r4["BTCUSDT"] < -0.03).to_numpy()[:, None]
    if scan == "hourly":
        sig = sig & on_hour[:, None]
    pos = (sig.astype(float) * per_w).rolling(hold, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    w = pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
    res = engine.run(C, w, fee=0.00045, slippage=0.001, funding=F, lag=1)
    r = res.returns
    st = engine.perf_stats(r, 8760 * 12)
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    n_entries = int(((pos > 0) & (pos.shift(1) == 0)).sum().sum())
    print(f"scan={scan:6s} market_wide={market_wide!s:5s}: sharpe {st['sharpe']:.2f} cagr {st['cagr']*100:5.1f}% "
          f"dd {st['max_dd']*100:5.1f}% entries {n_entries} {yr}", flush=True)
for mw in (False, True):
    for scan in ("5m", "hourly"):
        run(scan, mw)
