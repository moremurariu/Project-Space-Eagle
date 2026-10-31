"""Background scheduler for paper trading.

- full cycle ~20s after every hourly close (all strategies, funding, equity log, report) -> git push
- fast cycle every FAST_EVERY seconds (event-driven strategies, e.g. crash rebound)
- heartbeat in cache/daemon.json; cycles run as subprocesses so code edits take effect immediately.

Start:  nohup python -m ptrade.live.daemon > cache/daemon.log 2>&1 &
Check:  python -m ptrade.live.daemon --status
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import time

import pandas as pd

from ..config import CACHE, ROOT

HEARTBEAT = CACHE / "daemon.json"
FAST_EVERY = 180
FULL_DELAY_S = 20


def _run(args: list[str], timeout: int) -> int:
    try:
        r = subprocess.run([sys.executable, "-m", *args], cwd=ROOT, timeout=timeout, capture_output=True, text=True)
        out = (r.stdout + r.stderr).strip()
        if out:
            print(out[-3000:], flush=True)
        return r.returncode
    except subprocess.TimeoutExpired:
        print(f"timeout: {args}", flush=True)
        return -1


def status() -> dict:
    if not HEARTBEAT.exists():
        return {"alive": False, "reason": "no heartbeat"}
    hb = json.loads(HEARTBEAT.read_text())
    age = time.time() - hb.get("t", 0)
    alive = age < 120
    try:
        os.kill(hb["pid"], 0)
    except Exception:  # noqa: BLE001
        alive = False
    return {**hb, "age_s": round(age), "alive": alive}


def main():
    if "--status" in sys.argv:
        print(json.dumps(status(), indent=1))
        return
    st = status()
    if st.get("alive") and st.get("pid") != os.getpid():
        print(f"daemon already running (pid {st['pid']})")
        return
    last_full_hour = None
    last_fast = 0.0
    print(f"daemon started pid={os.getpid()}", flush=True)
    while True:
        now = pd.Timestamp.now(tz="UTC")
        hour = now.floor("h")
        if last_full_hour != hour and (now - hour).total_seconds() >= FULL_DELAY_S:
            print(f"--- full cycle {now:%Y-%m-%d %H:%M:%S}", flush=True)
            rc = _run(["ptrade.live.runner"], timeout=1500)
            last_full_hour = hour
            last_fast = time.time()
            _run(["ptrade.live.gitsync", f"paper: hourly cycle {hour:%Y-%m-%d %H:00}Z (rc={rc})"], timeout=300)
        elif time.time() - last_fast >= FAST_EVERY:
            _run(["ptrade.live.runner", "--fast"], timeout=600)
            last_fast = time.time()
        HEARTBEAT.write_text(json.dumps({"pid": os.getpid(), "t": time.time(), "last_full_hour": str(last_full_hour)}))
        time.sleep(10)


if __name__ == "__main__":
    main()
