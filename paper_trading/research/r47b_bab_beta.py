"""r47b xs_bab realised beta with correctly aligned daily returns, and alpha after hedging it (IS / OOS)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
w = sum(S.xs_bab(P, offset_d=o).ffill().fillna(0) for o in range(7)) / 7
x = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.01).returns
day = lambda v: (1 + v).groupby(v.index.floor("D")).prod() - 1
hr = P.pct_change(fill_method=None)
ew = day(hr.mean(axis=1).fillna(0)); btc = day(hr["BTCUSDT"].fillna(0)); b = day(x)
J = pd.DataFrame({"bab": b, "ew": ew, "btc": btc}).dropna()
for per, sl in (("IS 2020-23", slice("2020", "2023")), ("OOS 2024+", slice("2024", None)), ("2026", slice("2026", None))):
    X = J[sl]
    for m in ("ew", "btc"):
        beta = X["bab"].cov(X[m]) / X[m].var(); res = X["bab"] - beta * X[m]
        print(f"{per:10s} vs {m:3s}: beta {beta:5.2f} corr {X['bab'].corr(X[m]):5.2f} | raw sh {X['bab'].mean()/X['bab'].std()*np.sqrt(365):5.2f}"
              f" | hedged sh {res.mean()/res.std()*np.sqrt(365):5.2f} ann alpha {res.mean()*365*100:5.1f}%")
# beta estimated out-of-sample (rolling 90d, lagged) hedge vs EW market
rb = (J["bab"].rolling(90).cov(J["ew"]) / J["ew"].rolling(90).var()).shift(1)
h = (J["bab"] - rb * J["ew"]).dropna()
for per, sl in (("IS 2020-23", slice("2020", "2023")), ("OOS 2024+", slice("2024", None))):
    s = h[sl]; print(f"{per} rolling-hedged (lagged 90d beta vs EW) sh {s.mean()/s.std()*np.sqrt(365):.2f}  mean beta {rb[sl].mean():.2f}")
print("net $ (avg):", round(w.sum(axis=1)["2020":].mean(), 2))
