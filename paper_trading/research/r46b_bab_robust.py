"""r46b BAB beta-neutral robustness: realised market beta, net $ exposure, per-year, parameter grid."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
import importlib.util
from ptrade.backtest import engine
spec = importlib.util.spec_from_file_location("r46", "research/r46_lowvol_bab.py")
src = open("research/r46_lowvol_bab.py").read().split("res = {}")[0]
g = {}; exec(compile(src, "r46", "exec"), g)
P, F, d, r, mk, S = g["P"], g["F"], g["d"], g["r"], g["mk"], g["S"]
def run(w): return engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
base = None
for bw in (60,):
    g["beta"] = r.rolling(bw, min_periods=int(bw*2/3)).cov(mk).div(mk.rolling(bw, min_periods=int(bw*2/3)).var(), axis=0)
    for q in (0.25,):
        def legs_q(score, bn, q=q): return g["legs"](score, q=q, bn=bn)
        W = legs_q(-g["beta"], True)
        ws = []
        for o in range(7):
            m = W.copy(); keep = (np.arange(len(m)) - o) % 7 == 0
            m.loc[~keep] = np.nan; ws.append(m.ffill().fillna(0))
        Wd = sum(ws) / 7
        x = run(S.daily_to_hourly(Wd, P.index)); g["st"](f"BAB bn beta{bw}d q{q}", x)
        if bw == 60 and q == 0.25: base, Wbase = x, Wd
xd = base.resample("D").sum()
bc = [c for c in d.columns if c.upper().startswith("BTC")][0]; btc = d[bc].pct_change()
for nm, m in (("EW mkt", mk), ("BTC", btc)):
    j = pd.concat([xd, m.reindex(xd.index)], axis=1).dropna()
    b = j.iloc[:,0].cov(j.iloc[:,1]) / j.iloc[:,1].var()
    print(f"realised beta vs {nm}: {b:.2f}  corr {j.corr().iloc[0,1]:.2f}")
print("avg net $ exposure:", round(Wbase.sum(axis=1)["2019":].mean(), 3), " avg gross:", round(Wbase.abs().sum(axis=1)["2019":].mean(), 3))
yr = base.groupby(base.index.year).apply(lambda s: s.mean() / s.std() * np.sqrt(24*365) if s.std() > 0 else np.nan)
print("Sharpe by year:", yr.round(2).to_dict())
# hedged: subtract realised-beta * EW market (daily) to see residual alpha
