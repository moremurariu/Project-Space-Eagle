"""Global paths and constants."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CACHE = ROOT / "cache"
STATE = ROOT / "state"
REPORTS = ROOT / "reports"
for _p in (CACHE, STATE, REPORTS):
    _p.mkdir(parents=True, exist_ok=True)

HOUR_MS = 3_600_000
DAY_MS = 24 * HOUR_MS

# Hyperliquid fee schedule (base tier): perps taker 0.045%, maker 0.015%
HL_TAKER_FEE = 0.00045
HL_MAKER_FEE = 0.00015
# Default assumed slippage (one-way, fraction of price) on top of fees for liquid majors
DEFAULT_SLIPPAGE = 0.0002
