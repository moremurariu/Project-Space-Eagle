"""r31 crash-rebound exit timing on top of r30 scale-in: hold length and take-profit exits (hourly, lag 1h)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
P = load_panel(); F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
r4 = P / P.shift(4) - 1
mkt = (r4["BTCUSDT"] < -0.03).to_numpy()[:, None]

def tranche(trig, w, h, tp=None, ref=None):
    """hold `w` for h hours after each trigger; with tp, exit early once close >= entry ref * (1 + tp)"""
    pos = (trig.astype(float) * w).rolling(h, min_periods=1).max().fillna(0)
    if tp is None:
        return pos
    px_ref = P.where(trig).ffill(limit=h - 1)
    hit = (P >= px_ref * (1 + tp)) & ~trig
    grp = trig.cumsum()
    hit_since = pd.DataFrame({c: hit[c].astype(float).groupby(grp[c]).cummax() for c in P.columns}) > 0  # TP hit since latest trigger
    return pos.where(~hit_since, 0.0)

def strat(mw, h=24, tp=None, d=0.05, win=12):
    s = r4 < -0.10
    if mw: s = s & mkt
    ref = P.where(s).ffill(limit=win)
    t2 = (P < ref * (1 - d)) & ~s
    pos = tranche(s, 0.075, h, tp) + tranche(t2, 0.075, h, tp)
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    return pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)

def show(label, pos):
    r = engine.run(P, pos, fee=0.00045, slippage=0.001, funding=F, lag=1).returns
    a = engine.perf_stats(r["2019":"2023"]); b = engine.perf_stats(r["2024":])
    print(f"{label:30s} IS sh {a['sharpe']:5.2f} cagr {a['cagr']*100:4.0f}% dd {a['max_dd']*100:4.0f}% | "
          f"OOS sh {b['sharpe']:5.2f} cagr {b['cagr']*100:4.0f}% dd {b['max_dd']*100:4.0f}%", flush=True)

for mw in (False, True):
    tag = "mkt" if mw else "all"
    for h in (12, 18, 24, 30, 36, 48):
        show(f"[{tag}] hold {h}h", strat(mw, h))
    for tp in (0.05, 0.10):
        show(f"[{tag}] hold 24h, TP +{int(tp*100)}%", strat(mw, 24, tp))
        show(f"[{tag}] hold 48h, TP +{int(tp*100)}%", strat(mw, 48, tp))

# hold-length ensemble (parameter-risk reduction): average of positions over several hold lengths
for mw in (False, True):
    tag = "mkt" if mw else "all"
    for hs in ((12, 24, 48), (12, 18, 24, 30, 36, 48)):
        pos = sum(strat(mw, h).ffill().fillna(0) for h in hs) / len(hs)
        show(f"[{tag}] ensemble hold {hs}", pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan))
