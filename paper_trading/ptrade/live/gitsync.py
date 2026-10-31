"""Commit + push paper-trading state (locked so the daemon and manual runs never collide).

    python -m ptrade.live.gitsync "message"
"""
from __future__ import annotations

import fcntl
import subprocess
import sys
import time

from ..config import CACHE, ROOT

REPO = ROOT.parent
BRANCH = "claude/sleepy-clarke-uk181s"
TRAILER = ("\n\nCo-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\n"
           "Claude-Session: https://claude.ai/code/session_01PbDDL26FytYa6u32nRqDA1")


def _git(*args, check=True):
    return subprocess.run(["git", *args], cwd=REPO, capture_output=True, text=True, check=check)


def sync(message: str, paths=("paper_trading",)) -> str:
    lock = CACHE / "git.lock"
    with open(lock, "w") as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        try:
            _git("add", "-A", *paths)
            if _git("diff", "--cached", "--quiet", check=False).returncode == 0:
                return "nothing to commit"
            _git("commit", "-m", message + TRAILER)
            for i in range(5):
                r = _git("push", "-u", "origin", BRANCH, check=False)
                if r.returncode == 0:
                    return "pushed"
                if "non-fast-forward" in r.stderr or "rejected" in r.stderr:
                    _git("pull", "--rebase", "origin", BRANCH, check=False)
                time.sleep(2 ** (i + 1))
            return "push failed: " + r.stderr[-300:]
        finally:
            fcntl.flock(f, fcntl.LOCK_UN)


if __name__ == "__main__":
    print(sync(sys.argv[1] if len(sys.argv) > 1 else "paper: state update"))
