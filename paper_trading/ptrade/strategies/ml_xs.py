"""Cross-sectional ML model (LightGBM): daily features from hourly Binance spot data -> next-24h relative return.

Features use only data available at the daily 00:00 UTC close (no look-ahead); the target is the next 24h
return minus the cross-sectional mean. Walk-forward: model for year Y is trained on all data before Y.
"""
from __future__ import annotations

import numpy as np
import pandas as pd

from .signals import daily_to_hourly, to_daily

FEATURES = ["r1", "r3", "r7", "r14", "r30", "r60", "vol7", "vol30", "r1_v", "r7_v", "volz1", "volz7",
            "taker1", "taker7", "beta30", "resid7", "dhi30", "dlo30", "btc_r7", "btc_vol30"]


def build_features(close_h: pd.DataFrame, qvol_h: pd.DataFrame, taker_h: pd.DataFrame, hour: int = 0) -> pd.DataFrame:
    """Long-format frame indexed by (date, symbol) with FEATURES (+ 'fwd' target where available)."""
    d = to_daily(close_h, hour)
    qv = to_daily(qvol_h.rolling(24, min_periods=20).sum(), hour)
    tk = to_daily(taker_h.rolling(24, min_periods=20).sum(), hour)
    lr = np.log(d).diff()
    hr = np.log(close_h).diff()
    hv7 = to_daily(hr.rolling(24 * 7, min_periods=24 * 5).std(), hour) * np.sqrt(24)
    hv30 = to_daily(hr.rolling(24 * 30, min_periods=24 * 20).std(), hour) * np.sqrt(24)
    f = {}
    for k in (1, 3, 7, 14, 30, 60):
        f[f"r{k}"] = np.log(d / d.shift(k))
    f["vol7"], f["vol30"] = hv7, hv30
    f["r1_v"] = f["r1"] / hv30
    f["r7_v"] = f["r7"] / (hv30 * np.sqrt(7))
    lqv = np.log(qv.replace(0, np.nan))
    f["volz1"] = lqv - lqv.rolling(30, min_periods=20).mean()
    f["volz7"] = lqv.rolling(7).mean() - lqv.rolling(30, min_periods=20).mean()
    f["taker1"] = tk / qv
    f["taker7"] = tk.rolling(7).sum() / qv.rolling(7).sum()
    btc = lr["BTCUSDT"]
    cov = lr.rolling(30, min_periods=20).cov(btc)
    beta = cov.div(btc.rolling(30, min_periods=20).var(), axis=0)
    f["beta30"] = beta
    f["resid7"] = (lr - beta.mul(btc, axis=0)).rolling(7).sum()
    f["dhi30"] = np.log(d / d.rolling(30, min_periods=20).max())
    f["dlo30"] = np.log(d / d.rolling(30, min_periods=20).min())
    f["btc_r7"] = pd.DataFrame({c: f["r7"]["BTCUSDT"] for c in d.columns})
    f["btc_vol30"] = pd.DataFrame({c: hv30["BTCUSDT"] for c in d.columns})
    fwd = np.log(d.shift(-1) / d)
    fwd = fwd.sub(fwd.mean(axis=1), axis=0)
    parts = {k: v.stack(future_stack=True) for k, v in f.items()}
    parts["fwd"] = fwd.stack(future_stack=True)
    X = pd.DataFrame(parts)
    X.index.names = ["date", "sym"]
    X = X.dropna(subset=["r30", "vol30"])
    # cross-sectional rank-normalise the per-coin features (market features left raw)
    per_coin = [c for c in FEATURES if not c.startswith("btc_")]
    X[per_coin] = X.groupby(level="date")[per_coin].rank(pct=True) - 0.5
    X["n"] = X.groupby(level="date")["r1"].transform("size")
    return X[X["n"] >= 10]


PARAMS = dict(objective="regression", learning_rate=0.03, num_leaves=15, min_data_in_leaf=400,
              feature_fraction=0.7, bagging_fraction=0.7, bagging_freq=1, lambda_l2=10.0, verbose=-1, seed=7)


def train(X: pd.DataFrame, rounds: int = 300):
    import lightgbm as lgb
    y = X["fwd"].groupby(level="date").rank(pct=True) - 0.5  # rank target: robust to outliers
    ds = lgb.Dataset(X[FEATURES], label=y)
    return lgb.train(PARAMS, ds, num_boost_round=rounds)


def weights_from_scores(scores: pd.Series, quantile: float = 0.2, gross: float = 1.0) -> pd.Series:
    """Equal-weight long top / short bottom quantile."""
    s = scores.dropna()
    k = max(1, int(round(len(s) * quantile)))
    r = s.sort_values()
    w = pd.Series(0.0, index=s.index)
    w[r.index[-k:]] = gross / 2 / k
    w[r.index[:k]] = -gross / 2 / k
    return w


def walk_forward_weights(close_h, qvol_h, taker_h, first_year=2022, quantile=0.2, gross=1.0, rebalance_d=1,
                         smooth_d=1):
    X = build_features(close_h, qvol_h, taker_h)
    dates = X.index.get_level_values("date")
    out = {}
    last_year = dates.max().year
    for yr in range(first_year, last_year + 1):
        tr = X[(dates < pd.Timestamp(f"{yr}-01-01", tz="UTC") - pd.Timedelta(days=2)) & X["fwd"].notna()]
        te = X[(dates >= pd.Timestamp(f"{yr}-01-01", tz="UTC")) & (dates < pd.Timestamp(f"{yr + 1}-01-01", tz="UTC"))]
        if len(tr) < 5000 or not len(te):
            continue
        m = train(tr)
        sc = pd.Series(m.predict(te[FEATURES]), index=te.index).unstack("sym")
        if smooth_d > 1:
            sc = sc.rolling(smooth_d, min_periods=1).mean()
        for i, (dt, row) in enumerate(sc.iterrows()):
            if i % rebalance_d:
                continue
            out[dt] = weights_from_scores(row, quantile, gross)
    W = pd.DataFrame(out).T.reindex(columns=close_h.columns).fillna(0.0)
    W.index.name = None
    return daily_to_hourly(W, close_h.index)
