import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel, evaluate, print_eval
from ptrade.backtest import engine
from ptrade.strategies import signals as S
P = load_panel()
F = load_binance_funding_panel(P, default_8h=np.nan)  # NaN where no data
avail = F.notna()
F0 = F.fillna(0)
print("avg funding APR by year (cross-sectional mean, %):")
print((F0.where(avail).mean(axis=1).groupby(F.index.year).mean()*8760*100).round(1).to_string())

def carry_dn(F, entry_apr=0.15, exit_apr=0.05, lb_h=24*7, max_per=0.15, max_tot=0.75, cost_side=0.0022, min_hist=24*14, top_k=None):
    """Delta-neutral: short perp + long spot on coins with high trailing funding. Returns hourly return series."""
    trail = F.rolling(lb_h, min_periods=lb_h//2).mean()*8760
    hist = F.notna().cumsum()
    cols = F.columns; Fv = F.fillna(0).to_numpy()
    held = np.zeros(len(cols), bool); n = np.zeros(len(cols)); rets = np.zeros(len(F)); turn = np.zeros(len(F))
    for t in range(len(F)):
        # accrue funding on current notional (short perp receives +f)
        rets[t] = float(np.dot(n, Fv[t]))
        if F.index[t].hour == 0:
            tr = trail.iloc[t].to_numpy(); ok = (hist.iloc[t].to_numpy() > min_hist) & np.isfinite(tr)
            new_held = np.where(held, ok & (tr > exit_apr), ok & (tr > entry_apr))
            if top_k is not None and new_held.sum() > top_k:
                order = np.argsort(-np.where(new_held, tr, -np.inf)); keep = np.zeros_like(new_held); keep[order[:top_k]] = True; new_held &= keep
            k = new_held.sum()
            tgt = np.where(new_held, min(max_per, max_tot / k) if k else 0.0, 0.0)
            dn = np.abs(tgt - n).sum()
            rets[t] -= dn * cost_side
            turn[t] = dn
            n = tgt; held = new_held
    r = pd.Series(rets, index=F.index)
    return r, pd.Series(turn, index=F.index)

for kw in [dict(), dict(entry_apr=0.10, exit_apr=0.0), dict(entry_apr=0.25, exit_apr=0.10), dict(entry_apr=0.15, exit_apr=0.05, lb_h=72),
           dict(entry_apr=0.15, exit_apr=0.05, top_k=5, max_per=0.15), dict(entry_apr=0.30, exit_apr=0.10, top_k=5, max_per=0.15)]:
    r, to = carry_dn(F, **kw)
    st_is = engine.perf_stats(r[:"2023"]); st_oos = engine.perf_stats(r["2024":])
    yr = engine.yearly_returns(r)
    print(f"DN carry {kw}\n   IS  {engine.fmt_stats(st_is)}\n   OOS {engine.fmt_stats(st_oos)}\n   yearly: " + " ".join(f"{y}:{v*100:+.1f}%" for y,v in yr.items()))
# always-on BTC+ETH basis
r = (F0[["BTCUSDT","ETHUSDT"]]*0.375).sum(axis=1)
print("Always BTC+ETH DN 0.75 notional: yearly", " ".join(f"{y}:{v*100:+.1f}%" for y,v in engine.yearly_returns(r).items()))

print("=== Directional XS funding (long low funding, short high funding)")
for lb, reb in [(72,24),(168,24),(168,168)]:
    w = S.xs_funding_carry(P, F0.where(avail), lookback_h=lb, rebalance_h=reb)
    _, ev = evaluate(P, w, funding=F0, label=f"XS funding lb={lb} reb={reb}"); print_eval(ev)
