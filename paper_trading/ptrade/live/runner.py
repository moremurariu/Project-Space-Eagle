"""One paper-trading cycle.

    python -m ptrade.live.runner            # full cycle (all strategies, equity log, report)
    python -m ptrade.live.runner --fast     # fast cycle (event-driven strategies only)
"""
from __future__ import annotations

import argparse
import fcntl
import json
import sys
import traceback
from contextlib import contextmanager

import pandas as pd

from ..config import CACHE, STATE
from ..research_utils import load_panel
from .book import coin_of, is_spot
from .market import Market
from .report import write_report
from .strategies import all_strategies

LOCK = CACHE / "cycle.lock"
LAST_RUN = STATE / "last_run.json"


@contextmanager
def cycle_lock():
    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with open(LOCK, "w") as f:
        fcntl.flock(f, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(f, fcntl.LOCK_UN)


class DataCtx:
    def __init__(self, need_panel: bool):
        self.panel = load_panel(refresh=True) if need_panel else None


def accrue(book, mkt):
    for key in list(book.positions):
        if is_spot(key):
            continue
        coin = coin_of(key)
        pos = book.positions[key]
        rates = mkt.funding_since(coin, pd.Timestamp(pos["funded_through"]))
        try:
            px = mkt.oracle(coin)
        except KeyError:
            px = pos["avg_px"]
        book.accrue_funding(key, rates, px)


def run_cycle(fast: bool = False, only: list[str] | None = None) -> dict:
    summary = {"fast": fast, "errors": {}}
    with cycle_lock():
        now = pd.Timestamp.now(tz="UTC")
        summary["ts"] = now.isoformat()
        mkt = Market()
        strats = [s for s in all_strategies() if (s.fast or not fast) and (not only or s.name in only)]
        data = DataCtx(need_panel=not fast)
        for s in strats:
            book = s.book(now)
            try:
                accrue(book, mkt)
                s.run(book, mkt, data, now, fast)
                book.meta.pop("last_error", None)
            except Exception as e:  # noqa: BLE001
                tb = traceback.format_exc()
                print(f"[{s.name}] ERROR {e}\n{tb}", file=sys.stderr)
                book.meta["last_error"] = f"{now.isoformat()} {e}"
                summary["errors"][s.name] = str(e)
            if not fast:
                prices = mkt.prices(book.positions)
                book.log_equity(prices, now)
            book.save()
        if not fast:
            write_report(mkt, now)
            try:  # forward data collection for prediction-market research
                from ..data import polymarket
                summary["polymarket_rows"] = polymarket.collect(now)
            except Exception as e:  # noqa: BLE001
                summary["errors"]["polymarket_collect"] = str(e)[:200]
            try:  # forward news dataset
                from ..data import sentiment
                summary["news_rows"] = sentiment.collect_headlines(now)
            except Exception as e:  # noqa: BLE001
                summary["errors"]["news_collect"] = str(e)[:200]
            try:  # forward option-skew dataset (Deribit RR25 / ATM IV / put-call ratios)
                from ..data import deribit
                summary["skew_rows"] = deribit.collect_skew(now)
            except Exception as e:  # noqa: BLE001
                summary["errors"]["skew_collect"] = str(e)[:200]
            try:  # forward xyz order-book depth dataset (name-level execution costs for the weekend fade)
                from ..data import hyperliquid as _hl
                summary["xyz_depth_rows"] = _hl.collect_xyz_depth(now)
            except Exception as e:  # noqa: BLE001
                summary["errors"]["xyz_depth"] = str(e)[:200]
        LAST_RUN.write_text(json.dumps({**summary, "kind": "fast" if fast else "full"}, indent=1))
    return summary


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fast", action="store_true")
    ap.add_argument("--only", nargs="*")
    a = ap.parse_args()
    print(json.dumps(run_cycle(a.fast, a.only), indent=1))


if __name__ == "__main__":
    main()
