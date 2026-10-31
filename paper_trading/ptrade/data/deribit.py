"""Deribit public API: DVOL implied-volatility index history, realised vol, option chain summary. No key needed."""
from __future__ import annotations

import time

import pandas as pd

from ..config import CACHE
from .http import get

API = "https://www.deribit.com/api/v2/public"


def dvol_history(currency: str = "BTC", start: str = "2021-03-24", resolution: int = 3600) -> pd.DataFrame:
    """DVOL (30d implied vol index, annualised %) OHLC. Cached + incremental. resolution in seconds."""
    path = CACHE / f"deribit_dvol_{currency}_{resolution}.parquet"
    have = pd.read_parquet(path) if path.exists() else None
    start_ms = int(pd.Timestamp(start, tz="UTC").timestamp() * 1000)
    if have is not None and len(have):
        start_ms = int(have.index[-1].timestamp() * 1000)
    end_ms = int(time.time() * 1000)
    rows = []
    cur_end = end_ms
    while cur_end > start_ms:  # API pages backwards from end via `continuation`
        r = get(f"{API}/get_volatility_index_data", params={
            "currency": currency, "start_timestamp": start_ms, "end_timestamp": cur_end, "resolution": resolution}).json()
        res = r.get("result", {})
        data = res.get("data", [])
        if not data:
            break
        rows.extend(data)
        cont = res.get("continuation")
        if not cont or cont >= cur_end:
            break
        cur_end = cont
        time.sleep(0.1)
    df = pd.DataFrame(rows, columns=["t", "open", "high", "low", "close"])
    if len(df):
        df["t"] = pd.to_datetime(df["t"], unit="ms", utc=True)
        df = df.set_index("t").astype(float)
    if have is not None:
        df = pd.concat([have, df])
    df = df[~df.index.duplicated(keep="last")].sort_index()
    df.to_parquet(path)
    return df


def option_summary(currency: str = "BTC") -> pd.DataFrame:
    """Snapshot of every listed option: mark IV, OI, volume, underlying, mid."""
    r = get(f"{API}/get_book_summary_by_currency", params={"currency": currency, "kind": "option"}).json()["result"]
    df = pd.DataFrame(r)
    parts = df["instrument_name"].str.split("-", expand=True)
    df["expiry"] = pd.to_datetime(parts[1], format="%d%b%y", utc=True) + pd.Timedelta(hours=8)
    df["strike"] = parts[2].astype(float)
    df["cp"] = parts[3]
    return df


def index_price(currency: str = "BTC") -> float:
    return float(get(f"{API}/get_index_price", params={"index_name": f"{currency.lower()}_usd"}).json()["result"]["index_price"])
