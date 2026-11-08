"""Shared helpers for research scripts: load panels, run + split IS/OOS stats."""
from __future__ import annotations

import pandas as pd

from .backtest import engine
from .config import CACHE
from .data import binance
from .universe import UNIVERSE

IS_END = "2023-12-31 23:00"
OOS_START = "2024-01-01"


def load_panel(symbols=None, start="2019-01-01", refresh=False) -> pd.DataFrame:
    symbols = symbols or list(UNIVERSE)
    dfs = binance.load_many(symbols, "1h", start=start, refresh=refresh)
    p = binance.panel(dfs, "close")
    full = pd.date_range(p.index[0], p.index[-1], freq="h", tz="UTC")
    return p.reindex(full).ffill(limit=3)


def load_volume_panel(symbols=None, start="2019-01-01") -> pd.DataFrame:
    symbols = symbols or list(UNIVERSE)
    dfs = binance.load_many(symbols, "1h", start=start, refresh=False)
    return binance.panel(dfs, "quote_volume")


def load_binance_funding_panel(prices: pd.DataFrame, default_8h: float = 0.0001, splice_recon: bool = False) -> pd.DataFrame:
    """Hourly funding panel aligned to `prices`. Missing history -> default rate (spread evenly, 8h).
    splice_recon=True: after each symbol's last monthly archive, use the live 1m-premium reconstruction
    (`data.funding_recon`, cached under cache/frecon) instead of the flat default (r55)."""
    cols = {}
    for sym in prices.columns:
        path = CACHE / f"binance_funding_{sym}.parquet"
        alt = CACHE / f"binance_funding_1000{sym}.parquet"
        s = None
        for pth in (path, alt):
            if pth.exists():
                df = pd.read_parquet(pth)
                if len(df):
                    s = df["rate"]
                    break
        if s is None:
            cols[sym] = pd.Series(default_8h / 8, index=prices.index)
            continue
        s.index = s.index.floor("h")
        s = s[~s.index.duplicated(keep="last")]
        # bar-open convention: funding at time t is paid on the position held over bar t-1
        h = s.reindex(prices.index).fillna(0.0)
        # for periods without data (before listing / after last archive) use default spread hourly
        covered = (prices.index >= s.index[0]) & (prices.index <= s.index[-1] + pd.Timedelta(hours=8))
        h[~covered] = default_8h / 8
        if splice_recon:
            try:
                from .data import funding_recon
                end = s.index[-1]
                r = funding_recon.build("1000PEPEUSDT" if sym == "PEPEUSDT" else sym,
                                        start=(end + pd.Timedelta(days=1)).strftime("%Y-%m"), workers=4)
                r.index = r.index.floor("h"); r = r[~r.index.duplicated(keep="last")]
                win = prices.index[(prices.index > end) & (prices.index <= r.index[-1] + pd.Timedelta(hours=8))] if len(r) else []
                if len(win):
                    h.loc[win] = r.reindex(win).fillna(0.0).to_numpy()
            except Exception as e:  # noqa: BLE001
                print(f"[funding] recon splice failed {sym}: {e}")
        cols[sym] = h
    return pd.DataFrame(cols, index=prices.index)


def evaluate(prices, weights, fee=0.00045, slippage=0.0005, funding=None, min_trade=0.0, label=""):
    res = engine.run(prices, weights, fee=fee, slippage=slippage, funding=funding, min_trade=min_trade)
    r = res.returns
    full = engine.perf_stats(r, turnover=res.turnover, weights=res.weights)
    is_ = engine.perf_stats(r[:IS_END], turnover=res.turnover[:IS_END], weights=res.weights[:IS_END])
    oos = engine.perf_stats(r[OOS_START:], turnover=res.turnover[OOS_START:], weights=res.weights[OOS_START:])
    return res, {"label": label, "full": full, "is": is_, "oos": oos}


def print_eval(ev):
    print(f"== {ev['label']}")
    for k in ("is", "oos", "full"):
        print(f"   {k:4s} {engine.fmt_stats(ev[k])}")
