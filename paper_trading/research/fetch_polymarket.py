"""Resumable download of resolved Polymarket binary markets + hourly YES-price history.

usage: python research/fetch_polymarket.py [min_volume] [max_markets]
Output: cache/polymarket/markets.jsonl (metadata + resolved outcome), cache/polymarket/hist/<token>.json
"""
import json
import sys
import time
from pathlib import Path

sys.path.insert(0, ".")
from ptrade.data.http import get  # noqa: E402

OUT = Path("cache/polymarket")
HIST = OUT / "hist"
HIST.mkdir(parents=True, exist_ok=True)
min_vol = float(sys.argv[1]) if len(sys.argv) > 1 else 100_000
max_n = int(sys.argv[2]) if len(sys.argv) > 2 else 4000
mfile = OUT / "markets.jsonl"
seen = set()
if mfile.exists():
    for ln in mfile.read_text().splitlines():
        seen.add(json.loads(ln)["id"])

# 1) metadata pages (closed, resolved, binary)
if len(seen) < max_n:
    offset = 0
    with open(mfile, "a") as f:
        while len(seen) < max_n:
            try:
                page = get("https://gamma-api.polymarket.com/markets", params={
                    "closed": "true", "limit": 500, "offset": offset, "order": "volumeNum", "ascending": "false",
                    "volume_num_min": min_vol}, retries=2).json()
            except Exception as e:  # noqa: BLE001  (API caps the offset)
                print("pagination stopped:", str(e)[:120], flush=True)
                break
            if not page:
                break
            for m in page:
                try:
                    outs = json.loads(m.get("outcomes") or "[]")
                    prices = [float(x) for x in json.loads(m.get("outcomePrices") or "[]")]
                    toks = json.loads(m.get("clobTokenIds") or "[]")
                except Exception:  # noqa: BLE001
                    continue
                if m["id"] in seen or len(outs) != 2 or len(toks) != 2 or not prices:
                    continue
                if max(prices) < 0.99:  # unresolved / ambiguous
                    continue
                rec = {"id": m["id"], "question": m.get("question"), "start": m.get("startDate"), "end": m.get("endDate"),
                       "closed_time": m.get("closedTime"), "volume": float(m.get("volumeNum") or 0),
                       "outcomes": outs, "yes_token": toks[0], "yes_won": prices[0] > 0.5,
                       "neg_risk": bool(m.get("negRisk")), "slug": m.get("slug")}
                f.write(json.dumps(rec) + "\n")
                seen.add(m["id"])
            offset += 500
            time.sleep(0.3)
    print("markets:", len(seen), flush=True)

# 2) price history for each YES token (hourly fidelity)
recs = [json.loads(ln) for ln in mfile.read_text().splitlines()]
done = 0
for r in recs:
    p = HIST / f"{r['yes_token'][:40]}.json"
    if p.exists():
        continue
    try:
        h = get("https://clob.polymarket.com/prices-history",
                params={"market": r["yes_token"], "interval": "max", "fidelity": 60}, retries=2).json()
        p.write_text(json.dumps(h.get("history", [])))
        done += 1
        if done % 100 == 0:
            print("hist", done, flush=True)
    except Exception as e:  # noqa: BLE001
        print("err", r["id"], e, flush=True)
    time.sleep(0.15)
print("DONE", flush=True)
