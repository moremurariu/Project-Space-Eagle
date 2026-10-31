"""Paced download of Hyperliquid funding history for the most liquid perps (respects HL weight limits).

usage: python research/fetch_hl_funding.py [n_coins] [start_date]
"""
import sys
import time

sys.path.insert(0, ".")
import pandas as pd  # noqa: E402

from ptrade.config import CACHE  # noqa: E402
from ptrade.data import hyperliquid as hl  # noqa: E402

n = int(sys.argv[1]) if len(sys.argv) > 1 else 50
start = pd.Timestamp(sys.argv[2] if len(sys.argv) > 2 else "2024-01-01", tz="UTC")
meta = hl.meta_and_ctxs()
meta = meta[~meta.delisted].sort_values("day_ntl_vlm", ascending=False)
from ptrade.universe import UNIVERSE  # noqa: E402
coins = list(dict.fromkeys(list(UNIVERSE.values()) + list(meta.index[:n])))
for c in coins:
    path = CACHE / f"hl_funding_{c}.parquet"
    df = pd.read_parquet(path) if path.exists() else pd.DataFrame(columns=["funding", "premium"])
    cur = int((df.index[-1] if len(df) else start).timestamp() * 1000) + 1
    end = int(time.time() * 1000)
    parts = [df] if len(df) else []
    while cur < end:
        try:
            new = hl.funding_history(c, cur, min(end, cur + 499 * 3_600_000))
        except Exception as e:  # noqa: BLE001
            print("err", c, e, flush=True)
            time.sleep(20)
            continue
        if len(new) == 0:
            cur += 499 * 3_600_000
            continue
        parts.append(new)
        cur = int(new.index[-1].timestamp() * 1000) + 3_600_000
        time.sleep(2.2)
    if parts:
        out = pd.concat(parts)
        out = out[~out.index.duplicated(keep="last")].sort_index()
        out.to_parquet(path)
        print("hl funding", c, len(out), out.index[0], flush=True)
print("DONE", flush=True)
