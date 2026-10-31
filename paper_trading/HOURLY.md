# Hourly wake-up runbook

Claude is woken every hour by a scheduled task. Each wake-up, do the following (keep the turn short unless
there is real research to do).

## 1. Make sure the code/state is current (container may be fresh)
```bash
cd /home/user/Project-Space-Eagle
git checkout claude/sleepy-clarke-uk181s 2>/dev/null; git pull --rebase origin claude/sleepy-clarke-uk181s
cd paper_trading
python -c "import pyarrow, scipy, lightgbm" 2>/dev/null || pip install -q -r requirements.txt
```

## 2. Make sure the daemon is alive (it runs the hourly full cycle + 3-minute crash scans + git push)
```bash
python -m ptrade.live.daemon --status
# if "alive": false  ->
python -m ptrade.live.runner            # catch-up full cycle now
nohup python -m ptrade.live.daemon > cache/daemon.log 2>&1 &
```
(If the HL funding cache is empty in a fresh container nothing breaks; live strategies fetch what they need.)

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
