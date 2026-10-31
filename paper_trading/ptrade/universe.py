"""Tradeable universe: Binance spot symbol (for long history) <-> Hyperliquid perp coin (for live execution).

HL quotes some memecoins in thousands (kPEPE = 1000 PEPE); `HL_MULT` maps the Binance price to HL units.
"""

UNIVERSE = {
    # binance symbol : hyperliquid coin
    "BTCUSDT": "BTC", "ETHUSDT": "ETH", "SOLUSDT": "SOL", "XRPUSDT": "XRP", "BNBUSDT": "BNB",
    "DOGEUSDT": "DOGE", "ADAUSDT": "ADA", "AVAXUSDT": "AVAX", "LINKUSDT": "LINK", "DOTUSDT": "DOT",
    "LTCUSDT": "LTC", "BCHUSDT": "BCH", "TRXUSDT": "TRX", "NEARUSDT": "NEAR", "ATOMUSDT": "ATOM",
    "UNIUSDT": "UNI", "AAVEUSDT": "AAVE", "FILUSDT": "FIL", "ARBUSDT": "ARB", "OPUSDT": "OP",
    "SUIUSDT": "SUI", "APTUSDT": "APT", "INJUSDT": "INJ", "TIAUSDT": "TIA", "SEIUSDT": "SEI",
    "WLDUSDT": "WLD", "ENAUSDT": "ENA", "PEPEUSDT": "kPEPE", "XLMUSDT": "XLM", "ETCUSDT": "ETC",
    "HBARUSDT": "HBAR", "TAOUSDT": "TAO", "ZECUSDT": "ZEC", "CRVUSDT": "CRV", "LDOUSDT": "LDO",
    "ONDOUSDT": "ONDO", "JUPUSDT": "JUP", "PENDLEUSDT": "PENDLE", "RENDERUSDT": "RENDER", "ALGOUSDT": "ALGO",
}

HL_MULT = {"kPEPE": 1000.0}

MAJORS = ["BTCUSDT", "ETHUSDT", "SOLUSDT"]


def hl_coin(binance_symbol: str) -> str:
    return UNIVERSE[binance_symbol]


def binance_symbol(hl: str) -> str:
    inv = {v: k for k, v in UNIVERSE.items()}
    return inv[hl]
