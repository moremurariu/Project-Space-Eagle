"""Binance positioning metrics (OI, long/short ratios, taker ratio) as daily signals for BTC/ETH/SOL."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.data import binance
from ptrade.research_utils import load_panel
P = load_panel(["BTCUSDT", "ETHUSDT", "SOLUSDT"], start="2021-06-01")
rows = []
for s in ["BTCUSDT", "ETHUSDT", "SOLUSDT"]:
    m = binance.load_metrics(s)
    d = m.resample("D").mean()                         # daily average of day D
    d["oi_last"] = m["oi_usd"].resample("D").last()
    f = pd.DataFrame(index=d.index)
    z = lambda x, w=90: (x - x.rolling(w, min_periods=60).mean()) / x.rolling(w, min_periods=60).std()
    f["ls_acc_z"] = z(d.ls_accounts)
    f["top_pos_z"] = z(d.top_ls_positions)
    f["top_vs_retail"] = z(np.log(d.top_ls_positions / d.ls_accounts))
    f["oi_chg7"] = d.oi_last.pct_change(7)
    f["oi_chg1"] = d.oi_last.pct_change(1)
    f["taker_1d"] = np.log(d.taker_ls)
    f["taker_7d"] = np.log(d.taker_ls).rolling(7).mean()
    f = f.shift(1)                                     # day D data usable only from D+2 00:00 (publication lag)
    px = P[s].copy(); px.index = px.index + pd.Timedelta(hours=1); px = px[px.index.hour == 0]
    px.index = px.index.normalize()
    for h in (1, 3, 7):
        f[f"fwd{h}"] = px.shift(-h) / px - 1
    f.index = f.index + pd.Timedelta(days=1)           # features dated D are known at start of D+2 → align to D+1 close
    f["sym"] = s
    rows.append(f)
X = pd.concat(rows).dropna().sort_index()
feats = ["ls_acc_z", "top_pos_z", "top_vs_retail", "oi_chg7", "oi_chg1", "taker_1d", "taker_7d"]
for h in (1, 3, 7):
    print(f"--- Spearman IC vs fwd {h}d (pooled BTC/ETH/SOL)")
    for name, sl in [("2022-23", slice("2022", "2023")), ("2024-26", slice("2024", None))]:
        x = X.loc[sl]
        ics = {ft: round(x[ft].corr(x[f"fwd{h}"], method="spearman"), 3) for ft in feats}
        print(f"   {name} n={len(x)}: {ics}")
