"""r41 long/short trend breadth: majors (3) vs top-6 / top-10 / all-40 coins, with and without the ER30 gate."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
TOP6 = MAJORS + ["XRPUSDT", "BNBUSDT", "DOGEUSDT"]
TOP10 = TOP6 + ["ADAUSDT", "LINKUSDT", "AVAXUSDT", "LTCUSDT"]
def show(label, cols, gate):
    w = S.trend_breakout(P[cols], long_only=False, port_vol_target=0.25)
    if gate:
        w = w * S.efficiency_gate(P[cols], 30, 0.1, 0.3).reindex(columns=w.columns).where(w.notna())
    r = engine.run(P[cols], w, fee=0.00045, slippage=0.0005, funding=F[cols], min_trade=0.02).returns
    a, b = engine.perf_stats(r["2019":"2023"]), engine.perf_stats(r["2024":])
    yr = {str(k)[2:]: round(v * 100) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{label:28s} IS sh {a['sharpe']:5.2f} vol {a['vol']*100:3.0f}% dd {a['max_dd']*100:4.0f}% | OOS sh {b['sharpe']:5.2f} vol {b['vol']*100:3.0f}% dd {b['max_dd']*100:4.0f}% {yr}", flush=True)
    return r
missing = [c for c in TOP10 if c not in P]
print("missing:", missing)
R = {}
for lab, cols in (("majors3", MAJORS), ("top6", TOP6), ("top10", [c for c in TOP10 if c in P]), ("all40", list(P.columns))):
    for g in (False, True):
        R[(lab, g)] = show(f"L/S {lab}{' + ER30 gate' if g else ''}", cols, g)
D = pd.DataFrame({f"{k[0]}{'_g' if k[1] else ''}": v for k, v in R.items()}).resample("D").sum()
print(D["2020":].corr().round(2).loc[["majors3", "top10", "all40"], :].to_string())
