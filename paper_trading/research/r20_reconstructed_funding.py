"""Validate Binance funding reconstructed from premium-index archives; backtest XS funding on it (with lag)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.config import CACHE
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
from ptrade.universe import UNIVERSE
P = load_panel(); FB = load_binance_funding_panel(P, default_8h=np.nan)
rec = {}
for s in UNIVERSE:
    f = CACHE / f"binance_premium_{'1000PEPEUSDT' if s == 'PEPEUSDT' else s}.parquet"
    d = pd.read_parquet(f); p = (d["open"] + d["close"]) / 2
    rec[s] = (p + (0.0001 - p).clip(-0.0005, 0.0005)) / 8          # hourly-equivalent funding
R = pd.DataFrame(rec).reindex(P.index)
win = slice("2024-01-08", "2026-09-30")
a = FB.loc[win].fillna(0).rolling(168).sum(); b = R.loc[win].rolling(168).sum()
rc = a.rank(axis=1).corrwith(b.rank(axis=1), axis=1).dropna()
qa, qb = a.rank(axis=1, pct=True), b.rank(axis=1, pct=True)
top = ((qa > 0.75) & (qb > 0.75)).sum(axis=1) / (qa > 0.75).sum(axis=1)
bot = ((qa <= 0.25) & (qb <= 0.25)).sum(axis=1) / (qa <= 0.25).sum(axis=1)
print(f"7d-sum rank corr reconstructed vs actual: mean {rc.mean():.3f} median {rc.median():.3f}; quartile overlap top {top.mean():.2f} bottom {bot.mean():.2f}")
Pu = P[P.index >= "2024-01-08"]; Fpay = FB.reindex(Pu.index).fillna(0)
for src, F in [("actual funding", FB), ("reconstructed", R)]:
    for lag in (0, 48):
        w = S.xs_funding_carry(Pu, F.shift(lag).reindex(Pu.index), lookback_h=168, rebalance_h=24)
        r = engine.run(Pu, w, fee=0.00045, slippage=0.0005, funding=Fpay).returns
        st = engine.perf_stats(r); yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
        print(f"{src:15s} lag {lag:2d}h: sharpe {st['sharpe']:.2f} cagr {st['cagr']*100:5.1f}% dd {st['max_dd']*100:5.1f}% {yr}")
# --- v2: per-coin settlement interval (from archives, time-varying, carried forward)
print("\n--- v2 with per-coin funding interval")
iv = {}
for s in UNIVERSE:
    for nm in (s, "1000" + s):
        f = CACHE / f"binance_funding_{nm}.parquet"
        if f.exists():
            idx = pd.read_parquet(f).index.floor("h"); idx = idx[~idx.duplicated()]
            gaps = pd.Series(idx[1:] - idx[:-1], index=idx[1:]).dt.total_seconds() / 3600
            iv[s] = gaps.clip(1, 8).rolling(3, min_periods=1).median()
            break
IV = pd.DataFrame(iv).reindex(P.index).ffill().fillna(8)
print("coins ever on <8h interval:", sorted(c for c in IV if (IV[c] < 8).any()))
R2 = R * (8 / IV)
b2 = R2.loc[win].rolling(168).sum()
rc2 = a.rank(axis=1).corrwith(b2.rank(axis=1), axis=1).dropna()
print(f"v2 rank corr: mean {rc2.mean():.3f}")
for lag in (0, 48):
    w = S.xs_funding_carry(Pu, R2.shift(lag).reindex(Pu.index), lookback_h=168, rebalance_h=24)
    r = engine.run(Pu, w, fee=0.00045, slippage=0.0005, funding=Fpay).returns
    st = engine.perf_stats(r); yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"reconstructed v2 lag {lag:2d}h: sharpe {st['sharpe']:.2f} cagr {st['cagr']*100:5.1f}% dd {st['max_dd']*100:5.1f}% {yr}")
