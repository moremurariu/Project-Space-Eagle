"""r48 volatility-managed books: scale each book by its own long-run vol / lagged realised vol (cap 2x).
Cost of scaling: |d scale| x (fee+slippage)."""
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
COST = 0.00045 + 0.0005
def sh(x): return x.mean() / x.std() * np.sqrt(365) if x.std() > 0 else np.nan
def dd(x): e = (1 + x).cumprod(); return (e / e.cummax() - 1).min()
rows = []
for k in books:
    r = D[k]
    for win in (20, 60):
        rv = r.rolling(win, min_periods=win // 2).std().shift(1) * np.sqrt(365)
        tgt = r.rolling(365, min_periods=120).std().shift(1) * np.sqrt(365)   # own long-run vol (no look-ahead)
        sc = (tgt / rv).clip(0, 2).fillna(1.0)
        m = sc * r - sc.diff().abs().fillna(0) * COST
        for per, sl in (("IS", slice("2020", "2023")), ("OOS", slice("2024", None))):
            rows.append(dict(book=k, win=win, per=per, base=sh(r[sl]), managed=sh(m[sl]), dd_base=dd(r[sl]), dd_man=dd(m[sl]), avg_scale=sc[sl].mean()))
T = pd.DataFrame(rows).pivot_table(index=["book", "win"], columns="per", values=["base", "managed", "dd_base", "dd_man", "avg_scale"])
print(T.round(2).to_string())
