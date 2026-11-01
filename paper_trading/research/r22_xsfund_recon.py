"""XS funding with Binance funding reconstructed from 1m premium archives; HL funding paid (live venue)."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.config import CACHE
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
from ptrade.universe import UNIVERSE
P = load_panel(); P = P[P.index >= "2024-01-08"]
FB = load_binance_funding_panel(load_panel(), default_8h=np.nan).reindex(P.index)
rec = {}
for s in UNIVERSE:
    nm = "1000PEPEUSDT" if s == "PEPEUSDT" else s
    parts = [pd.read_parquet(f)["rate"] for f in glob.glob(str(CACHE / "frecon" / f"{nm}_*.parquet"))]
    if parts:
        x = pd.concat(parts); x = x[~x.index.duplicated()].sort_index()
        x.index = x.index - pd.Timedelta(hours=1)          # settlement at T pays on the bar opening at T-1h
        rec[s] = x
R = pd.DataFrame(rec).reindex(P.index).fillna(0.0)
R[R.index > max(v.index[-1] for v in rec.values())] = np.nan
inv = {v: k for k, v in UNIVERSE.items()}
FH = {}
for f in glob.glob(str(CACHE / "hl_funding_*.parquet")):
    c = f.split("hl_funding_")[1][:-8]
    if c in inv:
        FH[inv[c]] = pd.read_parquet(f)["funding"]
FH = pd.DataFrame(FH).reindex(P.index)
Fpay = FH.fillna(FB).fillna(0.0)                           # HL funding where known, else Binance
print("coins with reconstruction:", R.notna().any().sum(), " with HL funding:", FH.notna().any().sum())
a = FB.fillna(0).rolling(168).sum(); b = R.rolling(168).sum()
rc = a.rank(axis=1).corrwith(b.rank(axis=1), axis=1).dropna()
print(f"7d-sum rank corr reconstructed(1m) vs actual: mean {rc.mean():.3f}")
for src, F in [("actual Binance", FB), ("reconstructed 1m", R)]:
    for lag in (0, 24, 48):
        w = S.xs_funding_carry(P, F.shift(lag), lookback_h=168, rebalance_h=24)
        r = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=Fpay).returns
        st = engine.perf_stats(r); yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
        print(f"{src:17s} lag {lag:2d}h (HL funding paid): sharpe {st['sharpe']:.2f} cagr {st['cagr']*100:5.1f}% dd {st['max_dd']*100:5.1f}% {yr}")
