"""One-shot hourly wake-up routine (steps 1-3 of HOURLY.md).

    python -m ptrade.live.wakeup

- sync the branch (stash local state changes, pull --rebase, pop)
- if the daemon is dead: run a catch-up full cycle (unless this hour's full cycle already ran), restart it
- restart the HL funding fetcher if it isn't running and the cache is incomplete
- print a compact status: equity table, errors, crash-scan extremes, open crash positions
"""
from __future__ import annotations

import json
import subprocess
import sys

import pandas as pd

from ..config import CACHE, ROOT, STATE
from .daemon import status as daemon_status

REPO = ROOT.parent
BRANCH = "claude/sleepy-clarke-uk181s"


def sh(cmd: str, cwd=REPO, timeout=900) -> str:
    r = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True, timeout=timeout)
    return (r.stdout + r.stderr).strip()


def _proc_running(fragment: str) -> bool:
    out = sh("ps -eo pid,args")
    return any(fragment in ln and "python" in ln and "ps -eo" not in ln for ln in out.splitlines())


def main():
    print("sync:", sh(f"git stash -q; git pull -q --rebase origin {BRANCH}; git stash pop -q; git status -sb | head -1"))
    st = daemon_status()
    if not st.get("alive"):
        lr = {}
        try:
            lr = json.loads((STATE / "last_run.json").read_text())
        except Exception:  # noqa: BLE001
            pass
        hour_now = pd.Timestamp.now(tz="UTC").floor("h")
        done = lr.get("kind") == "full" and pd.Timestamp(lr["ts"]).floor("h") == hour_now
        if not done:
            print("daemon dead -> catch-up full cycle")
            out = sh(f"{sys.executable} -m ptrade.live.runner", cwd=ROOT, timeout=1500)
            print("\n".join(ln for ln in out.splitlines() if ln.startswith("[") or "ERROR" in ln)[-2000:])
        sh(f"nohup {sys.executable} -m ptrade.live.daemon >> cache/daemon.log 2>&1 &", cwd=ROOT)
        print("daemon restarted")
    else:
        print(f"daemon alive (pid {st['pid']}, last full {st.get('last_full_hour')})")
    n_cached = len(list(CACHE.glob("hl_funding_*.parquet")))
    if not _proc_running("fetch_hl_funding") and n_cached < 60:
        sh(f"nohup {sys.executable} research/fetch_hl_funding.py 45 >> cache/fetch_hl_funding.log 2>&1 &", cwd=ROOT)
        print(f"funding fetcher restarted ({n_cached} cached)")
    # status
    rep = (ROOT / "reports" / "STATUS.md").read_text().splitlines()
    print("\n".join(rep[:2 + 20]))
    errs = {p.stem: json.loads(p.read_text())["meta"].get("last_error") for p in (STATE / "books").glob("*.json")}
    errs = {k: v for k, v in errs.items() if v}
    print("errors:", errs or "none")
    for name in ("crash_rebound", "crash_rebound_mkt"):
        p = STATE / "books" / f"{name}.json"
        if p.exists():
            b = json.loads(p.read_text())
            print(f"{name}: worst4h={b['meta'].get('worst_4h')} open={list(b['positions'])} tranches={b['meta'].get('tranches')} watch={list(b['meta'].get('watch', {}))}")


if __name__ == "__main__":
    main()
