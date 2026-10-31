import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, evaluate, print_eval
P = load_panel()
F = pd.DataFrame(0.0001/8, index=P.index, columns=P.columns)
r4 = P/P.shift(4)-1
f24 = P.shift(-24)/P-1
for thr in (-0.06,-0.08,-0.10):
    m = (r4 < thr)
    vals_is = f24[:"2023"].where(m[:"2023"]).stack().dropna(); vals_oos = f24["2024":].where(m["2024":]).stack().dropna()
    print(f"thr={thr}: IS n={len(vals_is)} mean={vals_is.mean()*1e4:.0f} med={vals_is.median()*1e4:.0f} hit={(vals_is>0).mean():.2f} | OOS n={len(vals_oos)} mean={vals_oos.mean()*1e4:.0f} med={vals_oos.median()*1e4:.0f} hit={(vals_oos>0).mean():.2f}")
# BTC-relative version: idiosyncratic crash (asset minus BTC)
rel4 = r4.sub(r4["BTCUSDT"], axis=0)
for thr in (-0.05,-0.08):
    m = rel4 < thr
    vi = (f24.sub(f24["BTCUSDT"],axis=0))[:"2023"].where(m[:"2023"]).stack().dropna()
    vo = (f24.sub(f24["BTCUSDT"],axis=0))["2024":].where(m["2024":]).stack().dropna()
    print(f"REL thr={thr}: IS n={len(vi)} mean={vi.mean()*1e4:.0f} med={vi.median()*1e4:.0f} | OOS n={len(vo)} mean={vo.mean()*1e4:.0f} med={vo.median()*1e4:.0f}")

def crash_strategy(P, thr, hold, per_w=0.15, max_gross=1.0, hedge_btc=False):
    sig = (P/P.shift(4)-1) < thr
    pos = sig.astype(float).rolling(hold, min_periods=1).max().fillna(0)
    w = pos*per_w
    if hedge_btc:
        w["BTCUSDT"] = w["BTCUSDT"] - w.drop(columns="BTCUSDT").sum(axis=1)
    g = w.abs().sum(axis=1); w = w.mul((max_gross/g).clip(upper=1).fillna(1), axis=0)
    # only emit rows on changes of the *signal state* to let positions drift
    chg = pos.diff().abs().sum(axis=1) > 0
    return w.where(chg, np.nan)
for thr in (-0.06,-0.08,-0.10):
    for hold in (12,24,48):
        w = crash_strategy(P, thr, hold)
        _, ev = evaluate(P, w, funding=F, label=f"CrashRebound thr={thr} hold={hold}h"); print_eval(ev)
