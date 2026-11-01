"""Weekend effects: do weekend moves (Fri 00:00 → Mon 00:00 UTC) continue or reverse Mon-Tue?"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
P = load_panel()
d = P.copy(); d.index = d.index + pd.Timedelta(hours=1); d = d[d.index.hour == 0]   # daily closes at 00 UTC
mon = d[d.index.dayofweek == 0]
rows = []
for t in mon.index:
    fri = t - pd.Timedelta(days=3); wed = t + pd.Timedelta(days=2)
    if fri in d.index and wed in d.index:
        rows.append((t, d.loc[t] / d.loc[fri] - 1, d.loc[wed] / d.loc[t] - 1))
for name, sl in [("2019-23", lambda t: t.year <= 2023), ("2024-26", lambda t: t.year >= 2024)]:
    R = [(t, w, f) for t, w, f in rows if sl(t)]
    # BTC time-series
    wb = np.array([w["BTCUSDT"] for _, w, _ in R]); fb = np.array([f["BTCUSDT"] for _, _, f in R])
    ok = np.isfinite(wb) & np.isfinite(fb)
    print(f"{name} BTC: corr(weekend ret, Mon-Tue ret) = {np.corrcoef(wb[ok], fb[ok])[0,1]:+.3f}, n={ok.sum()}")
    # cross-sectional: rank weekend relative return, next Mon-Tue relative return, top-minus-bottom quintile
    spreads, ics = [], []
    for _, w, f in R:
        x = pd.DataFrame({"w": w, "f": f}).dropna()
        if len(x) < 10: continue
        x = x - x.mean()
        q = x.w.rank(pct=True)
        spreads.append(x.f[q > 0.8].mean() - x.f[q <= 0.2].mean()); ics.append(x.w.corr(x.f, method="spearman"))
    s = pd.Series(spreads)
    print(f"   XS: mean IC {np.mean(ics):+.3f}; weekend-winners minus losers Mon-Tue: {s.mean()*1e4:+.0f}bp (t={s.mean()/s.std()*np.sqrt(len(s)):.2f}, n={len(s)})")
# day-of-week mean returns for the equal-weight universe
ew = d.pct_change().mean(axis=1)
print("EW universe mean daily return by day (bp):", {k: round(v * 1e4, 1) for k, v in ew.groupby(ew.index.dayofweek).mean().items()})
