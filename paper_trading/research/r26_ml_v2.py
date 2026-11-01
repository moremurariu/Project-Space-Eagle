"""ML cross-sectional v2: vol-scaled simple-return target (7d), weekly rebalance, true inverse-vol sizing."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd, lightgbm as lgb
from ptrade.data import binance
from ptrade.universe import UNIVERSE
from ptrade.backtest import engine
from ptrade.strategies import ml_xs
from ptrade.strategies.signals import to_daily, daily_to_hourly
dfs = binance.load_many(list(UNIVERSE), "1h", start="2019-01-01", refresh=False)
idx = pd.date_range(pd.Timestamp("2019-01-01", tz="UTC"), max(v.index[-1] for v in dfs.values()), freq="h")
C = binance.panel(dfs, "close").reindex(idx).ffill(limit=3)
X = ml_xs.build_features(C, binance.panel(dfs, "quote_volume").reindex(idx), binance.panel(dfs, "taker_buy_quote").reindex(idx))
d = to_daily(C)
H = 7
fwd = (d.shift(-H) / d - 1)
dvol = d.pct_change().rolling(30, min_periods=20).std() * np.sqrt(H)
tgt = (fwd.sub(fwd.mean(axis=1), axis=0) / dvol).clip(-3, 3)
X["tgt"] = tgt.stack(future_stack=True).reindex(X.index)
X["dvol"] = (d.pct_change().rolling(30, min_periods=20).std()).stack(future_stack=True).reindex(X.index)
dates = X.index.get_level_values("date")
F = pd.DataFrame(0.0001 / 8, index=C.index, columns=C.columns)
params = dict(objective="regression", learning_rate=0.03, num_leaves=15, min_data_in_leaf=400, feature_fraction=0.7,
              bagging_fraction=0.7, bagging_freq=1, lambda_l2=10.0, verbose=-1, seed=7)
for q in (0.2, 0.3):
    W = {}
    for yr in range(2021, 2027):
        tr = X[(dates < pd.Timestamp(f"{yr}-01-01", tz="UTC") - pd.Timedelta(days=H + 1)) & X.tgt.notna()]
        te = X[dates.year == yr]
        m = lgb.train(params, lgb.Dataset(tr[ml_xs.FEATURES], label=tr.tgt), num_boost_round=300)
        sc = pd.Series(m.predict(te[ml_xs.FEATURES]), index=te.index).unstack("sym")
        vol = te["dvol"].unstack("sym")
        for i, (dt, row) in enumerate(sc.iterrows()):
            if i % H: continue
            s = row.dropna(); k = max(1, int(round(len(s) * q))); r = s.sort_values()
            iv = 1 / vol.loc[dt].reindex(s.index)
            w = pd.Series(0.0, index=C.columns)
            w[r.index[-k:]] = (iv[r.index[-k:]] / iv[r.index[-k:]].sum() * 0.5).values
            w[r.index[:k]] = (-iv[r.index[:k]] / iv[r.index[:k]].sum() * 0.5).values
            W[dt] = w.fillna(0)
    Wd = pd.DataFrame(W).T; Wh = daily_to_hourly(Wd, C.index)
    res = engine.run(C, Wh, fee=0.00045, slippage=0.0005, funding=F)
    r = res.returns["2021":]
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    st = engine.perf_stats(r, 8760, res.turnover["2021":], res.weights["2021":])
    print(f"ML v2 q={q} weekly: {engine.fmt_stats(st)}\n   yearly {yr}")
