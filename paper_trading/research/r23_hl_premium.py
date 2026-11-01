"""HL hourly premium extremes (live-available) → forward returns of the coin (Binance spot closes)."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.universe import UNIVERSE
P = load_panel(); inv = {v: k for k, v in UNIVERSE.items()}
PR = {}
for f in glob.glob("cache/hl_funding_*.parquet"):
    c = f.split("hl_funding_")[1][:-8]
    if c in inv:
        PR[inv[c]] = pd.read_parquet(f)["premium"]
PR = pd.DataFrame(PR).reindex(P.index)
PR.index = PR.index  # premium stamped at hour H relates to the bar ending at H -> bar-open H-1
PR = PR.shift(-1)
Pc = P[PR.columns]
z = (PR - PR.rolling(24 * 14, min_periods=24 * 7).mean()) / PR.rolling(24 * 14, min_periods=24 * 7).std()
xz = z.sub(z.mean(axis=1), axis=0)          # cross-sectional relative premium
for h in (6, 24, 72):
    fwd = Pc.shift(-h - 1) / Pc.shift(-1) - 1   # entry 1h after signal
    rel = fwd.sub(fwd.mean(axis=1), axis=0)
    print(f"--- fwd {h}h relative return (bp) by premium z bucket (entry lag 1h)")
    for name, sl in [("2024", slice("2024", "2024")), ("2025", slice("2025", "2025")), ("2026", slice("2026", None))]:
        zz = z.loc[sl].stack(); rr = rel.loc[sl].stack()
        d = pd.DataFrame({"z": zz, "r": rr}).dropna()
        b = pd.cut(d.z, [-99, -3, -2, 2, 3, 99], labels=["<-3", "-3..-2", "mid", "2..3", ">3"])
        g = d.groupby(b).r.agg(["mean", "count"])
        print(f"   {name}: " + " | ".join(f"{k}: {v['mean']*1e4:+.0f} (n={int(v['count'])})" for k, v in g.iterrows()))
