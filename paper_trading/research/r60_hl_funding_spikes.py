"""r60 HL hourly funding spikes → forward relative returns. Event: a coin's HL hourly funding (8h-equivalent APR) crosses
an extreme (> +X% or < −X% APR, first hour after ≥24h without a spike). Forward return from the next hour's close over
h hours, minus the equal-weight universe return (Binance 1h). Fade = short positive spikes / long negative spikes.
Cost: 0.045% + 5bp per side (0.19% round trip). IS = 2024, OOS = 2025+ (HL hourly funding history starts 2023-05)."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.universe import UNIVERSE
P = load_panel(); inv = {v: k for k, v in UNIVERSE.items()}
ew = P.pct_change(fill_method=None).mean(axis=1)
rows = []
for f in glob.glob("cache/hl_funding_*.parquet"):
    coin = f.split("hl_funding_")[1][:-8]
    sym = inv.get(coin)
    if sym is None or sym not in P: continue
    s = pd.read_parquet(f)["funding"]; s.index = pd.to_datetime(s.index, utc=True).floor("h")
    s = s[~s.index.duplicated()].sort_index()["2024":]
    apr = s * 24 * 365
    px = P[sym]
    for thr in (0.5, 1.0, 2.0):
        for sign in (1, -1):
            hit = (apr * sign > thr)
            prev = hit.rolling(24, min_periods=1).sum().shift(1).fillna(0)
            ev = hit & (prev == 0)
            for t in ev[ev].index:
                if t not in px.index: continue
                i = px.index.get_loc(t)
                if i + 1 + 72 >= len(px): continue
                p0 = px.iloc[i + 1]
                for h in (8, 24, 72):
                    r = px.iloc[i + 1 + h] / p0 - 1
                    m = (1 + ew.iloc[i + 2:i + 2 + h]).prod() - 1
                    rows.append({"coin": coin, "t": t, "thr": thr, "sign": sign, "h": h, "rel": r - m})
E = pd.DataFrame(rows); E["fade"] = -E.sign * E.rel - 0.0019
E["per"] = np.where(E.t < pd.Timestamp("2025-01-01", tz="UTC"), "IS 2024", "OOS 2025+")
g = E.groupby(["thr", "sign", "h", "per"]).fade.agg(["count", "mean", lambda x: x.mean() / x.std() * np.sqrt(len(x))])
g.columns = ["n", "fade_net", "t"]; g["fade_net"] = (g.fade_net * 1e4).round(1); g["t"] = g.t.round(2)
print(g.unstack("per").to_string())
