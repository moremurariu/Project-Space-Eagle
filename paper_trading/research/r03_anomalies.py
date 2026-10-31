import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.data import sentiment
P = load_panel()
btc = P["BTCUSDT"]; r = btc.pct_change()
print("=== BTC mean hourly return by hour-of-day (bp), IS 2019-23 vs OOS 2024-26")
df = pd.DataFrame({"r": r, "h": r.index.hour})
is_ = df[:"2023"].groupby("h").r.mean()*1e4; oos = df["2024":].groupby("h").r.mean()*1e4
t_is = df[:"2023"].groupby("h").r.apply(lambda x: x.mean()/x.std()*np.sqrt(len(x)))
print(pd.DataFrame({"is_bp": is_.round(2), "t_is": t_is.round(2), "oos_bp": oos.round(2)}).T.to_string())
print("corr IS vs OOS hour profile:", round(np.corrcoef(is_, oos)[0,1],3))
print("=== Day of week mean daily return (%)")
d = btc.resample("D").last().pct_change()
print(pd.DataFrame({"is": d[:"2023"].groupby(d[:"2023"].index.dayofweek).mean()*100, "oos": d["2024":].groupby(d["2024":].index.dayofweek).mean()*100}).round(3).T.to_string())

print("=== Crash rebound: after X% drop over 4h in asset, fwd return next 24h (all assets pooled, bp)")
r4 = P/P.shift(4)-1; f24 = P.shift(-24)/P-1; f6 = P.shift(-6)/P-1
for thr in (-0.05,-0.08,-0.12):
    m = r4 < thr
    for per, sl in (("IS", slice(None,"2023")), ("OOS", slice("2024",None))):
        a = f24.loc[sl][m.loc[sl]].stack(); b = f24.loc[sl].stack()
        a6 = f6.loc[sl][m.loc[sl]].stack()
        print(f" thr={thr} {per}: n={len(a)} fwd24={a.mean()*1e4:.0f}bp (uncond {b.mean()*1e4:.0f}) fwd6={a6.mean()*1e4:.0f}bp")
print("=== Pump fade: after X% rise over 4h, fwd 24h")
for thr in (0.05,0.08,0.12):
    m = r4 > thr
    for per, sl in (("IS", slice(None,"2023")), ("OOS", slice("2024",None))):
        a = f24.loc[sl][m.loc[sl]].stack(); b = f24.loc[sl].stack()
        print(f" thr={thr} {per}: n={len(a)} fwd24={a.mean()*1e4:.0f}bp (uncond {b.mean()*1e4:.0f})")

print("=== Fear & Greed: fwd 7d BTC return by FNG bucket")
fng = sentiment.fear_greed(0)
bd = btc.copy(); bd.index = bd.index + pd.Timedelta(hours=1); bd = bd[bd.index.hour==0]
fwd7 = bd.shift(-7)/bd-1
x = pd.DataFrame({"fng": fng.reindex(bd.index), "fwd7": fwd7}).dropna()
x["b"] = pd.cut(x.fng, [0,20,35,50,65,80,100])
for per, sl in (("IS", slice(None,"2023")), ("OOS", slice("2024",None))):
    g = x.loc[sl].groupby("b").fwd7.agg(["mean","count"]); g["mean"]=(g["mean"]*100).round(2)
    print(per); print(g.T.to_string())
