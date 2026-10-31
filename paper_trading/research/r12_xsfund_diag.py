"""Diagnose XS-funding discrepancy: signal source (Binance vs HL funding) vs universe (40 vs 15 coins)."""
import sys, glob, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.universe import UNIVERSE
from ptrade.strategies import signals as S
P = load_panel()
FB = load_binance_funding_panel(P, default_8h=np.nan)
inv = {v: k for k, v in UNIVERSE.items()}
hl = {}
for f in glob.glob("cache/hl_funding_*.parquet"):
    c = f.split("hl_funding_")[1][:-8]
    if c in inv:
        hl[inv[c]] = pd.read_parquet(f)["funding"]
FH = pd.DataFrame(hl).reindex(P.index)
s15 = list(FH.columns)
def run(Pu, Fsig, Fpay, label, start="2024-01-01", rev=False):
    Pu = Pu[Pu.index >= start]; Fsig = Fsig.reindex(Pu.index); Fpay = Fpay.reindex(Pu.index).fillna(0)
    sig = -Fsig if rev else Fsig
    w = S.xs_funding_carry(Pu, sig, lookback_h=168, rebalance_h=24, min_assets=6)
    res = engine.run(Pu, w, fee=0.00045, slippage=0.0005, funding=Fpay)
    r = res.returns; yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    print(f"{label:58s} {engine.fmt_stats(res.stats())[:70]} {yr}")
print("coins in HL∩universe:", s15)
run(P, FB, FB, "A Binance signal, 40 coins, 2024+")
run(P[s15], FB[s15], FB[s15], "B Binance signal, 15 coins, 2024+")
run(P[s15], FH, FH, "C HL signal, 15 coins, 2024+")
run(P[s15], FH, FH, "D HL signal REVERSED, 15 coins", rev=True)
run(P[s15], FB[s15], FH, "E Binance signal, HL funding paid, 15 coins")
print("corr of 7d-trailing funding ranks (HL vs Binance), daily mean:",
      round(FH.rolling(168).sum().rank(axis=1).corrwith(FB[s15].rolling(168).sum().rank(axis=1), axis=1).loc["2024":].mean(), 3))
