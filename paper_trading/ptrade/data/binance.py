"""Binance public market data via the binance.vision mirrors (no API key needed).

- Spot klines (REST, paginated):   https://data-api.binance.vision/api/v3/klines
- USDT-M futures funding archives: https://data.binance.vision/data/futures/um/monthly/fundingRate/...
"""
from __future__ import annotations

import io
import time
import zipfile
from concurrent.futures import ThreadPoolExecutor

import pandas as pd

from ..config import CACHE, HOUR_MS
from .http import get

KLINES_URL = "https://data-api.binance.vision/api/v3/klines"
ARCHIVE = "https://data.binance.vision/data"

_INTERVAL_MS = {"1h": HOUR_MS, "4h": 4 * HOUR_MS, "1d": 24 * HOUR_MS, "15m": HOUR_MS // 4}


def _fetch_klines(symbol: str, interval: str, start_ms: int, end_ms: int | None = None) -> pd.DataFrame:
    rows = []
    cur = start_ms
    end_ms = end_ms or int(time.time() * 1000)
    while cur < end_ms:
        r = get(KLINES_URL, params={"symbol": symbol, "interval": interval, "startTime": cur, "limit": 1000})
        data = r.json()
        if not data:
            break
        rows.extend(data)
        nxt = data[-1][0] + _INTERVAL_MS[interval]
        if nxt <= cur or len(data) < 1000:
            break
        cur = nxt
    if not rows:
        return pd.DataFrame(columns=["open", "high", "low", "close", "volume", "quote_volume", "trades", "taker_buy_quote"])
    df = pd.DataFrame(rows, columns=["t", "open", "high", "low", "close", "volume", "T", "quote_volume", "trades",
                                     "taker_buy_base", "taker_buy_quote", "ignore"])
    df["t"] = pd.to_datetime(df["t"], unit="ms", utc=True)
    df = df.set_index("t")
    for c in ["open", "high", "low", "close", "volume", "quote_volume", "taker_buy_quote"]:
        df[c] = df[c].astype(float)
    df["trades"] = df["trades"].astype(int)
    return df[["open", "high", "low", "close", "volume", "quote_volume", "trades", "taker_buy_quote"]]


def load_klines(symbol: str, interval: str = "1h", start: str = "2019-01-01", refresh: bool = True) -> pd.DataFrame:
    """Load cached klines and incrementally update from the API. Drops the still-forming last bar."""
    path = CACHE / f"binance_{symbol}_{interval}.parquet"
    start_ms = int(pd.Timestamp(start, tz="UTC").timestamp() * 1000)
    if path.exists():
        df = pd.read_parquet(path)
        if refresh:
            last_ms = int(df.index[-1].timestamp() * 1000)
            new = _fetch_klines(symbol, interval, last_ms)
            if len(new):
                df = pd.concat([df[df.index < new.index[0]], new])
    else:
        df = _fetch_klines(symbol, interval, start_ms)
    df = df[~df.index.duplicated(keep="last")].sort_index()
    df.to_parquet(path)
    # drop the incomplete (current) bar
    now = pd.Timestamp.now(tz="UTC")
    step = pd.Timedelta(milliseconds=_INTERVAL_MS[interval])
    df = df[df.index + step <= now]
    return df[df.index >= pd.Timestamp(start, tz="UTC")]


def load_many(symbols, interval="1h", start="2019-01-01", refresh=True, workers=6) -> dict[str, pd.DataFrame]:
    out = {}

    def _one(s):
        try:
            return s, load_klines(s, interval, start, refresh)
        except Exception as e:  # noqa: BLE001
            print(f"[binance] failed {s}: {e}")
            return s, None

    with ThreadPoolExecutor(workers) as ex:
        for s, df in ex.map(_one, symbols):
            if df is not None and len(df):
                out[s] = df
    return out


def panel(dfs: dict[str, pd.DataFrame], field: str = "close") -> pd.DataFrame:
    return pd.DataFrame({k: v[field] for k, v in dfs.items()}).sort_index()


# ---------------------------------------------------------------- funding archives

def _funding_month(symbol: str, ym: str) -> pd.DataFrame | None:
    url = f"{ARCHIVE}/futures/um/monthly/fundingRate/{symbol}/{symbol}-fundingRate-{ym}.zip"
    try:
        r = get(url, retries=2)
    except Exception:  # noqa: BLE001
        return None
    with zipfile.ZipFile(io.BytesIO(r.content)) as z:
        name = z.namelist()[0]
        raw = z.read(name).decode()
    lines = raw.strip().splitlines()
    if lines and not lines[0][0].isdigit():
        lines = lines[1:]
    rows = [ln.split(",") for ln in lines]
    df = pd.DataFrame(rows).iloc[:, :3]
    df.columns = ["t", "interval_h", "rate"]
    df["t"] = pd.to_datetime(df["t"].astype("int64"), unit="ms", utc=True)
    df["rate"] = df["rate"].astype(float)
    return df.set_index("t")[["rate"]]


def load_funding(symbol: str, start: str = "2019-09") -> pd.Series:
    """Binance USDT-M perp funding rate per funding event (typically every 8h). Cached."""
    path = CACHE / f"binance_funding_{symbol}.parquet"
    have = pd.read_parquet(path) if path.exists() else None
    months = pd.period_range(start, pd.Timestamp.now(tz="UTC").tz_localize(None).to_period("M") - 1, freq="M")
    done = set()
    if have is not None and len(have):
        done = set(have.index.tz_convert(None).to_period("M").astype(str))
    todo = [str(m) for m in months if str(m) not in done]
    # only re-fetch the last 2 months if they are present (partial months are not published)
    parts = [have] if have is not None else []
    if todo:
        with ThreadPoolExecutor(8) as ex:
            for df in ex.map(lambda m: _funding_month(symbol, m), todo):
                if df is not None and len(df):
                    parts.append(df)
    if not parts:
        return pd.Series(dtype=float)
    out = pd.concat(parts)
    out = out[~out.index.duplicated(keep="last")].sort_index()
    out.to_parquet(path)
    return out["rate"]
