"""r59 xyz weekend fade with the crypto-equity leg (COIN/MSTR/HOOD/CRCL/STRC) hedged by a BTC perp over the holding window."""
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
from ptrade.research_utils import load_panel
btc = load_panel(["BTCUSDT"])["BTCUSDT"]; btc_c = btc.copy(); btc_c.index = btc_c.index + pd.Timedelta(hours=1)  # close-time index
CRY = {"COIN", "MSTR", "HOOD", "CRCL", "STRC"}
eq = df[df.is_eq].copy()
eq["x"] = eq.wknd - eq.groupby("fri").wknd.transform("mean")
eq["w"] = -eq.x / eq.groupby("fri").x.transform(lambda v: v.abs().sum())
eq["w"] = eq.w.clip(-0.10, 0.10); eq["w"] = eq.w / eq.groupby("fri").w.transform(lambda v: v.abs().sum())
eq["cry"] = eq.coin.str.split(":").str[1].isin(CRY)
rows2 = []
for fri, g in eq.groupby("fri"):
    dst = 20 if 3 <= fri.month <= 10 else 21
    t_sun = fri + pd.Timedelta(days=2, hours=23); t_mon = fri + pd.Timedelta(days=3, hours=dst)
    if t_sun not in btc_c.index or t_mon not in btc_c.index: continue
    rb = btc_c[t_mon] / btc_c[t_sun] - 1; wc = g.loc[g.cry, "w"].sum()
    base = (g.w * g.mon).sum() - 0.0018
    hedged = base - wc * rb - abs(wc) * 2 * 0.0005        # BTC perp hedge sized 1:1 to the net crypto-equity weight (β≈1, r33)
    rows2.append({"fri": fri, "wc": wc, "btc": rb, "base": base, "hedged": hedged})
R = pd.DataFrame(rows2).set_index("fri")
for c in ("base", "hedged"):
    x = R[c]; print(f"{c:7s} weekends {len(x)} net {x.mean()*1e4:+6.1f}bp t {x.mean()/x.std()*np.sqrt(len(x)):+.2f} std {x.std()*1e4:5.0f}bp | last10 {x.tail(10).mean()*1e4:+.0f}bp")
print("avg |net crypto-equity weight|", round(R.wc.abs().mean(), 3), "| corr(wc*btc, base pnl)", round((R.wc * R.btc).corr(R.base), 2))
