"""r48b trend_univ_lo vol-managed robustness: realised vol from active days only, caps 1.5/2, per-year Sharpe."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
w = S.trend_breakout(P, long_only=True, port_vol_target=0.4)
w = w * S.efficiency_gate(P, 30, 0.1, 0.3).reindex(columns=w.columns).where(w.notna())
res = engine.run(P, w, fee=0.00045, slippage=0.0005, funding=F, min_trade=0.02)
x = res.returns
r = (1 + x).groupby(x.index.floor("D")).prod() - 1
gross = w.ffill().fillna(0).abs().sum(axis=1).groupby(w.index.floor("D")).last().reindex(r.index).fillna(0)
COST = 0.00095
def sh(v): return v.mean() / v.std() * np.sqrt(365) if v.std() > 0 else np.nan
print("share of days flat (gross<0.05):", round((gross < 0.05)["2020":].mean(), 2))
act = r.where(gross.shift(1) > 0.05)
for label, base_r in (("all days", r), ("active days", act)):
    for win in (20, 60):
        rv = base_r.rolling(win, min_periods=win // 2).std().shift(1)
        tgt = base_r.rolling(365, min_periods=120).std().shift(1)
        for cap in (1.5, 2.0):
            sc = (tgt / rv).clip(0, cap).ffill().fillna(1.0)
            m = sc * r - sc.diff().abs().fillna(0) * gross * COST
            yr = {y: round(sh(m[str(y)]), 2) for y in range(2020, 2027)}
            print(f"{label:11s} win {win} cap {cap}: IS {sh(m['2020':'2023']):.2f} OOS {sh(m['2024':]):.2f} avg scale {sc['2020':].mean():.2f} | {yr}")
print("base by year:", {y: round(sh(r[str(y)]), 2) for y in range(2020, 2027)})
