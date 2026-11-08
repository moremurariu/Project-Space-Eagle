"""r55 live vs backtest tracking: each live book's hourly equity vs the same recipe backtested on the Binance panel over
the identical window (from the book's first trade). Daily returns, correlation, tracking difference, cost split."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.backtest import engine
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.strategies import signals as S
from ptrade.universe import MAJORS
from ptrade.live.strategies import XSFunding
P = load_panel(); F = load_binance_funding_panel(P).fillna(0.0001 / 8)
E = pd.read_csv("state/equity.csv", parse_dates=["ts"]); E["ts"] = pd.to_datetime(E.ts, utc=True)
wm = sum(S.xs_momentum(P, lookback_d=lb, rebalance_d=7, offset_d=o, vol_scaled=True).ffill().fillna(0)
         for lb in (7, 14, 21) for o in range(7)) / 21
recipes = {"xs_momentum": (wm, None, 0.01),
           "trend_breakout_majors": (S.trend_breakout(P[MAJORS], long_only=False, port_vol_target=0.25), MAJORS, 0.02)}
for name, (w, cols, mt) in recipes.items():
    e = E[E.book == name].set_index("ts").sort_index()
    e = e[e.n_pos > 0] if (e.n_pos > 0).any() else e
    start = e.index[0].floor("h")
    live = e.equity.resample("h").last().ffill()
    Pu = P if cols is None else P[cols]; Fu = F if cols is None else F[cols]
    lb = start - pd.Timedelta(days=1)
    r = engine.run(Pu[lb:], w.reindex(Pu.index)[lb:], fee=0.00045, slippage=0.0005, funding=Fu[lb:], min_trade=mt).returns[start:]
    bt = (1 + r).cumprod() * live.iloc[0]
    J = pd.DataFrame({"live": live, "bt": bt}).dropna()
    dl, db = J.live.resample("D").last().pct_change().dropna(), J.bt.resample("D").last().pct_change().dropna()
    hl, hb = J.live.pct_change().dropna(), J.bt.pct_change().dropna()
    print(f"{name}: {J.index[0]:%m-%d %H}h → {J.index[-1]:%m-%d %H}h | live {J.live.iloc[-1]/J.live.iloc[0]-1:+.2%} vs bt {J.bt.iloc[-1]/J.bt.iloc[0]-1:+.2%}"
          f" | hourly corr {hl.corr(hb):.2f}, daily corr {dl.corr(db):.2f} | live fees ${e.fees.iloc[-1]-e.fees.iloc[0]:.0f}, slip ${e.slippage.iloc[-1]-e.slippage.iloc[0]:.0f}, funding ${e.funding.iloc[-1]-e.funding.iloc[0]:.0f}")
    print("   daily live vs bt (%):", [(f"{d:%m-%d}", round(a*100, 2), round(b*100, 2)) for d, a, b in zip(dl.index, dl, db.reindex(dl.index))])
