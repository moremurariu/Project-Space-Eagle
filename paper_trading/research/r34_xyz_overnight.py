"""r34 does the r32 weekend overreaction on HL xyz single-stock perps also exist on weeknights?
Window: US close -> entry hour E (UTC) the next session day; then E -> next US close. Linear XS fade, ≤10%/name."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import CACHE
P = pd.read_parquet(CACHE / "xyz_1h.parquet")
NON_EQ = {"GOLD", "SILVER", "CL", "BRENTOIL", "COPPER", "NATGAS", "PLATINUM", "PALLADIUM", "JPY", "EUR"}
cols = [c for c in P.columns if c.split(":")[1] not in NON_EQ]
P = P[cols]
def ny_close_bar(day):
    return pd.Timestamp(day.year, day.month, day.day, 16, tz="America/New_York").tz_convert("UTC") - pd.Timedelta(hours=1)
def study(entry_h, weekend, cost=0.0018):
    out = []
    for d in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="B"):
        nxt = d + pd.offsets.BDay(1)
        if (nxt.weekday() == 0) != weekend:
            continue
        t0, t2 = ny_close_bar(d), ny_close_bar(nxt)
        t1 = pd.Timestamp(nxt.year, nxt.month, nxt.day, tz="UTC") + pd.Timedelta(hours=entry_h - 1)
        if entry_h == 23:
            t1 = pd.Timestamp(nxt.year, nxt.month, nxt.day, tz="UTC") - pd.Timedelta(hours=2)   # 23:00 UTC the day before
        if not all(t in P.index for t in (t0, t1, t2)):
            continue
        a, b, e = P.loc[t0], P.loc[t1], P.loc[t2]
        ok = a.notna() & b.notna() & e.notna() & (a > 0)
        if ok.sum() < 10:
            continue
        x = (b[ok] / a[ok] - 1); x = x - x.mean(); y = e[ok] / b[ok] - 1
        w = (-x / x.abs().sum()).clip(-0.1, 0.1); w = w / w.abs().sum()
        out.append({"d": d, "pnl": float((w * y).sum()) - cost, "beta": np.polyfit(x, y - y.mean(), 1)[0]})
    df = pd.DataFrame(out).set_index("d")
    h = df.index[len(df) // 2]
    return (f"n={len(df):3d} beta {df.beta.mean():+.2f} (t {df.beta.mean()/df.beta.std()*np.sqrt(len(df)):+.1f}) | net "
            f"{df.pnl.mean()*1e4:+.1f}bp t={df.pnl.mean()/df.pnl.std()*np.sqrt(len(df)):+.2f} hit {np.mean(df.pnl>0):.0%} | "
            f"H1 {df.pnl[:h].mean()*1e4:+.1f} H2 {df.pnl[h:].mean()*1e4:+.1f}")
for wk in (True, False):
    for E in (23, 8, 13):
        print(f"{'weekend ' if wk else 'weeknight'} entry {E:02d}:00 UTC  {study(E, wk)}")

# weekend: vary entry (Sunday hour) and exit (Monday hour, or US close)
def wk(entry_sun_h, exit_mon_h, cost=0.0018):
    out = []
    for fri in pd.date_range(P.index[0].normalize(), P.index[-1].normalize(), freq="W-FRI"):
        mon = fri + pd.Timedelta(days=3)
        t0 = ny_close_bar(fri)
        t1 = fri + pd.Timedelta(days=2, hours=entry_sun_h - 1)
        t2 = ny_close_bar(mon) if exit_mon_h is None else mon + pd.Timedelta(hours=exit_mon_h - 1)
        if not all(t in P.index for t in (t0, t1, t2)):
            continue
        a, b, e = P.loc[t0], P.loc[t1], P.loc[t2]
        ok = a.notna() & b.notna() & e.notna() & (a > 0)
        x = (b[ok] / a[ok] - 1); x = x - x.mean(); y = e[ok] / b[ok] - 1
        w = (-x / x.abs().sum()).clip(-0.1, 0.1); w = w / w.abs().sum()
        out.append(float((w * y).sum()) - cost)
    s = pd.Series(out); h = len(s) // 2
    return f"n={len(s)} net {s.mean()*1e4:+6.1f}bp t={s.mean()/s.std()*np.sqrt(len(s)):+.2f} H1 {s[:h].mean()*1e4:+6.1f} H2 {s[h:].mean()*1e4:+6.1f}"
print("--- weekend entry/exit grid")
for en in (18, 21, 23):
    for ex in (3, 8, 13, None):
        print(f"entry Sun {en:02d}:00  exit {'Mon '+format(ex,'02d')+':00' if ex is not None else 'Mon US close'}  {wk(en, ex)}")
