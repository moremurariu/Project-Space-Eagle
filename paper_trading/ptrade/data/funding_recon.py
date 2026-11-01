"""Binance USDT-M funding reconstructed from 1-minute premium-index archives (data.binance.vision).

Binance's fapi is geo-blocked here and funding archives are monthly only, but 1m `premiumIndexKlines` are published
daily (~1 day lag). Funding per settlement = P + clamp(I − P, ±0.05%), P = linearly time-weighted mean premium over
the interval, I = interest per interval (0.01%/8h by default; per-coin baseline estimated from actual archives).
r21 validation (Aug-Sep 2026, 40 coins): median per-settlement corr 0.99, MAE ≈ 0.05bp.
"""
from __future__ import annotations

import io
import zipfile
from concurrent.futures import ThreadPoolExecutor

import numpy as np
import pandas as pd

from ..config import CACHE
from .http import get

ARCH = "https://data.binance.vision/data/futures/um"
OUT = CACHE / "frecon"
OUT.mkdir(parents=True, exist_ok=True)


def _fetch_1m(sym: str, kind: str, period: str) -> pd.Series | None:
    url = f"{ARCH}/{kind}/premiumIndexKlines/{sym}/1m/{sym}-1m-{period}.zip"
    try:
        r = get(url, retries=2)
    except Exception:  # noqa: BLE001
        return None
    z = zipfile.ZipFile(io.BytesIO(r.content))
    raw = z.read(z.namelist()[0]).decode().splitlines()
    rows = [ln.split(",")[:5] for ln in raw if ln and ln[0].isdigit()]
    if not rows:
        return None
    a = np.array(rows)
    t = pd.to_datetime(a[:, 0].astype("int64"), unit="ms", utc=True)
    p = (a[:, 1].astype(float) + a[:, 4].astype(float)) / 2
    return pd.Series(p, index=t).sort_index()


def actual_profile(sym: str) -> tuple[int, float]:
    """(interval_h, interest per interval) from the latest actual funding archive months."""
    f = CACHE / f"binance_funding_{sym}.parquet"
    if not f.exists():
        return 8, 0.0001
    s = pd.read_parquet(f)["rate"].iloc[-200:]
    idx = s.index.floor("min").round("h")
    iv = int(round(pd.Series(idx).diff().dt.total_seconds().median() / 3600)) or 8
    mode = s.round(7).mode()
    base = float(mode.iloc[0]) if len(mode) else 0.0001 * iv / 8
    if abs(base) > 0.0003:  # mode isn't a baseline (always-extreme coin) -> default
        base = 0.0001 * iv / 8
    return iv, base


def settle(p1m: pd.Series, interval_h: int, interest: float) -> pd.Series:
    """Funding at each settlement time (end of interval) from 1m premium."""
    end = p1m.index.ceil(f"{interval_h}h")
    end = end.where(end != p1m.index, p1m.index + pd.Timedelta(hours=interval_h))  # minute at exact boundary -> next
    df = pd.DataFrame({"p": p1m.values, "end": end})
    df["w"] = df.groupby("end").cumcount() + 1.0
    g = df.assign(pw=df.p * df.w).groupby("end")
    n = g.size()
    P = g.pw.sum() / g.w.sum()
    P = P[n >= interval_h * 50]  # require mostly complete windows
    return P + (interest - P).clip(-0.0005, 0.0005)


def build(sym: str, start: str = "2024-01", workers: int = 4) -> pd.Series:
    """Reconstructed settlement funding (cached per month / per day)."""
    iv, base = actual_profile(sym)
    now = pd.Timestamp.now(tz="UTC")
    this_m = now.tz_localize(None).to_period("M")
    jobs = []
    for m in pd.period_range(start, this_m - 1, freq="M"):
        if not (OUT / f"{sym}_{m}.parquet").exists():
            jobs.append(("monthly", str(m)))
    for d in pd.date_range((this_m - 1).start_time.tz_localize("UTC"), now.normalize() - pd.Timedelta(days=1), freq="D"):
        ds = d.strftime("%Y-%m-%d")
        if not (OUT / f"{sym}_{ds}.parquet").exists() and not (OUT / f"{sym}_{d.tz_localize(None).to_period('M')}.parquet").exists():
            jobs.append(("daily", ds))

    def run(job):
        kind, per = job
        p = _fetch_1m(sym, kind, per)
        if p is None or not len(p):
            return
        s = settle(p, iv, base)
        pd.DataFrame({"rate": s}).to_parquet(OUT / f"{sym}_{per}.parquet")

    with ThreadPoolExecutor(workers) as ex:
        list(ex.map(run, jobs))
    parts = [pd.read_parquet(f)["rate"] for f in sorted(OUT.glob(f"{sym}_*.parquet"))]
    if not parts:
        return pd.Series(dtype=float)
    s = pd.concat(parts)
    return s[~s.index.duplicated(keep="last")].sort_index()
