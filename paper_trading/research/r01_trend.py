import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import pandas as pd
from ptrade.research_utils import load_panel, evaluate, print_eval
from ptrade.strategies import signals as S
from ptrade.backtest import engine

P = load_panel()
fund_default = pd.DataFrame(0.0001/8, index=P.index, columns=P.columns)  # ~11% APR paid by longs
majors = ["BTCUSDT","ETHUSDT","SOLUSDT"]
# buy & hold BTC benchmark
w = pd.DataFrame(0.0, index=P.index, columns=["BTCUSDT"]); w.iloc[0] = 1.0; w.iloc[1:] = float("nan")
_, ev = evaluate(P[["BTCUSDT"]], w, label="BTC buy&hold (spot, no funding)"); print_eval(ev)

for uni_name, cols in [("BTC", ["BTCUSDT"]), ("majors", majors), ("all40", list(P.columns))]:
    Pu = P[cols]; Fu = fund_default[cols]
    for lo in (False, True):
        for pvt in (0.3, 0.5):
            w = S.trend_ewmac(Pu, long_only=lo, port_vol_target=pvt)
            _, ev = evaluate(Pu, w, funding=Fu, min_trade=0.02, label=f"EWMAC {uni_name} long_only={lo} pvt={pvt}")
            print_eval(ev)
        w = S.trend_breakout(Pu, long_only=lo, port_vol_target=0.4)
        _, ev = evaluate(Pu, w, funding=Fu, min_trade=0.02, label=f"Breakout {uni_name} long_only={lo} pvt=0.4")
        print_eval(ev)
