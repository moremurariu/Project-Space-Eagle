"""Portfolio view of live strategies: backtested daily return streams, correlations, combined allocation."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
streams = {}
def bt(w, cols=None, slip=0.0005, lag=0):
    Pu = P if cols is None else P[cols]; Fu = F if cols is None else F[cols]
    return engine.run(Pu, w, fee=0.00045, slippage=slip, funding=Fu, min_trade=0.02, lag=lag).returns
streams["trend_brk_majors"] = bt(S.trend_breakout(P[MAJORS], long_only=False, port_vol_target=0.4), MAJORS)
streams["trend_brk_univ_lo"] = bt(S.trend_breakout(P, long_only=True, port_vol_target=0.4))
streams["trend_ewmac_maj_lo"] = bt(S.trend_ewmac(P[MAJORS], long_only=True, port_vol_target=0.3), MAJORS)
r4 = P / P.shift(4) - 1
def crash(mkt):
    sig = (r4 < -0.10) & ((r4["BTCUSDT"] < -0.03).to_numpy()[:, None] if mkt else True)
    pos = (sig.astype(float) * 0.15).rolling(24, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    return pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
streams["crash"] = bt(crash(False), slip=0.001, lag=1)
streams["crash_mkt"] = bt(crash(True), slip=0.001, lag=1)
# DN carry (HL data, 2024+): reuse r11 logic at entry 15 / exit 5
fund, prem = {}, {}
for f in glob.glob("cache/hl_funding_*.parquet"):
    df = pd.read_parquet(f)
    if len(df) > 24 * 60:
        c = f.split("hl_funding_")[1][:-8]; fund[c] = df["funding"]; prem[c] = df["premium"]
FH = pd.DataFrame(fund).sort_index(); FH = FH[FH.index >= "2024-01-01"]; PR = pd.DataFrame(prem).reindex(FH.index)
trail = FH.rolling(168, min_periods=84).mean() * 8760
Fv = FH.fillna(0).to_numpy(); dP = PR.diff().fillna(0).to_numpy(); n = np.zeros(FH.shape[1]); held = np.zeros(FH.shape[1], bool); out = np.zeros(len(FH))
for t in range(len(FH)):
    out[t] = float(n @ Fv[t]) - float(n @ dP[t])
    if FH.index[t].hour == 0:
        tr = trail.iloc[t].to_numpy(); ok = np.isfinite(tr)
        nh = np.where(held, ok & (tr > 0.05), ok & (tr > 0.15))
        if nh.sum() > 6:
            keep = np.zeros_like(nh); keep[np.argsort(-np.where(nh, tr, -np.inf))[:6]] = True; nh &= keep
        k = nh.sum(); tgt = np.where(nh, min(0.15, 0.9 / k) if k else 0.0, 0.0)
        out[t] -= np.abs(tgt - n).sum() * 0.002; n, held = tgt, nh
streams["dn_carry"] = pd.Series(out, index=FH.index)
streams["btc_hold"] = P["BTCUSDT"].pct_change().fillna(0)
D = pd.DataFrame({k: (1 + v).groupby(v.index.floor("D")).prod() - 1 for k, v in streams.items()})
D = D[D.index >= "2024-01-01"].fillna(0)
print("OOS 2024-01 → 2026-10 daily streams")
st = pd.DataFrame({k: engine.perf_stats(D[k], 365) for k in D}).T[["cagr", "vol", "sharpe", "max_dd"]]
print((st * [100, 100, 1, 100]).round(2).to_string())
print("\ncorrelation (daily):"); print(D.corr().round(2).to_string())
books = [c for c in D.columns if c != "btc_hold"]
ew = D[books].mean(axis=1)
iv = 1 / D[books].rolling(90, min_periods=30).std().shift(1)
rp = (D[books] * iv.div(iv.sum(axis=1), axis=0)).sum(axis=1)
for name, r in [("equal-weight books", ew), ("inverse-vol (risk parity) books", rp)]:
    s = engine.perf_stats(r, 365)
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{name:32s} cagr {s['cagr']*100:5.1f}% vol {s['vol']*100:5.1f}% sharpe {s['sharpe']:.2f} maxdd {s['max_dd']*100:5.1f}% {yr}")
print("\n--- sensitivity: scale trend_brk_majors risk, drop ewmac")
for scale in (1.0, 0.6, 0.5):
    for drop_ewmac in (False, True):
        X = D[books].copy(); X["trend_brk_majors"] *= scale
        cols = [c for c in books if not (drop_ewmac and c == "trend_ewmac_maj_lo")]
        r = X[cols].mean(axis=1); s = engine.perf_stats(r, 365)
        print(f"  majors x{scale} drop_ewmac={drop_ewmac}: cagr {s['cagr']*100:5.1f}% vol {s['vol']*100:5.1f}% sharpe {s['sharpe']:.2f} maxdd {s['max_dd']*100:5.1f}%")
