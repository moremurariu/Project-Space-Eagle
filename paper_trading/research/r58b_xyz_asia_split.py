"""r58b xyz weekend fade: Asia-listed (home session opens Mon Asia morning) vs private/no-session vs US-listed."""
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
ASIA = {"CXMT", "ZHIPU", "UNITREE", "SKHX", "SKHY", "SMSN"}   # China/HK/Korea-listed (or listing) names
PRIV = {"SPCX", "PURRDAT"}                                      # private / no exchange session
grp = df.coin.str.split(":").str[1].map(lambda n: "asia" if n in ASIA else ("private" if n in PRIV else "us"))
df["grp"] = grp
eq = df[df.is_eq]
s2 = eq.copy(); s2["x"] = s2.wknd - s2.groupby("fri").wknd.transform("mean"); s2["y"] = s2.mon - s2.groupby("fri").mon.transform("mean")
for g in ("us", "asia", "private"):
    part = s2[s2.grp == g]
    b = part.groupby("fri").apply(lambda q: np.polyfit(q.x, q.y, 1)[0] if len(q) >= 2 and q.x.std() > 0 else np.nan).dropna()
    print(f"{g:8s} n {len(part):4d} | pooled beta {np.polyfit(part.x, part.y, 1)[0]:+.3f} corr {part.x.corr(part.y):+.3f} | "
          f"per-weekend beta mean {b.mean():+.3f} (t {b.mean()/b.std()*np.sqrt(len(b)):+.2f}, {len(b)} wk) | last 10 wk pooled corr "
          f"{part[part.fri >= sorted(part.fri.unique())[-10]].pipe(lambda q: q.x.corr(q.y)):+.3f}")
