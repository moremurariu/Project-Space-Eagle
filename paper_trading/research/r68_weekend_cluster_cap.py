"""r68: weekend fade (ex leveraged ETFs, the live rule since r67) with a cap on the net weight of correlated themes.
Prompted by weekend #3 (the short side was effectively one semis/AI bet). Clusters are hand-labelled before looking at results;
a data-driven alternative (hierarchical clustering on trailing hourly returns) is a robustness check. Pre-incident r32 panel."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
P = pd.read_parquet(CACHE / "xyz_1h.parquet")
NON = ("GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR")
LEV = {"SOXL", "KORU", "MUU"}
THEME = {**{c: "semis_ai" for c in "NVDA MU SKHX SKHY DRAM SNDK INTC AMD TSM MRVL SMSN CXMT KIOXIA LITE CBRS NBIS CRWV ORCL AVGO ARM SMH".split()},
         **{c: "crypto_eq" for c in "MSTR COIN HOOD CRCL STRC PURRDAT IREN BMNR".split()},
         **{c: "asia" for c in "BABA ZHIPU UNITREE EWY MINIMAX".split()}}
rows = []
for fri in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="W-FRI"):
    dst = 20 if 3 <= fri.month <= 10 else 21
    t_fri, t_sun, t_mon = fri + pd.Timedelta(hours=dst - 1), fri + pd.Timedelta(days=2, hours=22), fri + pd.Timedelta(days=3, hours=dst - 1)
    if not all(t in P.index for t in (t_fri, t_sun, t_mon)):
        continue
    for c in P.columns:
        n = c.split(":")[1]
        if n in NON or n in LEV:
            continue
        a, b, d = P.at[t_fri, c], P.at[t_sun, c], P.at[t_mon, c]
        if np.isfinite([a, b, d]).all() and a > 0:
            rows.append({"fri": fri, "coin": n, "wknd": b / a - 1, "mon": d / b - 1, "theme": THEME.get(n, n)})
S = pd.DataFrame(rows)
print("names by theme:", S.drop_duplicates("coin").theme.map(lambda t: t if t in ("semis_ai", "crypto_eq", "asia") else "other").value_counts().to_dict())

def weights(g, cap_theme=None):
    x = g.wknd - g.wknd.mean(); w = (-x / x.abs().sum()).clip(-0.1, 0.1); w = w / w.abs().sum()
    if cap_theme:
        for _ in range(20):                       # scale down any theme whose |net weight| exceeds the cap, renormalise to gross 1
            net = w.groupby(g.theme).sum(); bad = net[net.abs() > cap_theme + 1e-9]
            if bad.empty:
                break
            for t, v in bad.items():
                m = g.theme == t; w[m] = w[m] * cap_theme / abs(v)
            w = w / w.abs().sum()
    return w

def run(lab, cap=None):
    pnl, worst_theme = {}, []
    for f, g in S.groupby("fri"):
        w = weights(g, cap); pnl[f] = (w * g.mon).sum() - 0.0018
        worst_theme.append(w.groupby(g.theme).sum().abs().max())
    pnl = pd.Series(pnl); fr = sorted(pnl.index); h = fr[len(fr) // 2]
    print(f"{lab:26s} net {pnl.mean()*1e4:+6.1f}bp/wk t {pnl.mean()/pnl.std()*np.sqrt(len(pnl)):+.2f} worst {pnl.min()*1e4:+5.0f}bp "
          f"sd {pnl.std()*1e4:4.0f} | H1 {pnl[pnl.index<h].mean()*1e4:+5.1f} H2 {pnl[pnl.index>=h].mean()*1e4:+5.1f} | max theme net {np.mean(worst_theme):.2f}")
run("ex-lev (live rule)")
for cap in (0.35, 0.25, 0.15):
    run(f"theme net cap {cap:.2f}", cap)
