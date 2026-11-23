"""r67: weekend fade with leveraged ETFs (SOXL 3x, KORU 3x) excluded or all names inverse-vol scaled. Prompted by weekend #3
(11-23: NVDA +27% on CEO news; the SOXL short at -8.6% weight was half the loss). Uses the pre-incident r32 panel (to 10-30)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
P = pd.read_parquet(CACHE / "xyz_1h.parquet")
NON = ("GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR")
LEV = {"SOXL", "KORU"}
print("leveraged ETFs present:", sorted({c.split(':')[1] for c in P.columns} & LEV))
vol = np.log(P).diff().rolling(24 * 20, min_periods=24 * 5).std()        # trailing 20d hourly vol
rows = []
for fri in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="W-FRI"):
    dst = 20 if 3 <= fri.month <= 10 else 21
    t_fri, t_sun, t_mon = fri + pd.Timedelta(hours=dst - 1), fri + pd.Timedelta(days=2, hours=22), fri + pd.Timedelta(days=3, hours=dst - 1)
    if not all(t in P.index for t in (t_fri, t_sun, t_mon)):
        continue
    for c in P.columns:
        if c.split(":")[1] in NON:
            continue
        a, b, d, v = P.at[t_fri, c], P.at[t_sun, c], P.at[t_mon, c], vol.at[t_fri, c]
        if np.isfinite([a, b, d]).all() and a > 0:
            rows.append({"fri": fri, "coin": c.split(":")[1], "wknd": b / a - 1, "mon": d / b - 1, "vol": v})
S = pd.DataFrame(rows)
def run(s, lab, volscale=False):
    s = s.copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean")
    raw = -s.x / (s.vol.fillna(s.vol.median()) if volscale else 1.0)
    s["w"] = raw / raw.abs().groupby(s.fri).transform("sum")
    s["w"] = s.w.clip(-0.1, 0.1); s["w"] = s.w / s.w.abs().groupby(s.fri).transform("sum")
    pnl = s.groupby("fri").apply(lambda g: (g.w * g.mon).sum() - 0.0018)
    fr = sorted(pnl.index); h = fr[len(fr) // 2]
    print(f"{lab:28s} n {len(pnl)} net {pnl.mean()*1e4:+6.1f}bp/wk t {pnl.mean()/pnl.std()*np.sqrt(len(pnl)):+.2f} "
          f"worst {pnl.min()*1e4:+5.0f}bp sd {pnl.std()*1e4:4.0f}bp | H1 {pnl[pnl.index<h].mean()*1e4:+5.1f} H2 {pnl[pnl.index>=h].mean()*1e4:+5.1f}")
run(S, "baseline (live rule)")
run(S[~S.coin.isin(LEV)], "ex leveraged ETFs")
run(S, "inverse-vol weights", volscale=True)
run(S[~S.coin.isin(LEV)], "ex lev + inverse-vol", volscale=True)
