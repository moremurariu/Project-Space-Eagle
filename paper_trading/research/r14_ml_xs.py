import sys, warnings, time; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import pandas as pd
from ptrade.data import binance
from ptrade.universe import UNIVERSE
from ptrade.backtest import engine
from ptrade.strategies import ml_xs
t0 = time.time()
dfs = binance.load_many(list(UNIVERSE), "1h", start="2019-01-01", refresh=False)
idx = pd.date_range(pd.Timestamp("2019-01-01", tz="UTC"), max(v.index[-1] for v in dfs.values()), freq="h")
C = binance.panel(dfs, "close").reindex(idx).ffill(limit=3)
Q = binance.panel(dfs, "quote_volume").reindex(idx)
T = binance.panel(dfs, "taker_buy_quote").reindex(idx)
F = pd.DataFrame(0.0001 / 8, index=C.index, columns=C.columns)
for q, reb, sm, mt in [(0.2, 1, 1, 0.0), (0.2, 1, 3, 0.02), (0.2, 1, 5, 0.03), (0.2, 3, 3, 0.02), (0.3, 1, 5, 0.02)]:
    W = ml_xs.walk_forward_weights(C, Q, T, first_year=2021, quantile=q, rebalance_d=reb, smooth_d=sm)
    res = engine.run(C, W, fee=0.00045, slippage=0.0005, funding=F, min_trade=mt)
    q = f"{q} smooth={sm} buf={mt}"
    r = res.returns[res.returns.index >= "2021-01-01"]
    yr = {str(k): round(v * 100, 1) for k, v in ((1 + r).groupby(r.index.year).prod() - 1).items()}
    st = engine.perf_stats(r, 8760, res.turnover["2021":], res.weights["2021":])
    print(f"ML XS q={q} reb={reb}d: {engine.fmt_stats(st)}\n     yearly {yr}  ({time.time()-t0:.0f}s)", flush=True)

