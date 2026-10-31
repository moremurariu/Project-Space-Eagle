"""Crash-rebound event conditioning: volume capitulation, BTC trend regime, depth of drop, time-of-day."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_volume_panel
P = load_panel(); V = load_volume_panel().reindex(P.index)
r4 = P / P.shift(4) - 1
f24 = P.shift(-25) / P.shift(-1) - 1           # enter 1h after the trigger (realistic lag), hold 24h
vol4 = V.rolling(4).sum(); vratio = vol4 / (V.rolling(24 * 30, min_periods=24 * 10).mean() * 4)
btc = P["BTCUSDT"]; btc_trend = btc / btc.rolling(24 * 50).mean() - 1      # BTC vs 50d MA
r24pre = P / P.shift(24) - 1
ev = r4 < -0.10
rows = []
for t, row in ev[ev.any(axis=1)].iterrows():
    for s in row.index[row.values]:
        v = f24.at[t, s]
        if pd.notna(v):
            rows.append((t, s, v, vratio.at[t, s], btc_trend.at[t], r4.at[t, s], r4.at[t, "BTCUSDT"], r24pre.at[t, s], t.hour))
E = pd.DataFrame(rows, columns=["t", "sym", "fwd", "vratio", "btc_trend", "drop", "btc4h", "r24", "hour"]).set_index("t")
E = E[~E.index.duplicated(keep="first") | True]
print(f"events (−10% 4h, entry lag 1h): {len(E)}; mean fwd24 {E.fwd.mean()*1e4:.0f}bp, median {E.fwd.median()*1e4:.0f}bp")
SPL = [("2019-23", slice(None, "2023")), ("2024-26", slice("2024", None))]
def split(col, cuts, labels):
    for name, s in SPL:
        e = E.loc[s]; b = pd.cut(e[col], cuts, labels=labels)
        g = e.groupby(b).fwd.agg(["mean", "median", "count"])
        print(f"   {name}: " + " | ".join(f"{k}: mean {r['mean']*1e4:+.0f} med {r['median']*1e4:+.0f} n={int(r['count'])}" for k, r in g.iterrows()))
print("by 4h volume vs 30d avg (capitulation):"); split("vratio", [0, 3, 6, 1e9], ["<3x", "3-6x", ">6x"])
print("by BTC trend (vs 50d MA):"); split("btc_trend", [-1, -0.05, 0.05, 10], ["BTC<-5%", "flat", "BTC>+5%"])
print("by drop depth:"); split("drop", [-1, -0.2, -0.15, -0.10], ["<-20%", "-20..-15", "-15..-10"])
print("by prior 24h move (already down a lot?):"); split("r24", [-1, -0.2, -0.1, 10], ["24h<-20%", "-20..-10", ">-10%"])
