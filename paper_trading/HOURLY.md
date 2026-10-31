# Hourly wake-up runbook

Claude is woken every hour by a scheduled task. Each wake-up, do the following (keep the turn short unless
there is real research to do).

## 1-2. Sync, revive, catch up (one command)
```bash
cd /home/user/Project-Space-Eagle/paper_trading && python -m ptrade.live.wakeup
```
It pulls the branch, runs a catch-up full cycle if the daemon died (container sleeps between wake-ups),
restarts the daemon and the HL funding fetcher, and prints the status table, errors and crash-scan state.
(Fresh container: `pip install -q -r requirements.txt` first if imports fail.)

## 3. Review
- `head -30 reports/STATUS.md` — equity, drawdowns, exposures.
- `cat state/last_run.json` and grep books for `last_error`.
- `tail -20 cache/daemon.log` if the daemon was running.
- Sanity: gross exposure within limits, no runaway positions, no repeated errors.

## 4. Discretionary book (optional, when something is worth a view)
- `python -m ptrade.tools_brief` — prices, movers, funding extremes, TradFi perps, Fear&Greed, headlines.
- Edit `state/discretionary.json` (targets = weight of equity per HL key, e.g. `"BTC": 0.3`, `"xyz:GOLD": -0.2`;
  bump `updated` so the runner applies it). Always write the rationale.

## 5. Research (time-boxed, roughly every few hours or when idle)
- Pick the top item in `research/BACKLOG.md`, test it with the backtester (`ptrade/backtest/engine.py`,
  helpers in `ptrade/research_utils.py`), log results in `research/JOURNAL.md`.
- Promote a strategy to live only if it is positive in-sample (2019-2023) AND out-of-sample (2024+) after
  costs, with a sensible economic rationale. Add it in `ptrade/live/strategies.py::all_strategies()`.
- Demote/kill live strategies that badly underperform their backtest expectations over a meaningful window.

## 6. Persist
```bash
python -m ptrade.live.gitsync "hourly review: <one-line summary>"
```
