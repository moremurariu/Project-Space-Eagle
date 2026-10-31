"""Sentiment / alternative data: Crypto Fear & Greed index and crypto news RSS headlines."""
from __future__ import annotations

import re
import xml.etree.ElementTree as ET

import pandas as pd

from .http import get

FNG_URL = "https://api.alternative.me/fng/"
RSS_FEEDS = {
    "cointelegraph": "https://cointelegraph.com/rss",
}


def fear_greed(limit: int = 0) -> pd.Series:
    """Daily Fear & Greed index (0 = extreme fear, 100 = extreme greed). limit=0 -> full history."""
    data = get(FNG_URL, params={"limit": limit, "format": "json"}).json()["data"]
    s = pd.Series({pd.Timestamp(int(d["timestamp"]), unit="s", tz="UTC"): int(d["value"]) for d in data})
    return s.sort_index()


def headlines(max_items: int = 30) -> list[dict]:
    out = []
    for src, url in RSS_FEEDS.items():
        try:
            root = ET.fromstring(get(url, retries=2).content)
        except Exception:  # noqa: BLE001
            continue
        for item in root.iter("item"):
            title = (item.findtext("title") or "").strip()
            pub = item.findtext("pubDate") or ""
            out.append({"source": src, "title": re.sub(r"\s+", " ", title), "published": pub})
            if len(out) >= max_items:
                break
    return out
