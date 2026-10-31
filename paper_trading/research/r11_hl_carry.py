"""HL-native funding backtests: delta-neutral carry (funding + basis P&L) and directional XS funding."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel
from ptrade.universe import UNIVERSE
fund, prem = {}, {}
for f in glob.glob("cache/hl_funding_*.parquet"):
    c = f.split("hl_funding_")[1][:-8]
    df = pd.read_parquet(f)
    if len(df) > 24 * 60:
        fund[c] = df["funding"]; prem[c] = df["premium"]
F = pd.DataFrame(fund).sort_index(); PR = pd.DataFrame(prem).reindex(F.index)
F = F[F.index >= "2024-01-01"]; PR = PR.reindex(F.index)
print(f"coins: {F.shape[1]}, hours: {len(F)}, from {F.index[0]}")
print("mean funding APR by half-year (cross-sectional mean %):",
      (F.mean(axis=1).groupby(F.index.to_period('Q').astype(str)).mean() * 8760 * 100).round(1).to_dict())

def dn_carry(entry=0.10, exit_=0.03, lb=168, top_k=6, max_per=0.15, max_tot=0.9, cost_side=0.0020, basis=True):
    trail = F.rolling(lb, min_periods=lb // 2).mean() * 8760
    Fv = F.fillna(0).to_numpy(); dP = PR.diff().fillna(0).to_numpy()
    n = np.zeros(F.shape[1]); held = np.zeros(F.shape[1], bool); out = np.zeros(len(F))
    for t in range(len(F)):
        out[t] = float(n @ Fv[t]) - (float(n @ dP[t]) if basis else 0.0)  # short perp: gains when premium falls
        if F.index[t].hour == 0:
            tr = trail.iloc[t].to_numpy(); ok = np.isfinite(tr)
            nh = np.where(held, ok & (tr > exit_), ok & (tr > entry))
            if nh.sum() > top_k:
                keep = np.zeros_like(nh); keep[np.argsort(-np.where(nh, tr, -np.inf))[:top_k]] = True; nh &= keep
            k = nh.sum(); tgt = np.where(nh, min(max_per, max_tot / k) if k else 0.0, 0.0)
            out[t] -= np.abs(tgt - n).sum() * cost_side
            n, held = tgt, nh
    return pd.Series(out, index=F.index)
print("\n--- DN carry on HL (funding + basis, cost 0.20%/side both legs)")
for kw in [dict(), dict(basis=False), dict(entry=0.15, exit_=0.05), dict(entry=0.20, exit_=0.08), dict(entry=0.30, exit_=0.10),
           dict(entry=0.10, exit_=0.03, lb=72), dict(entry=0.10, exit_=0.03, top_k=10, max_per=0.09)]:
    r = dn_carry(**kw)
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    st = engine.perf_stats(r)
    print(f"  {str(kw):60s} cagr {st['cagr']*100:5.1f}% vol {st['vol']*100:4.1f}% sh {st['sharpe']:5.2f} dd {st['max_dd']*100:5.1f}% yearly {yr}")
# BTC/ETH always-on baseline
r = (F[["BTC", "ETH"]].fillna(0) * 0.45).sum(axis=1) - (PR[["BTC", "ETH"]].diff().fillna(0) * 0.45).sum(axis=1)
print(f"  always-on BTC+ETH 0.9 notional: cagr {engine.perf_stats(r)['cagr']*100:.1f}% sh {engine.perf_stats(r)['sharpe']:.2f}")

print("\n--- Directional XS funding using HL funding (prices: Binance)")
P = load_panel()
inv = {v: k for k, v in UNIVERSE.items()}
cols = [c for c in F.columns if c in inv]
Fb = F[cols].rename(columns=inv)
Pb = P[Fb.columns]; Pb = Pb[Pb.index >= F.index[0]]
Fh = Fb.reindex(Pb.index)
from ptrade.strategies import signals as S
for lb, reb in [(168, 24), (168, 168), (72, 24)]:
    w = S.xs_funding_carry(Pb, Fh, lookback_h=lb, rebalance_h=reb, min_assets=6)
    res = engine.run(Pb, w, fee=0.00045, slippage=0.0005, funding=Fh.fillna(0))
    r = res.returns
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    st = res.stats()
    print(f"  lb={lb} reb={reb} n_coins={len(cols)}: {engine.fmt_stats(st)} yearly {yr}")
