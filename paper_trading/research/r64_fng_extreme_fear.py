"""r64 Does extreme fear (Fear & Greed <= 15) predict BTC forward returns? Informs the discretionary book (F&G 13 on 11-14).
Episodes = first day F&G <= thr after >= 14 days above it. Forward returns from the next daily close (F&G is published
~00 UTC for the prior day). Compared with the unconditional mean over the same period. IS 2018-2023 / OOS 2024+."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.data import sentiment
from ptrade.research_utils import load_panel
f = sentiment.fear_greed(0); f.index = f.index.floor("D")
btc = load_panel()["BTCUSDT"].dropna(); d = btc.groupby(btc.index.floor("D")).last()
f = f.reindex(d.index).ffill(limit=2)
print(f"F&G {f.first_valid_index():%Y-%m-%d} .. {f.last_valid_index():%Y-%m-%d}; BTC daily from {d.index[0]:%Y-%m-%d}")
H = (7, 14, 30, 60)
fwd = {h: d.shift(-(h + 1)) / d.shift(-1) - 1 for h in H}
for thr in (10, 15, 20):
    low = (f <= thr)
    first = low & ~low.shift(1).rolling(14, min_periods=1).max().fillna(0).astype(bool)
    rows = []
    for per, m in (("IS", d.index < "2024-01-01"), ("OOS", d.index >= "2024-01-01")):
        ep = first & m
        r = {"thr": thr, "per": per, "episodes": int(ep.sum()), "days_low": int((low & m).sum())}
        for h in H:
            x = fwd[h][ep].dropna(); base = fwd[h][m].dropna()
            r[f"ep{h}d%"] = x.mean() * 100; r[f"base{h}d%"] = base.mean() * 100; r[f"hit{h}"] = (x > 0).mean()
            xl = fwd[h][low & m].dropna(); r[f"alllow{h}d%"] = xl.mean() * 100   # every low day (overlapping)
        rows.append(r)
    print(pd.DataFrame(rows).set_index(["thr", "per"]).round(2).T.to_string())
thr = 15; low = f <= thr; first = low & ~low.shift(1).rolling(14, min_periods=1).max().fillna(0).astype(bool)
ep = pd.DataFrame({"fng": f[first], "btc": d[first], **{f"fwd{h}d%": (fwd[h][first] * 100).round(1) for h in H}})
print(ep.to_string())
