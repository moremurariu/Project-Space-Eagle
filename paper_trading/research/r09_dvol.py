"""Deribit DVOL (BTC 30d implied vol) as a signal: VRP, DVOL spikes, crash-rebound conditioning."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.data import deribit
from ptrade.research_utils import load_panel
P = load_panel()
dv = deribit.dvol_history("BTC")["close"]
btc = P["BTCUSDT"]
rv = btc.pct_change().rolling(24 * 30).std() * np.sqrt(8760) * 100
df = pd.DataFrame({"px": btc, "dvol": dv.reindex(btc.index).ffill(limit=3), "rv": rv}).dropna()
df["vrp"] = df.dvol - df.rv
df["dvol_chg24"] = df.dvol / df.dvol.shift(24) - 1
for h in (24, 168):
    df[f"fwd{h}"] = df.px.shift(-h) / df.px - 1
SPL = [("2021-23", slice("2021", "2023")), ("2024-26", slice("2024", None))]
def bucket_table(col, q=5, fwd="fwd168"):
    for name, s in SPL:
        x = df.loc[s].dropna(subset=[fwd])
        b = pd.qcut(x[col], q, labels=False, duplicates="drop")
        g = x.groupby(b)[fwd].mean() * 100
        print(f"   {name}: " + " ".join(f"q{int(k)}={v:+.2f}%" for k, v in g.items()))
print("BTC fwd 7d return by VRP quintile (q0 = implied << realised):"); bucket_table("vrp")
print("BTC fwd 7d return by DVOL level quintile:"); bucket_table("dvol")
print("BTC fwd 24h return by 24h DVOL change quintile (q4 = vol spike):"); bucket_table("dvol_chg24", fwd="fwd24")
# crash-rebound conditioning: pooled alt events, split by market context at event time
r4 = P / P.shift(4) - 1
f24 = P.shift(-24) / P - 1
ev = (r4 < -0.08)
rows = []
for t, row in ev[ev.any(axis=1)].iterrows():
    if t not in df.index: continue
    for sym in row.index[row.values]:
        v = f24.at[t, sym]
        if pd.notna(v):
            rows.append((t, sym, v, df.at[t, "dvol"], df.at[t, "dvol_chg24"], df.at[t, "vrp"], r4.at[t, "BTCUSDT"]))
E = pd.DataFrame(rows, columns=["t", "sym", "fwd24", "dvol", "dchg", "vrp", "btc4h"]).set_index("t")
print(f"\ncrash events (-8% 4h) with DVOL data: {len(E)}")
for name, s in SPL:
    e = E.loc[s]
    for col in ("dvol", "dchg", "vrp", "btc4h"):
        med = E[col].median()
        hi, lo = e[e[col] > med].fwd24, e[e[col] <= med].fwd24
        print(f"   {name} split by {col:6s} (median {med:+.3f}): high mean {hi.mean()*1e4:+.0f}bp n={len(hi)} | low mean {lo.mean()*1e4:+.0f}bp n={len(lo)}")
