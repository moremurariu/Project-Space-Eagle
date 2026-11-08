"""Binance public market data via the binance.vision mirrors (no API key needed).

- Spot klines (REST, paginated):   https://data-api.binance.vision/api/v3/klines
- USDT-M futures funding archives: https://data.binance.vision/data/futures/um/monthly/fundingRate/...
"""
from __future__ import annotations

import io
import time
import zipfile
from concurrent.futures import ThreadPoolExecutor

import numpy as np
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
            if refresh:  # keep the coin in the panel from cache (stale) instead of silently dropping it
                try:
                    return s, load_klines(s, interval, start, refresh=False)
                except Exception:  # noqa: BLE001
                    pass
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


# ---------------------------------------------------------------- futures positioning metrics (OI, long/short ratios)

def _metrics_day(symbol: str, day: str) -> pd.DataFrame | None:
    url = f"{ARCHIVE}/futures/um/daily/metrics/{symbol}/{symbol}-metrics-{day}.zip"
    try:
        r = get(url, retries=2)
    except Exception:  # noqa: BLE001
        return None
    with zipfile.ZipFile(io.BytesIO(r.content)) as z:
        df = pd.read_csv(io.BytesIO(z.read(z.namelist()[0])))
    df["t"] = pd.to_datetime(df["create_time"], utc=True)
    cols = {"sum_open_interest": "oi", "sum_open_interest_value": "oi_usd",
            "count_toptrader_long_short_ratio": "top_ls_accounts", "sum_toptrader_long_short_ratio": "top_ls_positions",
            "count_long_short_ratio": "ls_accounts", "sum_taker_long_short_vol_ratio": "taker_ls"}
    return df.set_index("t")[list(cols)].rename(columns=cols).astype(float)


def load_metrics(symbol: str, start: str = "2022-01-01", workers: int = 8) -> pd.DataFrame:
    """Binance USDT-M positioning metrics (5-min, published daily with ~1 day lag), resampled to hourly. Cached."""
    path = CACHE / f"binance_metrics_{symbol}.parquet"
    have = pd.read_parquet(path) if path.exists() else None
    last_day = pd.Timestamp.now(tz="UTC").normalize() - pd.Timedelta(days=1)
    first = pd.Timestamp(start, tz="UTC") if have is None or not len(have) else have.index[-1].normalize() + pd.Timedelta(days=1)
    days = [d.strftime("%Y-%m-%d") for d in pd.date_range(first, last_day, freq="D")]
    parts = [have] if have is not None else []
    if days:
        with ThreadPoolExecutor(workers) as ex:
            for df in ex.map(lambda d: _metrics_day(symbol, d), days):
                if df is not None and len(df):
                    parts.append(df.resample("h").last())
    if not parts:
        return pd.DataFrame()
    out = pd.concat(parts)
    out = out[~out.index.duplicated(keep="last")].sort_index()
    out.to_parquet(path)
    return out


# ---------------------------------------------------------------- premium index (→ reconstructed funding, live-able)

def _premium_file(symbol: str, kind: str, period: str) -> pd.DataFrame | None:
    url = (f"{ARCHIVE}/futures/um/{kind}/premiumIndexKlines/{symbol}/1h/{symbol}-1h-{period}.zip")
    try:
        r = get(url, retries=2)
    except Exception:  # noqa: BLE001
        return None
    with zipfile.ZipFile(io.BytesIO(r.content)) as z:
        raw = z.read(z.namelist()[0]).decode()
    lines = [ln for ln in raw.strip().splitlines() if ln and ln[0].isdigit()]
    if not lines:
        return None
    df = pd.DataFrame([ln.split(",")[:5] for ln in lines], columns=["t", "open", "high", "low", "close"])
    df["t"] = pd.to_datetime(df["t"].astype("int64"), unit="ms", utc=True)
    return df.set_index("t").astype(float)


def load_premium_index(symbol: str, start: str = "2024-01", workers: int = 8) -> pd.DataFrame:
    """Hourly premium-index klines for a USDT-M perp: monthly archives for complete months + daily for the rest
    (daily files appear with ~1 day lag). Cached."""
    path = CACHE / f"binance_premium_{symbol}.parquet"
    have = pd.read_parquet(path) if path.exists() else None
    now = pd.Timestamp.now(tz="UTC")
    this_month = now.tz_localize(None).to_period("M")
    done_months = set()
    if have is not None and len(have):
        cnt = have.groupby(have.index.tz_localize(None).to_period("M")).size()
        done_months = {str(m) for m, n in cnt.items() if n >= 24 * 27}
    months = [str(m) for m in pd.period_range(start, this_month - 1, freq="M") if str(m) not in done_months]
    jobs = [("monthly", m) for m in months]
    # current month (and last month if its monthly file isn't out yet) via daily files
    have_days = set() if have is None else {d.strftime("%Y-%m-%d") for d in have.index.normalize().unique()}
    for d in pd.date_range((this_month - 1).start_time.tz_localize("UTC"), now.normalize() - pd.Timedelta(days=1), freq="D"):
        ds = d.strftime("%Y-%m-%d")
        if ds not in have_days:
            jobs.append(("daily", ds))
    parts = [have] if have is not None else []
    with ThreadPoolExecutor(workers) as ex:
        for df in ex.map(lambda j: _premium_file(symbol, *j), jobs):
            if df is not None and len(df):
                parts.append(df)
    if not parts:
        return pd.DataFrame()
    out = pd.concat(parts)
    out = out[~out.index.duplicated(keep="last")].sort_index()
    out.to_parquet(path)
    return out


def reconstruct_funding(prem: pd.DataFrame, interval_h: int = 8, interest: float = 0.0001,
                        clamp: float = 0.0005, cap: float = 0.02) -> pd.Series:
    """Approximate Binance funding per settlement from hourly premium index:
    F = clamp(P + clamp(I - P, ±0.05%), ±cap), P = linearly-weighted mean premium over the interval.
    Returned as an hourly series with the rate on settlement hours (00/08/16 UTC) and 0 elsewhere."""
    p = (prem["open"] + prem["close"]) / 2
    w = np.arange(1, interval_h + 1, dtype=float)
    wavg = p.rolling(interval_h).apply(lambda x: float(np.dot(x, w) / w.sum()), raw=True)
    f = (wavg + (interest - wavg).clip(-clamp, clamp)).clip(-cap, cap)
    settle = f.index + pd.Timedelta(hours=1)  # bar opening at 23:00 closes at the 00:00 settlement
    out = pd.Series(0.0, index=prem.index)
    mask = (settle.hour % interval_h) == 0
    out[mask] = f[mask]
    return out
