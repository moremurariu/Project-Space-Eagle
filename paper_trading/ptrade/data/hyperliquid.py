"""Hyperliquid public info API (POST https://api.hyperliquid.xyz/info). No key needed for reads."""
from __future__ import annotations

import time

import pandas as pd

from ..config import CACHE, HOUR_MS
from .http import post_json

INFO = "https://api.hyperliquid.xyz/info"

_INTERVAL_MS = {"1m": 60_000, "5m": 300_000, "15m": 900_000, "1h": HOUR_MS, "4h": 4 * HOUR_MS, "1d": 24 * HOUR_MS}


def info(payload: dict):
    return post_json(INFO, payload)


def meta_and_ctxs(dex: str = "") -> pd.DataFrame:
    """Universe + live contexts (mark, oracle, funding, OI, volume) for a perp dex ('' = main, 'xyz' = HIP-3)."""
    payload = {"type": "metaAndAssetCtxs"}
    if dex:
        payload["dex"] = dex
    meta, ctxs = info(payload)
    rows = []
    for u, c in zip(meta["universe"], ctxs):
        if c is None:
            continue
        rows.append({
            "coin": u["name"],
            "delisted": bool(u.get("isDelisted", False)),
            "max_leverage": u.get("maxLeverage"),
            "sz_decimals": u.get("szDecimals"),
            "mark": float(c.get("markPx") or "nan"),
            "mid": float(c["midPx"]) if c.get("midPx") else float("nan"),
            "oracle": float(c.get("oraclePx") or "nan"),
            "funding": float(c.get("funding") or 0.0),  # hourly rate
            "premium": float(c["premium"]) if c.get("premium") else float("nan"),
            "open_interest": float(c.get("openInterest") or 0.0),
            "day_ntl_vlm": float(c.get("dayNtlVlm") or 0.0),
            "prev_day_px": float(c.get("prevDayPx") or "nan"),
            "impact_bid": float(c["impactPxs"][0]) if c.get("impactPxs") else float("nan"),
            "impact_ask": float(c["impactPxs"][1]) if c.get("impactPxs") else float("nan"),
        })
    df = pd.DataFrame(rows).set_index("coin")
    df["oi_usd"] = df["open_interest"] * df["mark"]
    return df


def all_mids(dex: str = "") -> dict[str, float]:
    payload = {"type": "allMids"}
    if dex:
        payload["dex"] = dex
    return {k: float(v) for k, v in info(payload).items()}


def candles(coin: str, interval: str = "1h", start_ms: int | None = None, end_ms: int | None = None) -> pd.DataFrame:
    """Candles (HL keeps only the most recent 5000 per interval)."""
    end_ms = end_ms or int(time.time() * 1000)
    start_ms = start_ms or end_ms - 5000 * _INTERVAL_MS[interval]
    rows, cur = [], start_ms
    while cur < end_ms:
        data = info({"type": "candleSnapshot", "req": {"coin": coin, "interval": interval, "startTime": cur, "endTime": end_ms}})
        if not data:
            break
        rows.extend(data)
        nxt = data[-1]["t"] + _INTERVAL_MS[interval]
        if nxt <= cur or len(data) < 500:
            break
        cur = nxt
    if not rows:
        return pd.DataFrame()
    df = pd.DataFrame(rows)
    df["t"] = pd.to_datetime(df["t"], unit="ms", utc=True)
    df = df.drop_duplicates("t").set_index("t").sort_index()
    out = df[["o", "h", "l", "c", "v"]].astype(float)
    out.columns = ["open", "high", "low", "close", "volume"]
    out["trades"] = df["n"].astype(int)
    return out


def funding_history(coin: str, start_ms: int, end_ms: int | None = None) -> pd.DataFrame:
    """Hourly funding + premium history (paginated, 500 per call)."""
    end_ms = end_ms or int(time.time() * 1000)
    rows, cur = [], start_ms
    while cur < end_ms:
        data = info({"type": "fundingHistory", "coin": coin, "startTime": cur, "endTime": end_ms})
        if not data:
            break
        rows.extend(data)
        nxt = data[-1]["time"] + 1
        if nxt <= cur or len(data) < 500:
            break
        cur = nxt
    if not rows:
        return pd.DataFrame(columns=["funding", "premium"])
    df = pd.DataFrame(rows)
    df["t"] = pd.to_datetime(df["time"], unit="ms", utc=True).dt.floor("h")
    df = df.drop_duplicates("t").set_index("t").sort_index()
    return pd.DataFrame({"funding": df["fundingRate"].astype(float), "premium": df["premium"].astype(float)})


def load_funding_history(coin: str, start: str = "2023-05-01", refresh: bool = True) -> pd.DataFrame:
    safe = coin.replace(":", "_")
    path = CACHE / f"hl_funding_{safe}.parquet"
    start_ms = int(pd.Timestamp(start, tz="UTC").timestamp() * 1000)
    if path.exists():
        df = pd.read_parquet(path)
        if refresh and len(df):
            new = funding_history(coin, int(df.index[-1].timestamp() * 1000) + 1)
            if len(new):
                df = pd.concat([df, new])
    else:
        df = funding_history(coin, start_ms)
    df = df[~df.index.duplicated(keep="last")].sort_index()
    df.to_parquet(path)
    return df


def l2_book(coin: str) -> dict:
    return info({"type": "l2Book", "coin": coin})


def book_slippage(coin: str, notional_usd: float, side: str) -> float:
    """Estimated one-way slippage vs mid (fraction) to fill `notional_usd` by walking the L2 book."""
    b = l2_book(coin)
    bids, asks = b["levels"][0], b["levels"][1]
    if not bids or not asks:
        return 0.002
    mid = (float(bids[0]["px"]) + float(asks[0]["px"])) / 2
    levels = asks if side == "buy" else bids
    remaining, cost, qty = notional_usd, 0.0, 0.0
    for lv in levels:
        px, sz = float(lv["px"]), float(lv["sz"])
        take = min(remaining, px * sz)
        cost += take
        qty += take / px
        remaining -= take
        if remaining <= 1e-9:
            break
    if qty == 0:
        return 0.002
    if remaining > 1e-9:  # book too thin: penalise
        return 0.01
    avg = cost / qty
    return abs(avg / mid - 1)


def _walk(levels, notional):
    """Average fill price walking `levels` for `notional` USD; None if the visible book is too thin."""
    remaining, cost, qty = notional, 0.0, 0.0
    for lv in levels:
        px, sz = float(lv["px"]), float(lv["sz"])
        take = min(remaining, px * sz)
        cost += take; qty += take / px; remaining -= take
        if remaining <= 1e-9:
            return cost / qty
    return None


def collect_xyz_depth(now, top_n: int = 45, sizes=(2500, 5000, 10000)) -> int:
    """Hourly L2 snapshot of HIP-3 xyz perps (top by OI): spread and book-walk slippage at a few sizes, both sides.
    Forward dataset for name-level execution costs of `xyz_weekend_fade` (2026-11-08: thin pre-IPO names cost 1%+)."""
    import csv, time
    from ..config import STATE
    d = STATE / "xyz_depth"; d.mkdir(parents=True, exist_ok=True)
    path = d / f"depth_{now:%Y-%m}.csv"
    m = meta_and_ctxs("xyz"); m = m[~m["delisted"]].sort_values("oi_usd", ascending=False).head(top_n)
    rows = []
    for c in m.index:
        try:
            b = l2_book(c); bids, asks = b["levels"][0], b["levels"][1]
        except Exception:  # noqa: BLE001
            continue
        if not bids or not asks:
            continue
        bb, ba = float(bids[0]["px"]), float(asks[0]["px"]); mid = (bb + ba) / 2
        r = {"ts": now.isoformat(), "coin": c, "mid": mid, "spread_bp": round((ba - bb) / mid * 1e4, 2),
             "oi_usd": round(float(m.at[c, "oi_usd"])), "vol24_usd": round(float(m.at[c, "day_ntl_vlm"]))}
        for s in sizes:
            fb, fs = _walk(asks, s), _walk(bids, s)
            r[f"buy{s}_bp"] = round((fb / mid - 1) * 1e4, 1) if fb else ""
            r[f"sell{s}_bp"] = round((1 - fs / mid) * 1e4, 1) if fs else ""
        rows.append(r); time.sleep(0.05)
    if rows:
        new = not path.exists()
        with open(path, "a", newline="") as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0])); new and w.writeheader(); w.writerows(rows)
    return len(rows)
