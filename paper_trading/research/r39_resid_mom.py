"""r39 XS momentum variants: raw vs BTC-residual vs vol-scaled ranking (7/14/21d ensemble, staggered weekly)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P)
d = S.to_daily(P); rets = d.pct_change(fill_method=None); vol = S.ewm_vol(rets, 30)
rb = rets["BTCUSDT"]
beta = rets.rolling(60, min_periods=30).cov(rb).div(rb.rolling(60, min_periods=30).var(), axis=0)
def score(kind, lb):
    if kind == "raw":
        return d / d.shift(lb) - 1
    if kind == "resid":
        res = rets - beta.shift(1).mul(rb, axis=0)
        return res.rolling(lb).sum()
    if kind == "volscaled":
        return (d / d.shift(lb) - 1) / (vol * np.sqrt(lb))
def weights(sc, offset, q=0.25):
    out = pd.DataFrame(np.nan, index=d.index, columns=d.columns)
    for i, t in enumerate(d.index):
        if (i - offset) % 7:
            continue
        s = sc.loc[t].dropna()
        if len(s) < 8:
            out.loc[t] = 0.0; continue
        k = max(1, int(round(len(s) * q))); r = s.sort_values()
        longs, shorts = r.index[-k:], r.index[:k]
        iv = 1 / vol.loc[t]; row = pd.Series(0.0, index=d.columns)
        row[longs] = (iv[longs] / iv[longs].sum()) / 2; row[shorts] = -(iv[shorts] / iv[shorts].sum()) / 2
        out.loc[t] = row.fillna(0.0)
    return out.ffill().fillna(0.0)
def ens(kind):
    w = sum(weights(score(kind, lb), o) for lb in (7, 14, 21) for o in range(7)) / 21
    return S.daily_to_hourly(w, P.index).ffill()
def st(label, r):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    yr = {str(k)[2:]: round(v * 100) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{label:28s} IS sh {a['sharpe']:5.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} dd {b['max_dd']*100:4.0f}% {yr}", flush=True)
run = lambda w: engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
R = {}
for kind in ("raw", "resid", "volscaled"):
    R[kind] = run(ens(kind)); st(f"ensemble {kind}", R[kind])
rf = run(S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24))
D = pd.DataFrame({**R, "fund": rf}).resample("D").sum(); print(D.corr().round(2))
for kind in R:
    st(f"2/3 funding + 1/3 {kind}", 2 / 3 * rf + 1 / 3 * R[kind])
