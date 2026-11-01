"""Polymarket public data + forward snapshot collector (for calibration research and future paper trading).

Resolved-market price history is not available from the CLOB API and the trades API only exposes the most recent
~10k trades per market, so we collect our own forward dataset:
- every full cycle: markets ending within 7 days (near-expiry dynamics), top 500 by 24h volume
- once a day (00 UTC cycle): the full top-500 snapshot
Files: state/polymarket/snap_YYYY-MM-DD.csv.gz (append-only, small).
"""
from __future__ import annotations

import gzip
import io
import json

import pandas as pd

from ..config import STATE
from .http import get

GAMMA = "https://gamma-api.polymarket.com/markets"
OUT = STATE / "polymarket"
FIELDS = ["ts", "id", "condition_id", "question", "end", "best_bid", "best_ask", "last", "v24", "liquidity", "neg_risk"]


def active_markets(limit: int = 500, page_size: int = 100) -> pd.DataFrame:
    """Top `limit` active markets by 24h volume (the API caps pages at 100, so paginate)."""
    page = []
    for off in range(0, limit, page_size):
        try:
            chunk = get(GAMMA, params={"active": "true", "closed": "false", "limit": page_size, "offset": off,
                                       "order": "volume24hr", "ascending": "false"}, retries=2).json()
        except Exception:  # noqa: BLE001
            break
        if not chunk:
            break
        page.extend(chunk)
    rows = []
    seen = set()
    for m in page:
        if m.get("id") in seen:
            continue
        seen.add(m.get("id"))
        try:
            outs = json.loads(m.get("outcomes") or "[]")
        except Exception:  # noqa: BLE001
            outs = []
        if len(outs) != 2:
            continue
        rows.append({"id": m.get("id"), "condition_id": m.get("conditionId"), "question": (m.get("question") or "")[:140],
                     "end": m.get("endDate"), "best_bid": m.get("bestBid"), "best_ask": m.get("bestAsk"),
                     "last": m.get("lastTradePrice"), "v24": m.get("volume24hr"), "liquidity": m.get("liquidityNum"),
                     "neg_risk": bool(m.get("negRisk"))})
    return pd.DataFrame(rows)


def collect(now: pd.Timestamp) -> int:
    df = active_markets()
    if df.empty:
        return 0
    end = pd.to_datetime(df["end"], errors="coerce", utc=True)
    days_left = (end - now).dt.total_seconds() / 86400
    keep = df[(days_left > -1) & (days_left < 7)] if now.hour != 0 else df
    keep = keep.copy()
    keep.insert(0, "ts", now.floor("min").isoformat())
    OUT.mkdir(parents=True, exist_ok=True)
    path = OUT / f"snap_{now:%Y-%m-%d}.csv.gz"
    buf = io.StringIO()
    keep[FIELDS].to_csv(buf, index=False, header=not path.exists())
    with gzip.open(path, "at") as f:
        f.write(buf.getvalue())
    return len(keep)


def resolved_outcome(market_id: str) -> dict | None:
    """For calibration later: final outcome prices of a (closed) market."""
    r = get(f"{GAMMA}/{market_id}", retries=2).json()
    if not r.get("closed"):
        return None
    prices = [float(x) for x in json.loads(r.get("outcomePrices") or "[]")]
    return {"id": market_id, "yes_won": prices[0] > 0.5 if prices else None, "closed_time": r.get("closedTime")}
