"""Paper trading book: cash + positions, fully-funded accounting (equity = cash + sum(qty * price)).

Position keys: "<COIN>" = Hyperliquid perp (incl. HIP-3 like "xyz:GOLD"), "<COIN>/spot" = spot leg
(priced at the HL oracle, used for delta-neutral hedges). Perp positions pay/receive hourly funding.
All trades and equity snapshots are appended to CSV logs under state/.
"""
from __future__ import annotations

import csv
import json
from dataclasses import asdict, dataclass, field
from pathlib import Path

import pandas as pd

from ..config import STATE

BOOKS_DIR = STATE / "books"
BOOKS_DIR.mkdir(parents=True, exist_ok=True)
TRADES_CSV = STATE / "trades.csv"
EQUITY_CSV = STATE / "equity.csv"
FUNDING_CSV = STATE / "funding.csv"

TRADE_FIELDS = ["ts", "book", "key", "side", "qty", "mid", "fill_px", "notional", "fee", "slip_cost", "reason"]
EQUITY_FIELDS = ["ts", "book", "equity", "cash", "gross", "net", "n_pos", "fees", "slippage", "funding"]


def _append(path: Path, fields: list[str], row: dict):
    new = not path.exists()
    with open(path, "a", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        if new:
            w.writeheader()
        w.writerow({k: row.get(k) for k in fields})


def is_spot(key: str) -> bool:
    return key.endswith("/spot")


def coin_of(key: str) -> str:
    return key[:-5] if is_spot(key) else key


@dataclass
class Book:
    name: str
    initial: float
    cash: float
    created: str
    positions: dict = field(default_factory=dict)  # key -> {qty, avg_px, opened, funded_through}
    fees: float = 0.0
    slippage: float = 0.0
    funding: float = 0.0
    realized: float = 0.0
    meta: dict = field(default_factory=dict)
    description: str = ""

    # ------------------------------------------------------------------ persistence
    @property
    def path(self) -> Path:
        return BOOKS_DIR / f"{self.name}.json"

    @classmethod
    def load_or_create(cls, name: str, initial: float, now: pd.Timestamp, description: str = "") -> "Book":
        p = BOOKS_DIR / f"{name}.json"
        if p.exists():
            d = json.loads(p.read_text())
            b = cls(**d)
            if description:
                b.description = description
            return b
        return cls(name=name, initial=initial, cash=initial, created=now.isoformat(), description=description)

    def save(self):
        self.path.write_text(json.dumps(asdict(self), indent=1, sort_keys=True, default=str))

    # ------------------------------------------------------------------ valuation
    def equity(self, prices: dict[str, float]) -> float:
        return self.cash + sum(p["qty"] * prices.get(k, p["avg_px"]) for k, p in self.positions.items())

    def weights(self, prices: dict[str, float]) -> dict[str, float]:
        eq = self.equity(prices)
        if eq <= 0:
            return {}
        return {k: p["qty"] * prices.get(k, p["avg_px"]) / eq for k, p in self.positions.items()}

    def exposure(self, prices) -> tuple[float, float]:
        w = self.weights(prices)
        return sum(abs(v) for v in w.values()), sum(w.values())

    # ------------------------------------------------------------------ trading
    def trade(self, key: str, dqty: float, mid: float, slip: float, fee_rate: float, now: pd.Timestamp,
              reason: str = ""):
        if dqty == 0 or not mid or mid <= 0:
            return
        side = "buy" if dqty > 0 else "sell"
        fill = mid * (1 + slip) if dqty > 0 else mid * (1 - slip)
        notional = abs(dqty) * fill
        fee = notional * fee_rate
        slip_cost = abs(dqty) * abs(fill - mid)
        self.cash -= dqty * fill + fee
        self.fees += fee
        self.slippage += slip_cost
        pos = self.positions.get(key)
        hour = now.floor("h").isoformat()
        if pos is None:
            self.positions[key] = {"qty": dqty, "avg_px": fill, "opened": now.isoformat(), "funded_through": hour}
        else:
            old = pos["qty"]
            new = old + dqty
            if old != 0 and (old > 0) != (dqty > 0):  # reducing / flipping: realise PnL on closed part
                closed = min(abs(dqty), abs(old)) * (1 if old > 0 else -1)
                self.realized += closed * (fill - pos["avg_px"])
            if abs(new) < 1e-12:
                del self.positions[key]
            else:
                if old == 0 or (old > 0) == (dqty > 0):
                    pos["avg_px"] = (old * pos["avg_px"] + dqty * fill) / new
                elif (old > 0) != (new > 0):  # flipped through zero
                    pos["avg_px"] = fill
                    pos["opened"] = now.isoformat()
                pos["qty"] = new
        _append(TRADES_CSV, TRADE_FIELDS, {
            "ts": now.isoformat(), "book": self.name, "key": key, "side": side, "qty": f"{dqty:.8g}",
            "mid": f"{mid:.8g}", "fill_px": f"{fill:.8g}", "notional": f"{notional:.2f}", "fee": f"{fee:.4f}",
            "slip_cost": f"{slip_cost:.4f}", "reason": reason})

    def accrue_funding(self, key: str, rates: pd.Series, px: float):
        """Apply hourly funding entries with timestamp > funded_through (perps only). rates: index=hour ts."""
        pos = self.positions.get(key)
        if pos is None or is_spot(key) or rates is None or len(rates) == 0:
            return 0.0
        ft = pd.Timestamp(pos["funded_through"])
        due = rates[rates.index > ft]
        if len(due) == 0:
            return 0.0
        pay = -pos["qty"] * px * float(due.sum())  # longs pay positive funding
        self.cash += pay
        self.funding += pay
        pos["funded_through"] = due.index[-1].isoformat()
        if abs(pay) > 0:
            _append(FUNDING_CSV, ["ts", "book", "key", "hours", "amount"], {
                "ts": due.index[-1].isoformat(), "book": self.name, "key": key, "hours": len(due),
                "amount": f"{pay:.4f}"})
        return pay

    def log_equity(self, prices, now: pd.Timestamp):
        g, n = self.exposure(prices)
        _append(EQUITY_CSV, EQUITY_FIELDS, {
            "ts": now.isoformat(), "book": self.name, "equity": f"{self.equity(prices):.2f}",
            "cash": f"{self.cash:.2f}", "gross": f"{g:.4f}", "net": f"{n:.4f}", "n_pos": len(self.positions),
            "fees": f"{self.fees:.2f}", "slippage": f"{self.slippage:.2f}", "funding": f"{self.funding:.2f}"})
