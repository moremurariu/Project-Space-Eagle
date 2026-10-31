import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.data import binance
MAP = {"SP500": "SP500", "NASDAQ100": "XYZ100", "DCOILWTICO": "CL", "DCOILBRENTEU": "BRENTOIL",
       "DEXUSEU": "EUR", "DEXJPUS": "JPY", "DHHNGSP": "NATGAS"}
cols = {}
for f, hlname in MAP.items():
    s = pd.read_csv(f"cache/fred/{f}.csv", index_col=0, parse_dates=True).iloc[:, 0]
    s = pd.to_numeric(s, errors="coerce").dropna()
    s = s[s > 0]
    cols[hlname] = s
# TLT proxy from 10y yield: daily return ~ -dur*dy + y/252 (duration ~ 16 for TLT)
y = pd.to_numeric(pd.read_csv("cache/fred/DGS10.csv", index_col=0, parse_dates=True).iloc[:, 0], errors="coerce").dropna() / 100
tlt_r = -16 * y.diff() * 1.0 + y.shift(1) / 252
cols["TLT"] = (1 + tlt_r.fillna(0)).cumprod()
# gold proxy: PAXG daily from Binance (2020+)
try:
    px = binance.load_klines("PAXGUSDT", "1d", start="2019-01-01")
    g = px["close"]; g.index = g.index.tz_localize(None)
    cols["GOLD"] = g
except Exception as e:
    print("paxg failed", e)
D = pd.DataFrame(cols).sort_index()
D = D[D.index >= "2000-01-01"]
D = D[D.index.dayofweek < 5].ffill(limit=5)
print("history start:", {c: str(D[c].first_valid_index().date()) for c in D})
F = pd.DataFrame(0.055 / 252, index=D.index, columns=D.columns)  # ~5.5% APR financing paid by longs (HL xyz baseline)
F[["EUR", "JPY"]] = 0.0
def run(w, label):
    res = engine.run(D, w, fee=0.0009, slippage=0.0003, funding=F, min_trade=0.02)
    r = res.returns
    st = {p: engine.perf_stats(r[s], 252, res.turnover[s], res.weights[s]) for p, s in
          [("2000-09", slice("2000", "2009")), ("2010-19", slice("2010", "2019")), ("2020-26", slice("2020", None)), ("full", slice(None))]}
    print(f"== {label}")
    for k, v in st.items():
        print(f"   {k:8s} {engine.fmt_stats(v)}")
    return res
for lo in (False, True):
    for blend in (1.0, 0.5, 0.0):
        run(S.trend_daily(D, long_only=lo, blend=blend), f"TradFi trend long_only={lo} blend(brk)={blend}")
# long-only equity benchmark
w = pd.DataFrame(0.0, index=D.index, columns=D.columns); w["XYZ100"] = 1.0
run(w.where(w.index == w.index[0], np.nan), "NDX buy&hold (financed)")
