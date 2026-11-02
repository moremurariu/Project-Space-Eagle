"""r35 hour-of-day seasonality (UTC) in crypto: BTC and equal-weight 40-coin basket, IS 2019-2023 vs OOS 2024+.
Also: is there a cheap timing gain for the daily 00 UTC rebalance (execute at a better hour)?"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
P = load_panel()
r = np.log(P).diff()
ew = r.mean(axis=1)
out = {}
for lab, s in (("BTC", r["BTCUSDT"]), ("EW40", ew), ("EW40-BTC", ew - r["BTCUSDT"])):
    for per, sl in (("IS", slice("2019", "2023")), ("OOS", slice("2024", None))):
        x = s[sl].dropna()
        g = x.groupby(x.index.hour)       # bar open hour; return realised over that hour
        out[(lab, per)] = (g.mean() * 1e4, g.mean() / g.std() * np.sqrt(g.count()))
for lab in ("BTC", "EW40", "EW40-BTC"):
    m_is, t_is = out[(lab, "IS")]; m_oos, t_oos = out[(lab, "OOS")]
    print(f"\n{lab}: mean bp per hour (t)   corr(IS, OOS) of hourly means = {np.corrcoef(m_is, m_oos)[0,1]:+.2f}")
    for h in range(24):
        flag = " <- same sign, |t|>2 both" if np.sign(m_is[h]) == np.sign(m_oos[h]) and abs(t_is[h]) > 2 and abs(t_oos[h]) > 2 else ""
        print(f"  {h:02d}h  IS {m_is[h]:+5.1f} ({t_is[h]:+4.1f})  OOS {m_oos[h]:+5.1f} ({t_oos[h]:+4.1f}){flag}")
