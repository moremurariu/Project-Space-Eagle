"""r51 wider XS universe (backlog #13): xs_momentum on the 40-coin universe vs 40 + top-30 HL candidates, 2024+ only.
Usage: python research/r51_wide_universe.py [n_extra] [seasoning_days]"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.data import binance
from ptrade.research_utils import load_panel
from ptrade.strategies import signals as S
N = int(sys.argv[1]) if len(sys.argv) > 1 else 30
C = pd.read_csv("research/universe_candidates_2026-11-08.csv")
C = C[~C.hl.isin(["PAXG"])].sort_values("oi_musd", ascending=False).head(N)
P0 = load_panel()
dfs = binance.load_many(list(C.binance), "1h", start="2023-06-01", refresh=False)
X = binance.panel(dfs, "close")
X = X.reindex(P0.index).ffill(limit=3)
first = X.apply(lambda s: s.first_valid_index())
print("extra coins with data:", X.notna().any().sum(), "| first-bar dates:\n", first.sort_values().dt.strftime("%Y-%m").value_counts().sort_index().to_dict())
SEASON = int(sys.argv[2]) if len(sys.argv) > 2 else 0   # days to skip after a coin's first bar (post-listing)
for c in X.columns:
    f = X[c].first_valid_index()
    if f is not None and SEASON: X.loc[:f + pd.Timedelta(days=SEASON), c] = np.nan
P1 = pd.concat([P0, X], axis=1)
P0, P1 = P0["2023-06":], P1["2023-06":]
def mom(P):
    w = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
            for lb in (7, 14, 21) for o in range(7)) / 21
    return engine.run(P, w, fee=0.00045, slippage=0.0005, min_trade=0.01).returns, w
for lab, P in (("40 coins", P0), (f"40+{N}", P1)):
    r, w = mom(P)
    n = P.notna().sum(axis=1).resample("QE").mean().round(0).to_dict()
    for a, b in (("2024", "2024"), ("2025", "2025"), ("2026", None), ("2024", None)):
        st = engine.perf_stats(r[a:b]); print(f"{lab:9s} {a}-{b or 'now'}: sh {st['sharpe']:5.2f} cagr {st['cagr']*100:6.1f}% dd {st['max_dd']*100:6.1f}%")
    print("  avg coins per quarter:", {k.strftime('%y-Q%q') if hasattr(k,'strftime') else k: v for k, v in list(n.items())[-6:]})
