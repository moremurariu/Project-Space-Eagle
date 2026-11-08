"""r56 r44 re-run with Binance funding spliced with the live 1m-premium reconstruction after the last monthly archive
(load_binance_funding_panel(splice_recon=True)); adds per-year and Oct-Nov 2026 stats for the live recipe."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P, splice_recon=True)
run = lambda w: engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
def st(label, r):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    ew = P.pct_change().mean(axis=1)
    beta = r.resample("D").sum().cov(ew.resample("D").sum()) / ew.resample("D").sum().var()
    print(f"{label:34s} IS sh {a['sharpe']:5.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} dd {b['max_dd']*100:4.0f}% | beta to EW mkt {beta:+.2f}", flush=True)
    return r
w = S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24)
st("dollar-neutral (live)", run(w))
# rolling 60d beta of each coin to the equal-weight market (hourly returns), lagged
rh = P.pct_change(); mk = rh.mean(axis=1)
beta = rh.rolling(24 * 60, min_periods=24 * 20).cov(mk).div(mk.rolling(24 * 60, min_periods=24 * 20).var(), axis=0).shift(1)
def beta_neutral(w, cap=2.0):
    out = w.copy()
    rows = w.dropna(how="all").index
    for t in rows:
        x = w.loc[t].fillna(0.0); b = beta.loc[t].reindex(x.index).fillna(1.0).clip(0.2, 3)
        L, Sx = x.clip(lower=0), x.clip(upper=0)
        bl, bs = (L * b).sum(), (-Sx * b).sum()
        if bl <= 0 or bs <= 0:
            continue
        # scale legs: long leg by k_l, short by k_s with k_l*bl = k_s*bs and gross 1
        kl, ks = bs / (bl + bs) * 2, bl / (bl + bs) * 2
        kl, ks = min(kl, cap), min(ks, cap)
        y = L * kl + Sx * ks; out.loc[t] = y / y.abs().sum()
    return out
wb = beta_neutral(w)
rb = st("beta-neutral legs (live recipe)", run(wb))
yr = rb.groupby(rb.index.year).apply(lambda x: x.mean() / x.std() * np.sqrt(24 * 365) if x.std() > 0 else np.nan)
print("Sharpe by year:", yr.round(2).to_dict())
m = (1 + rb["2026-07":]).groupby(rb["2026-07":].index.to_period("M")).prod() - 1
print("monthly return 2026-07+:", {str(k): f"{v*100:+.2f}%" for k, v in m.items()})