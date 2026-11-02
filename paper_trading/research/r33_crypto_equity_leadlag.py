"""r33 do HL xyz crypto-equities (COIN, MSTR, HOOD, CRCL) under-react to BTC moves while US cash is closed?
Overnight/weekend window: US close -> 23:00 UTC before the next session (Sun for weekends); next = -> next US close."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
from ptrade.data import hyperliquid as hl
P = pd.read_parquet(CACHE / "xyz_1h.parquet")
btc = hl.candles("BTC", "1h")["close"]
names = [c for c in ("xyz:COIN", "xyz:MSTR", "xyz:HOOD", "xyz:CRCL") if c in P]
rows = []
for d in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="B"):  # each session close day
    nxt = d + pd.offsets.BDay(1)
    def close_t(day):
        ny = pd.Timestamp(day.year, day.month, day.day, 16, tz="America/New_York").tz_convert("UTC")
        return ny - pd.Timedelta(hours=1)          # bar opening 1h before the close
    t0, t2 = close_t(d), close_t(nxt)
    t1 = \
        pd.Timestamp(nxt.year, nxt.month, nxt.day, tz="UTC") - pd.Timedelta(hours=1)   # 23:00 UTC bar before next session
    if not all(t in P.index and t in btc.index for t in (t0, t1, t2)):
        continue
    b_on = btc[t1] / btc[t0] - 1
    for c in names:
        a = P.at[t0, c]; b = P.at[t1, c]; e = P.at[t2, c]
        if np.isfinite([a, b, e]).all():
            rows.append({"d": d, "coin": c, "wkend": nxt.weekday() == 0, "btc_on": b_on, "on": b / a - 1, "next": e / b - 1,
                         "btc_next": btc[t2] / btc[t1] - 1})
df = pd.DataFrame(rows)
print("sessions", df.d.nunique(), "obs", len(df))
for lab, s in (("all nights", df), ("weekends", df[df.wkend]), ("weeknights", df[~df.wkend])):
    beta_on = np.polyfit(s.btc_on, s.on, 1)[0]
    s = s.assign(resid=s.on - beta_on * s.btc_on, next_x=s.next - s.btc_next * np.polyfit(s.btc_next, s.next, 1)[0])
    X = np.column_stack([np.ones(len(s)), s.btc_on, s.resid])
    coef, *_ = np.linalg.lstsq(X, s.next, rcond=None)
    res = s.next - X @ coef
    se = np.sqrt(np.diag(np.linalg.inv(X.T @ X)) * res.var())
    print(f"{lab:11s} n={len(s):4d} on-beta to BTC {beta_on:.2f} | next ~ btc_on {coef[1]:+.3f} (t {coef[1]/se[1]:+.1f}), "
          f"resid {coef[2]:+.3f} (t {coef[2]/se[2]:+.1f})")
