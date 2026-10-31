"""Crash-rebound v2 (market-wide conditioning) + DVOL-spike BTC long. Lag 1h (hourly execution)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
from ptrade.data import deribit
P = load_panel(); F = pd.DataFrame(0.0001/8, index=P.index, columns=P.columns)
r4 = P / P.shift(4) - 1
btc4 = r4["BTCUSDT"]
def crash(thr=-0.10, hold=24, base=0.15, mkt_mult=1.0, idio_mult=1.0, mkt_thr=-0.03, max_gross=1.0):
    sig = r4 < thr
    mult = np.where(btc4 < mkt_thr, mkt_mult, idio_mult)
    size = sig.astype(float).mul(mult, axis=0) * base
    pos = size.rolling(hold, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); pos = pos.mul((max_gross / g).clip(upper=1).fillna(1), axis=0)
    chg = (pos.diff().abs().sum(axis=1) > 0)
    return pos.where(chg, np.nan)
def show(w, label, cols=None, lag=1):
    Pu = P if cols is None else P[cols]; Fu = F if cols is None else F[cols]
    res = engine.run(Pu, w, fee=0.00045, slippage=0.001, funding=Fu, lag=lag)
    r = res.returns
    a, b = engine.perf_stats(r[:"2023"], 8760, res.turnover[:"2023"], res.weights[:"2023"]), engine.perf_stats(r["2024":], 8760, res.turnover["2024":], res.weights["2024":])
    print(f"{label:55s} IS sh {a['sharpe']:.2f} cagr {a['cagr']*100:5.1f}% dd {a['max_dd']*100:5.1f}% | OOS sh {b['sharpe']:.2f} cagr {b['cagr']*100:5.1f}% dd {b['max_dd']*100:5.1f}% gross {b['avg_gross']:.2f}")
print("--- crash rebound variants (lag 1h)")
show(crash(), "baseline thr-10% 15%")
show(crash(thr=-0.08), "thr-8% 15%")
show(crash(mkt_mult=1.0, idio_mult=0.0), "thr-10% market-wide only (btc4h<-3%)")
show(crash(thr=-0.08, mkt_mult=1.0, idio_mult=0.0), "thr-8% market-wide only")
show(crash(mkt_mult=1.5, idio_mult=0.5), "thr-10% size 22.5% mkt / 7.5% idio")
show(crash(thr=-0.08, mkt_mult=1.5, idio_mult=0.5), "thr-8% size 22.5% mkt / 7.5% idio")
show(crash(mkt_mult=1.0, idio_mult=0.0, mkt_thr=-0.05), "thr-10% only if btc4h<-5%")
print("--- DVOL spike -> long BTC 24h")
dv = deribit.dvol_history("BTC")["close"].reindex(P.index).ffill(limit=3)
dchg = dv / dv.shift(24) - 1
for q in (0.8, 0.9):
    thr = dchg.expanding(min_periods=24 * 90).quantile(q)  # no look-ahead
    for hold in (24, 48):
        s = (dchg > thr).astype(float).rolling(hold, min_periods=1).max()
        s[dv.isna()] = 0
        w = pd.DataFrame({"BTCUSDT": s}); chg = w.diff().abs().sum(axis=1) > 0
        show(w.where(chg, np.nan), f"DVOL 24h chg > q{int(q*100)} hold {hold}h", cols=["BTCUSDT"])
