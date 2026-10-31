import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.data import binance
from ptrade.universe import UNIVERSE
from ptrade.strategies import ml_xs
dfs = binance.load_many(list(UNIVERSE), "1h", start="2019-01-01", refresh=False)
idx = pd.date_range(pd.Timestamp("2019-01-01", tz="UTC"), max(v.index[-1] for v in dfs.values()), freq="h")
C = binance.panel(dfs, "close").reindex(idx).ffill(limit=3)
X = ml_xs.build_features(C, binance.panel(dfs, "quote_volume").reindex(idx), binance.panel(dfs, "taker_buy_quote").reindex(idx))
dates = X.index.get_level_values("date")
ics = {}
for yr in range(2021, 2027):
    tr = X[(dates < pd.Timestamp(f"{yr}-01-01", tz="UTC") - pd.Timedelta(days=2)) & X.fwd.notna()]
    te = X[(dates.year == yr) & X.fwd.notna()]
    m = ml_xs.train(tr)
    sc = pd.Series(m.predict(te[ml_xs.FEATURES]), index=te.index)
    ic = pd.DataFrame({"s": sc, "f": te.fwd}).groupby(level="date").apply(lambda g: g.s.corr(g.f, method="spearman"))
    # gross daily long-short top/bottom quintile spread (equal weight)
    def spread(g):
        q = g.s.rank(pct=True); return g.f[q > 0.8].mean() - g.f[q <= 0.2].mean()
    sp = pd.DataFrame({"s": sc, "f": te.fwd}).groupby(level="date").apply(spread)
    ics[yr] = (round(ic.mean(), 4), round(ic.mean() / ic.std() * np.sqrt(365), 2), round(sp.mean() * 1e4, 1), round(sp.mean() / sp.std() * np.sqrt(365), 2))
print("year: (mean daily rank IC, IC t-ann, gross Q5-Q1 spread bp/day, gross spread Sharpe)")
for k, v in ics.items(): print(k, v)
