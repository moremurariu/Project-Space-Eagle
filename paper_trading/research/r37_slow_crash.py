"""r37 slow 24h bleeds (no 4h cascade): do they rebound like 4h liquidation cascades, or keep falling (news-driven)?"""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
P = load_panel(); F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
r4 = P / P.shift(4) - 1; r24 = P / P.shift(24) - 1
fwd24 = P.shift(-25) / P.shift(-1) - 1      # entry 1h after the signal close
btc24 = r24["BTCUSDT"].to_numpy()[:, None]
def ev(label, sig):
    for per, sl in (("IS", slice("2019", "2023")), ("OOS", slice("2024", None))):
        s = sig[sl]; f = fwd24[sl]
        first = s & ~s.shift(1, fill_value=False).rolling(24, min_periods=1).max().astype(bool)   # de-overlap: first hour
        x = f[first].stack().dropna()
        print(f"  {label:42s} {per}: events {len(x):4d}  mean fwd24 {x.mean()*1e4:+6.0f}bp  median {x.median()*1e4:+5.0f}bp  hit {np.mean(x>0):.0%}")
print("event study (fwd 24h from 1h after signal; first signal hour per 24h per coin)")
ev("4h cascade < -10% (live)", r4 < -0.10)
for thr in (-0.12, -0.15, -0.20):
    slow = (r24 < thr) & (r4.rolling(24).min() > -0.10)
    ev(f"24h < {int(thr*100)}% with no 4h < -10% in window", slow)
    ev(f"  ... and BTC 24h > -3% (idiosyncratic)", slow & (btc24 > -0.03))
def show(label, sig, w=0.15):
    pos = (sig.astype(float) * w).rolling(24, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); pos = pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
    pos = pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
    r = engine.run(P, pos, fee=0.00045, slippage=0.001, funding=F, lag=1).returns
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    print(f"  {label:42s} IS sh {a['sharpe']:5.2f} cagr {a['cagr']*100:4.0f}% dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} cagr {b['cagr']*100:4.0f}% dd {b['max_dd']*100:4.0f}%")
print("backtests (15%/pos, hold 24h, lag 1h, fees + 0.1% slip)")
show("4h cascade < -10% (live, single tranche)", r4 < -0.10)
for thr in (-0.15, -0.20):
    show(f"slow 24h < {int(thr*100)}%", (r24 < thr) & (r4.rolling(24).min() > -0.10))
# diversification vs the live crash book (r30 scale-in) and as an add-on
def pos_of(sig, w=0.15):
    pos = (sig.astype(float) * w).rolling(24, min_periods=1).max().fillna(0)
    g = pos.sum(axis=1); return pos.mul((1 / g).clip(upper=1).fillna(1), axis=0)
s = r4 < -0.10; ref = P.where(s).ffill(limit=12); t2 = (P < ref * 0.95) & ~s
crash = pos_of(s, 0.075) + pos_of(t2, 0.075)
slow = pos_of((r24 < -0.15) & (r4.rolling(24).min() > -0.10))
def rets(pos):
    pos = pos.where(pos.diff().abs().sum(axis=1) > 0, np.nan)
    return engine.run(P, pos, fee=0.00045, slippage=0.001, funding=F, lag=1).returns
rc, rs = rets(crash), rets(slow)
dc, ds = rc.resample("D").sum(), rs.resample("D").sum()
print("daily corr crash(live r30) vs slow:", round(dc.corr(ds), 2), "| IS", round(dc[:"2023"].corr(ds[:"2023"]), 2), "OOS", round(dc["2024":].corr(ds["2024":]), 2))
for lab, r in (("crash r30 alone", rc), ("slow alone", rs), ("50/50 crash + slow", 0.5 * rc + 0.5 * rs)):
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    print(f"  {lab:22s} IS sh {a['sharpe']:.2f} dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:.2f} dd {b['max_dd']*100:4.0f}%")
