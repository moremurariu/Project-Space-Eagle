"""r50 xyz weekend fade conditioned on the weekend cross-sectional dispersion (is the edge only there when moves are big?)."""
import sys, time, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
from ptrade.data import hyperliquid as hl

path = CACHE / "xyz_1h.parquet"
if path.exists() and time.time() - path.stat().st_mtime < 86400:
    P = pd.read_parquet(path)
else:
    m = hl.meta_and_ctxs("xyz")
    m = m[~m["delisted"]].sort_values("oi_usd", ascending=False)
    cols = {}
    for c in m.index[:45]:
        try:
            d = hl.candles(c, "1h")
            if len(d) > 500:
                cols[c] = d["close"]
        except Exception as e:  # noqa: BLE001
            print(c, e)
        time.sleep(0.3)
    P = pd.DataFrame(cols).sort_index(); P.to_parquet(path)
print(P.shape, P.index[0], P.index[-1])
equity_like = [c for c in P.columns if c.split(":")[1] not in
               ("GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR")]
rows = []
for fri in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="W-FRI"):
    dst = 20 if 3 <= fri.month <= 10 else 21          # US cash close in UTC (approx DST)
    t_fri = fri + pd.Timedelta(hours=dst - 1)          # bar opening 1h before close -> close at US close
    t_sun = fri + pd.Timedelta(days=2, hours=22)       # Sunday 23:00 UTC close
    t_mon = fri + pd.Timedelta(days=3, hours=dst - 1)  # Monday US close
    if t_mon not in P.index or t_fri not in P.index or t_sun not in P.index:
        continue
    for c in P.columns:
        a, b, d = P.at[t_fri, c], P.at[t_sun, c], P.at[t_mon, c]
        if np.isfinite([a, b, d]).all() and a > 0:
            rows.append({"fri": fri, "coin": c, "wknd": b / a - 1, "mon": d / b - 1, "is_eq": c in equity_like})
df = pd.DataFrame(rows)
s = df[df.is_eq].copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean")
s["w"] = -s.x / s.groupby("fri").x.transform(lambda v: v.abs().sum())
s["w"] = s.w.clip(-0.10, 0.10); s["w"] = s.w / s.groupby("fri").w.transform(lambda v: v.abs().sum())
g = s.groupby("fri")
W = pd.DataFrame({"disp": g.x.std(), "pnl": g.apply(lambda q: (q.w * q.mon).sum() - 0.0018), "n": g.size()})
print("weekends", len(W), "| dispersion pctiles (%):", (W.disp.quantile([.1, .25, .5, .75, .9]) * 100).round(2).to_dict())
print("corr(disp, net pnl):", round(W.disp.corr(W.pnl), 2))
W["bucket"] = pd.qcut(W.disp, 3, labels=["low", "mid", "high"])
print(W.groupby("bucket").agg(n=("pnl", "size"), disp_med=("disp", lambda v: round(v.median() * 100, 2)),
      net_bp=("pnl", lambda v: round(v.mean() * 1e4, 1)), hit=("pnl", lambda v: round((v > 0).mean(), 2))).to_string())
for thr in (0.005, 0.0075, 0.01, 0.0125):
    k = W[W.disp >= thr]; o = W[W.disp < thr]
    print(f"disp >= {thr*100:.2f}%: n {len(k):2d} net {k.pnl.mean()*1e4:+6.1f}bp t {k.pnl.mean()/k.pnl.std()*np.sqrt(len(k)):+.2f} | below: n {len(o):2d} net {o.pnl.mean()*1e4:+6.1f}bp")
print(W.assign(disp=lambda d: (d.disp * 100).round(2), pnl=lambda d: (d.pnl * 1e4).round(0)).tail(10).to_string())
