"""Sentiment / alternative data: Crypto Fear & Greed index and crypto news RSS headlines."""
from __future__ import annotations

import datetime
import re
import xml.etree.ElementTree as ET

import pandas as pd

from .http import get

FNG_URL = "https://api.alternative.me/fng/"
RSS_FEEDS = {
    "cointelegraph": "https://cointelegraph.com/rss",
    "coindesk": "https://www.coindesk.com/arc/outboundfeeds/rss/",
    "decrypt": "https://decrypt.co/feed",
    "theblock": "https://www.theblock.co/rss.xml",
    "dlnews": "https://www.dlnews.com/arc/outboundfeeds/rss/",
    # "bitcoinmagazine": dropped 2026-11-08 — returns 403 to this client since ~11-07 (not worked around)
    "cryptoslate": "https://cryptoslate.com/feed/",
}


def fear_greed(limit: int = 0) -> pd.Series:
    """Daily Fear & Greed index (0 = extreme fear, 100 = extreme greed). limit=0 -> full history."""
    data = get(FNG_URL, params={"limit": limit, "format": "json"}).json()["data"]
    s = pd.Series({pd.Timestamp(int(d["timestamp"]), unit="s", tz="UTC"): int(d["value"]) for d in data})
    return s.sort_index()


def headlines(max_items: int = 30, feeds: dict | None = None) -> list[dict]:
    out = []
    for src, url in (feeds or {"cointelegraph": RSS_FEEDS["cointelegraph"]}).items():
        try:
            root = ET.fromstring(get(url, retries=2).content)
        except Exception:  # noqa: BLE001
            continue
        n = 0
        for item in root.iter("item"):   # max_items is per feed (was a global cap that starved the last feeds)
            title = (item.findtext("title") or "").strip()
            pub = item.findtext("pubDate") or ""
            out.append({"source": src, "title": re.sub(r"\s+", " ", title), "published": pub})
            n += 1
            if n >= max_items:
                break
    return out


def collect_headlines(now) -> int:
    """Append new (deduplicated) headlines from all feeds to state/news/headlines_YYYY-MM.csv — forward dataset for
    news-shock event studies (historical crypto news with timestamps isn't available from free APIs here)."""
    import csv
    from email.utils import parsedate_to_datetime
    from ..config import STATE
    d = STATE / "news"
    d.mkdir(parents=True, exist_ok=True)
    path = d / f"headlines_{now:%Y-%m}.csv"
    seen = set()
    if path.exists():
        with open(path, newline="") as f:
            seen = {(r["source"], r["title"]) for r in csv.DictReader(f)}
    rows = []
    for h in headlines(max_items=500, feeds=RSS_FEEDS):
        key = (h["source"], h["title"])
        if not h["title"] or key in seen:
            continue
        seen.add(key)
        try:
            pub = parsedate_to_datetime(h["published"]).astimezone(datetime.timezone.utc).isoformat()
        except Exception:  # noqa: BLE001
            pub = ""
        rows.append({"seen_at": now.isoformat(), "published": pub, "source": h["source"], "title": h["title"]})
    new = not path.exists()
    with open(path, "a", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["seen_at", "published", "source", "title"])
        if new:
            w.writeheader()
        w.writerows(rows)
    return len(rows)
