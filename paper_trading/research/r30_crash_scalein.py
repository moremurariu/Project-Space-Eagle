"""r30 crash-rebound entry timing: scale-in tranches, delayed entry, green-hour confirmation (hourly, lag 1h)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
P = load_panel(); F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
r1 = P / P.shift(1) - 1; r4 = P / P.shift(4) - 1
mkt = (r4["BTCUSDT"] < -0.03).to_numpy()[:, None]

def hold(trig, w, h=24):
    return (trig.astype(float) * w).rolling(h, min_periods=1).max().fillna(0)

def finish(pos):
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    return pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)

def base_sig(market_wide):
    s = r4 < -0.10
    return s & mkt if market_wide else s

def baseline(mw, w=0.15):
    return finish(hold(base_sig(mw), w))

def delayed(mw, k, w=0.15):
    return finish(hold(base_sig(mw).shift(k, fill_value=False), w))

def confirm(mw, win=6, w=0.15):
    """enter at the first green hourly close within `win` hours after a signal"""
    s = base_sig(mw)
    recent = s.shift(1, fill_value=False).astype(float).rolling(win, min_periods=1).max() > 0
    return finish(hold(recent & (r1 > 0), w))

def scale_in(mw, d=0.05, win=12, w1=0.075, w2=0.075):
    """tranche 1 at signal; tranche 2 if price falls a further d below the signal close within `win` hours"""
    s = base_sig(mw)
    ref = P.where(s).ffill(limit=win)
    t2 = (P < ref * (1 - d)) & ~s
    return finish(hold(s, w1) + hold(t2, w2))

def show(label, pos):
    res = engine.run(P, pos, fee=0.00045, slippage=0.001, funding=F, lag=1)
    r = res.returns
    a = engine.perf_stats(r["2019":"2023"]); b = engine.perf_stats(r["2024":])
    print(f"{label:38s} IS sh {a['sharpe']:5.2f} cagr {a['cagr']*100:4.0f}% dd {a['max_dd']*100:4.0f}% | "
          f"OOS sh {b['sharpe']:5.2f} cagr {b['cagr']*100:4.0f}% dd {b['max_dd']*100:4.0f}%", flush=True)

for mw in (False, True):
    tag = "mkt" if mw else "all"
    show(f"[{tag}] baseline 15%", baseline(mw))
    for k in (1, 2, 3):
        show(f"[{tag}] delay {k}h", delayed(mw, k))
    show(f"[{tag}] first green hour (6h win)", confirm(mw))
    for d in (0.03, 0.05, 0.08):
        show(f"[{tag}] scale-in 7.5+7.5 add @-{int(d*100)}%", scale_in(mw, d))
    show(f"[{tag}] scale-in 10+10 add @-5%", scale_in(mw, 0.05, w1=0.10, w2=0.10))
