import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
P = load_panel(); F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
r4 = P / P.shift(4) - 1; r24 = P / P.shift(24) - 1
btc = P["BTCUSDT"]; bt_tr = (btc / btc.rolling(24 * 50).mean() - 1)
mkt = (r4["BTCUSDT"] < -0.03).to_numpy()[:, None]
notflat = (bt_tr.abs() > 0.05).to_numpy()[:, None]
def crash(market_wide=False, regime=False, deep_mult=1.0, base=0.15):
    sig = (r4 < -0.10)
    if market_wide: sig = sig & mkt
    if regime: sig = sig & notflat
    size = sig.astype(float) * base * np.where(r24 < -0.20, deep_mult, 1.0)
    pos = size.rolling(24, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    return pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
def show(label, **kw):
    res = engine.run(P, crash(**kw), fee=0.00045, slippage=0.001, funding=F, lag=1)
    r = res.returns
    a, b = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":], 8760, res.turnover["2024":], res.weights["2024":])
    yr = {str(k): round(v * 100) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{label:40s} IS sh {a['sharpe']:.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:.2f} cagr {b['cagr']*100:4.0f}% dd {b['max_dd']*100:4.0f}% | {yr}")
show("baseline (live crash_rebound)")
show("+ regime filter (skip BTC flat)", regime=True)
show("+ deep x1.5 (24h<-20%)", deep_mult=1.5)
show("+ regime + deep x1.5", regime=True, deep_mult=1.5)
show("market-wide (live crash_rebound_mkt)", market_wide=True)
show("market-wide + regime", market_wide=True, regime=True)
show("market-wide + regime + deep x1.5", market_wide=True, regime=True, deep_mult=1.5)
