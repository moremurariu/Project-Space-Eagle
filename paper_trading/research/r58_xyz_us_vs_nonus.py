"""r58 xyz weekend fade: US-listed names vs names without a US cash session (private/pre-IPO, Korea/China-listed)."""
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
NONUS = {"CXMT", "ZHIPU", "SPCX", "PURRDAT", "UNITREE", "SKHX", "SKHY", "SMSN"}
df["nonus"] = df.coin.str.split(":").str[1].isin(NONUS)
def fade(s, cap=0.10):
    s = s.copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean")
    s["w"] = -s.x / s.groupby("fri").x.transform(lambda v: v.abs().sum())
    s["w"] = s.w.clip(-cap, cap); s["w"] = s.w / s.groupby("fri").w.transform(lambda v: v.abs().sum())
    pnl = s.groupby("fri").apply(lambda g: (g.w * g.mon).sum() - 0.0018)
    pnl = pnl[s.groupby("fri").size() >= 5]
    return pnl
eq = df[df.is_eq]
for lab, s in (("all equity-like", eq), ("US-listed only", eq[~eq.nonus]), ("non-US / private only", eq[eq.nonus])):
    p = fade(s, cap=0.10 if lab != "non-US / private only" else 0.35)
    fr = sorted(p.index); h = fr[len(fr) // 2]
    print(f"{lab:22s} weekends {len(p):2d} names/wk {s.groupby('fri').size().median():.0f} | net {p.mean()*1e4:+6.1f}bp t {p.mean()/p.std()*np.sqrt(len(p)):+.2f} "
          f"hit {np.mean(p > 0):.0%} | H1 {p[p.index < h].mean()*1e4:+.0f} H2 {p[p.index >= h].mean()*1e4:+.0f}bp | last10 {p.tail(10).mean()*1e4:+.0f}bp")
s = eq.copy(); s["x"] = s.wknd - s.groupby("fri").wknd.transform("mean"); s["y"] = s.mon - s.groupby("fri").mon.transform("mean")
for lab, part in (("US-listed", s[~s.nonus]), ("non-US/private", s[s.nonus])):
    print(f"pooled XS beta (Monday on weekend move) {lab:15s}: {np.polyfit(part.x, part.y, 1)[0]:+.3f}  corr {part.x.corr(part.y):+.3f}  n {len(part)}")
