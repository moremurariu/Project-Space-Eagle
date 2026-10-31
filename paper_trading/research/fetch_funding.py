"""Download funding histories: Binance USDT-M (archives) and Hyperliquid (all listed perps)."""
import sys
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0, ".")
from ptrade.data import binance, hyperliquid as hl
from ptrade.universe import UNIVERSE

def bn(sym):
    s = binance.load_funding(sym)
    if len(s) == 0 and sym == "PEPEUSDT":
        s = binance.load_funding("1000PEPEUSDT")
    return sym, len(s)

with ThreadPoolExecutor(4) as ex:
    for sym, n in ex.map(bn, list(UNIVERSE)):
        print("binance funding", sym, n, flush=True)

meta = hl.meta_and_ctxs()
coins = [c for c in meta.index if not meta.loc[c, "delisted"]]
def hlf(c):
    try:
        return c, len(hl.load_funding_history(c))
    except Exception as e:
        return c, f"ERR {e}"
with ThreadPoolExecutor(3) as ex:
    for c, n in ex.map(hlf, coins):
        print("hl funding", c, n, flush=True)
print("DONE")
