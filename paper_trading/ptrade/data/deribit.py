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


# ------------------------------------------------------------------ option skew (forward dataset)

def _bs_delta(F, K, T, iv, cp):
    import math
    from statistics import NormalDist
    if T <= 0 or iv <= 0:
        return float("nan")
    d1 = (math.log(F / K) + 0.5 * iv * iv * T) / (iv * math.sqrt(T))
    n = NormalDist().cdf(d1)
    return n if cp == "C" else n - 1


def skew_snapshot(currency: str = "BTC", now: pd.Timestamp | None = None, tenors=(7, 30)) -> list[dict]:
    """ATM IV, 25-delta risk reversal / butterfly per target tenor (nearest listed expiry), plus put/call ratios.
    Historical skew isn't available from free APIs, so this is collected forward every full cycle."""
    import numpy as np
    now = now or pd.Timestamp.now(tz="UTC")
    df = option_summary(currency)
    df = df[(df["mark_iv"] > 0) & df["underlying_price"].notna()]
    df["days"] = (df["expiry"] - now).dt.total_seconds() / 86400
    puts, calls = df[df.cp == "P"], df[df.cp == "C"]
    base = {"ts": now.floor("min").isoformat(), "currency": currency,
            "pcr_oi": round(float(puts.open_interest.sum() / max(calls.open_interest.sum(), 1e-9)), 4),
            "pcr_vol": round(float(puts.volume.sum() / max(calls.volume.sum(), 1e-9)), 4)}
    out = []
    exps = df.groupby("expiry")["days"].first()
    exps = exps[exps > 2]
    for tenor in tenors:
        if exps.empty:
            break
        exp = (exps - tenor).abs().idxmin()
        e = df[df.expiry == exp].copy()
        F = float(e["underlying_price"].median()); T = float(e["days"].iloc[0]) / 365
        otm = e[((e.cp == "C") & (e.strike >= F)) | ((e.cp == "P") & (e.strike < F))].copy()
        otm["iv"] = otm["mark_iv"] / 100
        otm["delta"] = [_bs_delta(F, k, T, v, c) for k, v, c in zip(otm.strike, otm.iv, otm.cp)]
        c, p = otm[otm.cp == "C"].sort_values("delta"), otm[otm.cp == "P"].sort_values("delta")
        def at(d, frame):
            frame = frame.dropna(subset=["delta"])
            return float(np.interp(d, frame["delta"], frame["iv"])) if len(frame) >= 2 else float("nan")
        lk = np.log(otm["strike"] / F)
        atm = float(np.interp(0.0, lk.sort_values(), otm.loc[lk.sort_values().index, "iv"])) if len(otm) >= 2 else float("nan")
        c25, p25 = at(0.25, c), at(-0.25, p)
        out.append({**base, "tenor": tenor, "expiry": exp.isoformat(), "days": round(T * 365, 2), "fwd": round(F, 2),
                    "atm_iv": round(atm * 100, 2), "rr25": round((c25 - p25) * 100, 2),
                    "bf25": round(((c25 + p25) / 2 - atm) * 100, 2)})
    return out


def collect_skew(now: pd.Timestamp) -> int:
    import csv
    from ..config import STATE
    d = STATE / "options"
    d.mkdir(parents=True, exist_ok=True)
    path = d / f"skew_{now:%Y-%m}.csv"
    rows = []
    for cur in ("BTC", "ETH"):
        rows += skew_snapshot(cur, now)
    if not rows:
        return 0
    new = not path.exists()
    with open(path, "a", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        if new:
            w.writeheader()
        w.writerows(rows)
    return len(rows)
