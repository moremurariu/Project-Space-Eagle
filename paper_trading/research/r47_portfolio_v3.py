"""r47 portfolio view v3: r40 books + xs_bab (r46). Equal-weight with vs without BAB."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
def bt(w, cols=None, slip=0.0005, lag=0, mt=0.02):
    Pu = P if cols is None else P[cols]; Fu = F if cols is None else F[cols]
    return engine.run(Pu, w, fee=0.00045, slippage=slip, funding=Fu, min_trade=mt, lag=lag).returns
st = {}
st["trend_majors"] = bt(S.trend_breakout(P[MAJORS], long_only=False, port_vol_target=0.25), MAJORS)
w = S.trend_breakout(P, long_only=True, port_vol_target=0.4)
st["trend_univ_lo"] = bt(w * S.efficiency_gate(P, 30, 0.1, 0.3).reindex(columns=w.columns).where(w.notna()))
st["xs_funding"] = bt(S.xs_funding_carry(P, F.shift(48), lookback_h=168, rebalance_h=24), mt=0.01)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
st["xs_momentum"] = bt(wm, mt=0.01)
st["xs_bab"] = bt(sum(S.xs_bab(P, offset_d=o).ffill().fillna(0) for o in range(7)) / 7, mt=0.01)
r4 = P / P.shift(4) - 1
def crash(mkt):
    s = (r4 < -0.10) & ((r4["BTCUSDT"] < -0.03).to_numpy()[:, None] if mkt else True)
    ref = P.where(s).ffill(limit=12); t2 = (P < ref * 0.95) & ~s
    pos = sum((x.astype(float) * 0.075).rolling(24, min_periods=1).max().fillna(0) for x in (s, t2))
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    return pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
st["crash"] = bt(crash(False), slip=0.001, lag=1)
st["crash_mkt"] = bt(crash(True), slip=0.001, lag=1)
st["btc_hold"] = P["BTCUSDT"].pct_change().fillna(0)
D = pd.DataFrame({k: (1 + v).groupby(v.index.floor("D")).prod() - 1 for k, v in st.items()}).fillna(0)
books = [c for c in D.columns if c != "btc_hold"]
for per, sl in (("IS 2020-2023", slice("2020", "2023")), ("OOS 2024+", slice("2024", None))):
    X = D[sl]
    s = pd.DataFrame({k: engine.perf_stats(X[k], 365) for k in X}).T[["cagr", "vol", "sharpe", "max_dd"]]
    print(f"\n== {per}"); print((s * [100, 100, 1, 100]).round(2).to_string())
    nob = [b for b in books if b != "xs_bab"]
    for name, r in (("equal-weight books (with BAB)", X[books].mean(axis=1)), ("equal-weight books (no BAB)", X[nob].mean(axis=1)),
                    ("inverse-vol books (90d, lagged)", (X[books] * (1 / X[books].rolling(90, min_periods=30).std().shift(1)).pipe(lambda iv: iv.div(iv.sum(axis=1), axis=0))).sum(axis=1))):
        p = engine.perf_stats(r, 365)
        print(f"  {name:32s} cagr {p['cagr']*100:5.1f}% vol {p['vol']*100:5.1f}% sharpe {p['sharpe']:.2f} maxdd {p['max_dd']*100:5.1f}%")
print("\ncorrelation (daily, 2020+):"); print(D["2020":].corr().round(2).to_string())
