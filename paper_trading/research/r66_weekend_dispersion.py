"""r66: does the r32 xyz weekend fade (linear, cap 10%, 0.18% round trip) earn more on high-dispersion weekends?
Re-uses r32's data/rows (exec r32 up to the trade section via import of its df is awkward, so the panel logic is repeated)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
P = pd.read_parquet(CACHE / "xyz_1h.parquet")
NON = ("GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR")
rows = []
for fri in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="W-FRI"):
    dst = 20 if 3 <= fri.month <= 10 else 21
    t_fri, t_sun, t_mon = fri + pd.Timedelta(hours=dst - 1), fri + pd.Timedelta(days=2, hours=22), fri + pd.Timedelta(days=3, hours=dst - 1)
    if not all(t in P.index for t in (t_fri, t_sun, t_mon)):
        continue
    for c in P.columns:
        if c.split(":")[1] in NON:
            continue
        a, b, d = P.at[t_fri, c], P.at[t_sun, c], P.at[t_mon, c]
        if np.isfinite([a, b, d]).all() and a > 0:
            rows.append({"fri": fri, "coin": c, "wknd": b / a - 1, "mon": d / b - 1})
s = pd.DataFrame(rows)
s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean")
s["w"] = -s.x / s.groupby("fri").x.transform(lambda v: v.abs().sum())
s["w"] = s.w.clip(-0.1, 0.1); s["w"] = s.w / s.groupby("fri").w.transform(lambda v: v.abs().sum())
g = s.groupby("fri")
wk = pd.DataFrame({"disp": g.x.std(), "mean_move": g.wknd.mean(), "n": g.size(),
                   "pnl": g.apply(lambda q: (q.w * q.mon).sum() - 0.0018)})
print(f"weekends {len(wk)} ({wk.index[0]:%Y-%m-%d} -> {wk.index[-1]:%Y-%m-%d}); median dispersion {wk.disp.median()*100:.2f}%")
print(f"corr(disp, pnl) {wk.disp.corr(wk.pnl):+.2f}  corr(|mean move|, pnl) {wk.mean_move.abs().corr(wk.pnl):+.2f}")
wk["tercile"] = pd.qcut(wk.disp, 3, labels=["low", "mid", "high"])
for t, q in wk.groupby("tercile"):
    print(f"  {t:4s} disp {q.disp.min()*100:.2f}-{q.disp.max()*100:.2f}%  n {len(q):2d}  net {q.pnl.mean()*1e4:+6.1f}bp  hit {np.mean(q.pnl > 0):.0%}")
for thr in (0.01, 0.015):
    a, b = wk[wk.disp < thr], wk[wk.disp >= thr]
    print(f"  disp < {thr*100:.1f}%: n {len(a):2d} net {a.pnl.mean()*1e4:+6.1f}bp | >= : n {len(b):2d} net {b.pnl.mean()*1e4:+6.1f}bp")
print(wk.tail(8).assign(disp=lambda d: (d.disp*100).round(2), mean_move=lambda d: (d.mean_move*100).round(2), pnl=lambda d: (d.pnl*1e4).round(0)).to_string())
