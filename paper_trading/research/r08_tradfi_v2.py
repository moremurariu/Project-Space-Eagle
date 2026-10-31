"""TradFi retest with Yahoo history (gold/silver/copper/S&P/NDX/DXY/VIX) + FRED (oil, FX, natgas)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.strategies import signals as S

def yahoo(f):
    d = pd.read_csv(f"data_imports/yahoo/{f}.csv", parse_dates=["date"]).set_index("date")["close"]
    return d[d > 0].astype(float)
def fred(f):
    s = pd.to_numeric(pd.read_csv(f"cache/fred/{f}.csv", index_col=0, parse_dates=True).iloc[:, 0], errors="coerce").dropna()
    return s[s > 0]
cols = {"GOLD": yahoo("GC_F"), "SILVER": yahoo("SI_F"), "COPPER": yahoo("HG_F"), "SP500": yahoo("GSPC"),
        "XYZ100": yahoo("NDX"), "DXY": yahoo("DX-Y_NYB"), "CL": fred("DCOILWTICO"), "BRENTOIL": fred("DCOILBRENTEU"),
        "EUR": fred("DEXUSEU"), "JPY": fred("DEXJPUS"), "NATGAS": fred("DHHNGSP")}
D = pd.DataFrame(cols).sort_index()
D = D[(D.index >= "2000-01-01") & (D.index.dayofweek < 5)].ffill(limit=5)
F = pd.DataFrame(0.055 / 252, index=D.index, columns=D.columns); F[["EUR", "JPY", "DXY"]] = 0.0
PER = [("2000-09", slice("2000", "2009")), ("2010-19", slice("2010", "2019")), ("2020-26", slice("2020", None)), ("full", slice(None))]

def report(res, label, per=PER, ppy=252):
    print(f"== {label}")
    for k, s in per:
        print(f"   {k:8s} {engine.fmt_stats(engine.perf_stats(res.returns[s], ppy, res.turnover[s], res.weights[s]))}")

print("### A. Trend, 11 markets")
for lo in (False, True):
    for blend in (1.0, 0.5):
        res = engine.run(D, S.trend_daily(D, long_only=lo, blend=blend), fee=0.0009, slippage=0.0003, funding=F, min_trade=0.02)
        report(res, f"trend 11 mkts long_only={lo} blend={blend}")
print("### A2. Trend, metals only (gold/silver/copper)")
M = D[["GOLD", "SILVER", "COPPER"]]
for lo in (False, True):
    res = engine.run(M, S.trend_daily(M, long_only=lo, blend=0.5), fee=0.0009, slippage=0.0003, funding=F[M.columns], min_trade=0.02)
    report(res, f"metals trend long_only={lo}")

print("### B. S&P 500 turn-of-month (long last day + first 3 days of month), 1928-2026")
spx = yahoo("GSPC"); spx = spx[spx.index >= "1928-01-01"]
r = spx.pct_change().fillna(0)
ym = spx.index.to_period("M")
pos_in_month = pd.Series(1, index=spx.index).groupby(ym).cumcount()
n_in_month = pd.Series(1, index=spx.index).groupby(ym).transform("size")
from_end = n_in_month - pos_in_month - 1
# signal decided at close t, earns return t+1: hold over days that are last-day (from_end==0) .. 3rd day (pos 2)
hold_next = ((from_end == 1) | (from_end == 0) | (pos_in_month <= 1)).shift(0)  # in market for next day's return
tom = hold_next.shift(1).fillna(False).astype(float)
cost = 0.0012 * tom.diff().abs().fillna(0)  # 0.09% fee + 0.03% slip per switch
fin = 0.055 / 252 * tom
tom_r = tom * r - cost - fin
PER2 = [("1928-59", slice("1928", "1959")), ("1960-89", slice("1960", "1989")), ("1990-09", slice("1990", "2009")),
        ("2010-26", slice("2010", None))]
for k, s in PER2:
    st = engine.perf_stats(tom_r[s], 252); bh = engine.perf_stats(r[s], 252)
    print(f"   {k}: TOM {engine.fmt_stats(st)} | in-mkt {tom[s].mean():.0%} | B&H sharpe {bh['sharpe']:.2f} cagr {bh['cagr']*100:.1f}%")
print("   mean daily ret in TOM window vs other days (bp):",
      {k: (round(r[s][tom[s] > 0].mean() * 1e4, 1), round(r[s][tom[s] == 0].mean() * 1e4, 1)) for k, s in PER2})

print("### C. S&P long when VIX elevated vs its 1y median (fear premium)")
vix = yahoo("VIX")
spx2 = spx[spx.index >= "1990-01-01"]; r2 = spx2.pct_change().fillna(0)
v = vix.reindex(spx2.index).ffill()
for thr in (1.0, 1.2, 1.5):
    sig = (v > thr * v.rolling(252, min_periods=120).median()).astype(float)
    pos = sig.shift(1).fillna(0)
    rr = pos * r2 - 0.0012 * pos.diff().abs().fillna(0) - 0.055 / 252 * pos
    out = [f"{k}: sh {engine.perf_stats(rr[s],252)['sharpe']:.2f} cagr {engine.perf_stats(rr[s],252)['cagr']*100:.1f}% in {pos[s].mean():.0%}"
           for k, s in [("1990-09", slice("1990", "2009")), ("2010-26", slice("2010", None))]]
    print(f"   VIX > {thr}x median: " + " | ".join(out))
print("   fwd 20d S&P return by VIX level (%):")
fwd = spx2.shift(-20) / spx2 - 1
b = pd.cut(v, [0, 15, 20, 25, 30, 40, 100])
for k, s in [("1990-09", slice("1990", "2009")), ("2010-26", slice("2010", None))]:
    g = fwd[s].groupby(b[s]).agg(["mean", "count"]); g["mean"] = (g["mean"] * 100).round(2)
    print(f"   {k}:", dict(zip(g.index.astype(str), zip(g["mean"], g["count"]))))
