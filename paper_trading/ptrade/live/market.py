"""Live market snapshot for paper execution on Hyperliquid (mids, oracle, funding, book-walk slippage)."""
from __future__ import annotations

import time

import pandas as pd

from ..config import HL_TAKER_FEE
from ..data import hyperliquid as hl

SPOT_FEE = 0.0007          # HL spot taker (base tier); used for hedge legs
MAX_SLIP = 0.01


class Market:
    def __init__(self, dexes=("", "xyz")):
        self.now = pd.Timestamp.now(tz="UTC")
        frames = []
        for d in dexes:
            try:
                frames.append(hl.meta_and_ctxs(d))
            except Exception as e:  # noqa: BLE001
                print(f"[market] dex '{d}' failed: {e}")
        self.ctx = pd.concat(frames) if frames else pd.DataFrame()
        self._slip_cache: dict = {}
        self._funding_cache: dict = {}

    # prices -------------------------------------------------------------
    def mid(self, coin: str) -> float:
        r = self.ctx.loc[coin]
        m = r["mid"]
        return float(m if pd.notna(m) and m > 0 else r["mark"])

    def oracle(self, coin: str) -> float:
        return float(self.ctx.loc[coin, "oracle"])

    def price(self, key: str) -> float:
        if key.endswith("/spot"):
            return self.oracle(key[:-5])
        return self.mid(key)

    def prices(self, keys) -> dict[str, float]:
        out = {}
        for k in keys:
            try:
                out[k] = self.price(k)
            except KeyError:
                pass
        return out

    def has(self, coin: str) -> bool:
        return coin in self.ctx.index and not bool(self.ctx.loc[coin, "delisted"])

    # costs --------------------------------------------------------------
    def slippage(self, key: str, notional: float, side: str) -> float:
        coin = key[:-5] if key.endswith("/spot") else key
        bucket = (coin, side, round(notional, -2))
        if bucket in self._slip_cache:
            return self._slip_cache[bucket]
        try:
            s = hl.book_slippage(coin, notional, side)
            time.sleep(0.05)
        except Exception:  # noqa: BLE001
            s = 0.001
        s = min(max(s, 0.00005), MAX_SLIP)
        if key.endswith("/spot"):
            s = max(s, 0.0005)  # spot hedge executed elsewhere: assume at least 5bp
        self._slip_cache[bucket] = s
        return s

    @staticmethod
    def fee(key: str) -> float:
        return SPOT_FEE if key.endswith("/spot") else HL_TAKER_FEE

    # funding ------------------------------------------------------------
    def funding_since(self, coin: str, since: pd.Timestamp) -> pd.Series:
        key = (coin, since.floor("h"))
        if key in self._funding_cache:
            return self._funding_cache[key]
        try:
            df = hl.funding_history(coin, int(since.timestamp() * 1000) + 1)
            s = df["funding"] if len(df) else pd.Series(dtype=float)
        except Exception as e:  # noqa: BLE001
            print(f"[market] funding history failed {coin}: {e}")
            s = pd.Series(dtype=float)
        self._funding_cache[key] = s
        return s
