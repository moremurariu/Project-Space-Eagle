"""r61 turn-of-the-month effect in crypto: daily returns on the last 2 + first 3 calendar days of each month vs the rest.
BTC and the equal-weight 40 (Binance, 00 UTC daily closes). Strategy = long only in the TOM window (cost 0.19%/round trip)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.strategies import signals as S
P = load_panel(); d = S.to_daily(P, 0); r = d.pct_change(fill_method=None)
series = {"BTC": r["BTCUSDT"], "EW40": r.mean(axis=1)}
day = r.index - pd.Timedelta(days=1)          # return of row t is the calendar day t-1 (close-to-close at 00 UTC)
dom = day.day; dim = day.days_in_month
tom = (dom <= 3) | (dom > dim - 2)
for name, x in series.items():
    x = x.dropna()
    t = pd.Series(tom, index=r.index).reindex(x.index)
    for per, sl in (("IS 2019-23", slice("2019", "2023")), ("OOS 2024+", slice("2024", None))):
        xs, ts = x[sl], t[sl]
        a, b = xs[ts], xs[~ts]
        diff = a.mean() - b.mean(); se = np.sqrt(a.var() / len(a) + b.var() / len(b))
        months = xs[ts].groupby([xs[ts].index.year, (xs[ts].index - pd.Timedelta(days=4)).month]).apply(lambda v: (1 + v).prod() - 1)
        strat = months - 0.0019
        print(f"{name:4s} {per}: TOM mean {a.mean()*1e4:+6.1f}bp/day (n {len(a)}) vs rest {b.mean()*1e4:+6.1f}bp (n {len(b)}) | diff t {diff/se:+.2f} "
              f"| TOM-only long: {strat.mean()*1e4:+.0f}bp/month net, hit {np.mean(strat > 0):.0%}, {len(strat)} windows")
