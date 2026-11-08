"""r46 cross-sectional low-risk anomalies: low-vol, MAX (lottery), betting-against-beta. Weekly, 7 staggered offsets,
quartile legs, real Binance funding paid, 0.045% fee + 5bp slippage. Dollar-neutral and beta-neutral variants."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P)
d = S.to_daily(P, 0); r = d.pct_change(fill_method=None)
mk = r.mean(axis=1)
vol = r.rolling(30, min_periods=20).std()
mx = r.rolling(30, min_periods=20).max()
beta = r.rolling(60, min_periods=40).cov(mk).div(mk.rolling(60, min_periods=40).var(), axis=0)
def legs(score, q=0.25, bn=False, min_assets=8):
    out = pd.DataFrame(0.0, index=d.index, columns=d.columns)
    for t in d.index:
        s = score.loc[t].dropna()
        if len(s) < min_assets: continue
        k = max(1, int(round(len(s) * q))); o = s.sort_values()
        L, Sh = o.index[-k:], o.index[:k]
        iv = 1 / vol.loc[t]
        lw, sw = iv[L] / iv[L].sum(), iv[Sh] / iv[Sh].sum()
        if bn:  # scale legs so long-beta == short-beta (gross 1)
            b = beta.loc[t].clip(0.2, 3).fillna(1)
            bl, bs = (lw * b[L]).sum(), (sw * b[Sh]).sum()
            kl, ks = bs / (bl + bs), bl / (bl + bs)
            lw, sw = lw * kl * 2, sw * ks * 2
        out.loc[t, L] = lw.values / 2; out.loc[t, Sh] = -sw.values / 2
    return out
def stag(score, bn):
    W = legs(score, bn=bn)
    ws = []
    for o in range(7):
        m = W.copy(); keep = (np.arange(len(m)) - o) % 7 == 0
        m.loc[~keep] = np.nan; ws.append(m.ffill().fillna(0))
    return S.daily_to_hourly(sum(ws) / 7, P.index)
def st(label, x):
    a, b = engine.perf_stats(x["2019":"2023"]), engine.perf_stats(x["2024":])
    print(f"{label:34s} IS sh {a['sharpe']:5.2f} cagr {a['cagr']*100:6.1f}% dd {a['max_dd']*100:6.1f}% | OOS sh {b['sharpe']:5.2f} cagr {b['cagr']*100:6.1f}% dd {b['max_dd']*100:6.1f}%", flush=True)
    return x
res = {}
for name, sc in (("low-vol", -vol), ("low-MAX", -mx), ("low-beta (BAB)", -beta)):
    for bn in (False, True):
        w = stag(sc, bn)
        res[f"{name}{' bn' if bn else ''}"] = st(f"{name}{' beta-neutral' if bn else ' $-neutral'}",
            engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
rm = engine.run(P, wm, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
D = pd.DataFrame({**res, "xs_mom": rm}).resample("D").sum()
print("daily corr vs xs_mom:", D.corr()["xs_mom"].round(2).to_dict())
