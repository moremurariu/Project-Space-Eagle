"""r32 weekend price discovery on HL xyz (24/7) equity/index perps: does the weekend move (Fri US close -> Sun night)
continue or revert into Monday's US close? Hourly HL candles (last ~5000h only)."""
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
DEMEAN = sys.argv[1] if len(sys.argv) > 1 else "mean"
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
print("weekends:", df.fri.nunique(), "obs:", len(df))
for lab, s in (("all", df), ("equity-like", df[df.is_eq]), ("non-equity", df[~df.is_eq])):
    print(f"{lab:12s} n={len(s):4d} corr(wknd, mon)={s.wknd.corr(s.mon):+.3f}  "
          f"beta={np.polyfit(s.wknd, s.mon, 1)[0]:+.3f}")
# trade: fade the weekend move (cross-sectionally demeaned) Sunday 23:00 -> Monday US close, cost 2 x 0.09%
for lab, s in (("all", df), ("equity-like", df[df.is_eq])):
    s = s.copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform(DEMEAN)
    s["y"] = s.mon - s.groupby("fri").mon.transform("mean")
    pnl = s.groupby("fri").apply(lambda g: -(np.sign(g.x) * g.y).mean() - 0.0018)
    big = s[s.wknd.abs() > 0.03]
    print(f"{lab:12s} XS fade per weekend: mean {pnl.mean()*1e4:+.1f}bp t={pnl.mean()/pnl.std()*np.sqrt(len(pnl)):+.2f} "
          f"hit {np.mean(pnl > 0):.0%} n={len(pnl)} | |wknd|>3%: n={len(big)} mean mon {(-np.sign(big.wknd)*big.mon).mean()*1e4:+.0f}bp (fade, gross)")
idx = df[df.coin.isin(["xyz:SP500", "xyz:XYZ100"])]
print("indices: n", len(idx), "corr", round(idx.wknd.corr(idx.mon), 3))
print(idx.groupby("fri")[["wknd", "mon"]].mean().round(4).tail(12))
# stability: halves, and the per-weekend cross-sectional beta (clustered by weekend)
s = df[df.is_eq].copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean"); s["y"] = s.mon - s.groupby("fri").mon.transform("mean")
b = s.groupby("fri").apply(lambda g: np.polyfit(g.x, g.y, 1)[0] if g.x.std() > 0 else np.nan).dropna()
print(f"per-weekend XS beta: mean {b.mean():+.3f} t={b.mean()/b.std()*np.sqrt(len(b)):+.2f} share<0 {np.mean(b<0):.0%}")
fr = sorted(s.fri.unique()); h = fr[len(fr)//2]
for lab, part in (("H1", s[s.fri < h]), ("H2", s[s.fri >= h])):
    pnl = part.groupby("fri").apply(lambda g: -(np.sign(g.x) * g.y).mean() - 0.0018)
    print(f"{lab}: weekends {part.fri.nunique()} corr {part.x.corr(part.y):+.3f} fade net {pnl.mean()*1e4:+.1f}bp/wk")
# linear weights w = -x / sum|x| (x demeaned by the mean -> dollar-neutral, gross 1); cost 0.18% round trip x gross
s = df[df.is_eq].copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean")
s["w"] = -s.x / s.groupby("fri").x.transform(lambda v: v.abs().sum())
for cap in (None, 0.10):
    ss = s.copy()
    if cap:
        ss["w"] = ss.w.clip(-cap, cap); ss["w"] = ss.w / ss.groupby("fri").w.transform(lambda v: v.abs().sum())
    pnl = ss.groupby("fri").apply(lambda g: (g.w * g.mon).sum() - 0.0018)
    fr = sorted(ss.fri.unique()); h = fr[len(fr)//2]
    print(f"linear cap={cap}: mean {pnl.mean()*1e4:+.1f}bp t={pnl.mean()/pnl.std()*np.sqrt(len(pnl)):+.2f} hit {np.mean(pnl>0):.0%} "
          f"ann.Sharpe {pnl.mean()/pnl.std()*np.sqrt(52):.2f} | H1 {pnl[pnl.index<h].mean()*1e4:+.1f} H2 {pnl[pnl.index>=h].mean()*1e4:+.1f}bp")
