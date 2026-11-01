import sys, time; sys.path.insert(0, ".")
from concurrent.futures import ThreadPoolExecutor
from ptrade.data import funding_recon
from ptrade.universe import UNIVERSE
syms = [s if s != "PEPEUSDT" else "1000PEPEUSDT" for s in UNIVERSE]
t0 = time.time()
def one(s):
    r = funding_recon.build(s, start="2024-01", workers=4)
    return s, len(r)
with ThreadPoolExecutor(3) as ex:
    for s, n in ex.map(one, syms):
        print(s, n, f"{time.time()-t0:.0f}s", flush=True)
print("DONE", flush=True)
