import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import pandas as pd
from ptrade.research_utils import load_panel
from ptrade.backtest import engine
from ptrade.strategies import signals as S
P = load_panel()
F = pd.DataFrame(0.0001 / 8, index=P.index, columns=P.columns)
pairs = [("ETHUSDT", "BTCUSDT"), ("SOLUSDT", "ETHUSDT"), ("ARBUSDT", "OPUSDT"), ("LTCUSDT", "BCHUSDT"),
         ("ATOMUSDT", "DOTUSDT"), ("UNIUSDT", "AAVEUSDT"), ("AVAXUSDT", "NEARUSDT"), ("DOGEUSDT", "PEPEUSDT")]
rows = []
for win_d, entry in [(20, 2.0), (60, 2.0), (20, 2.5)]:
    rets = []
    for a, b in pairs:
        Pu = P[[a, b]].dropna()
        w = S.pair_zscore(Pu, a, b, window_h=24 * win_d, entry=entry, exit_=0.5, gross=1.0, beta_window_h=24 * 60)
        res = engine.run(Pu, w, fee=0.00045, slippage=0.0005, funding=F[[a, b]])
        r = res.returns
        i, o = engine.perf_stats(r[:"2023"]), engine.perf_stats(r["2024":])
        rows.append((win_d, entry, f"{a[:-4]}/{b[:-4]}", round(i.get("sharpe", float("nan")), 2), round(o["sharpe"], 2), round(o["cagr"] * 100, 1)))
        rets.append(r.rename(f"{a}/{b}"))
    port = pd.concat(rets, axis=1).fillna(0).mean(axis=1)
    i, o = engine.perf_stats(port[:"2023"]), engine.perf_stats(port["2024":])
    print(f"window {win_d}d entry {entry}: equal-weight pairs portfolio IS sharpe {i['sharpe']:.2f} | OOS sharpe {o['sharpe']:.2f} cagr {o['cagr']*100:.1f}%")
print(pd.DataFrame(rows, columns=["win_d", "entry", "pair", "IS_sh", "OOS_sh", "OOS_cagr%"]).to_string(index=False))
