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
The daemon rewrites `state/last_run.json` after the sync, so a stop-hook "uncommitted changes" warning just means
running gitsync again. If tool calls are blocked, the next wake-up's gitsync commits everything pending.

## Research hygiene (lessons, keep applying)
- **Additive or not promoted**: a strategy positive IS+OOS that does not improve the equal-weight portfolio OOS
  (`research/r47_portfolio_v3.py` pattern) stays out (r37, r46/r47 BAB).
- **Realised beta**: compute it from the same hourly→daily returns as the book (`(1+r).groupby(floor('D')).prod()-1`),
  never `to_daily()` closes vs bar-open returns (one-day offset → fake β≈0, r46b).
- **Funding data**: `load_binance_funding_panel(P, splice_recon=True)`. Monthly archives end ~1 month back; without the
  splice, recent months get a flat default rate (r55).
- **Costs**: thin HIP-3/xyz names can cost 1%+ per side. Use name-level costs (`state/xyz_depth/`), not a flat 0.18%.
- Compare like-for-like baselines in the same engine and settings before calling an improvement (r48).
- Don't spoof User-Agents or work around bot blocks or geo-blocks; drop the source instead.

## Recurring checks
- Daily 00 UTC: rebalance output (trade counts per book) in the wake-up print.
- Weekly (~Fri): `python research/r42_polymarket_calib.py 600` (grow resolved outcomes); `python research/r55_live_vs_bt.py`
  (live vs backtest tracking).
- Monday 21:00 UTC: `xyz_weekend_fade` exit. Log the weekend's net P&L. Kill rule: cumulative net < 0 after 12 weekends;
  retire early if live weekends keep showing continuation (r58b).
- Discretionary: follow the rules in `state/discretionary.json` (price triggers) and `review_by`.
- Forward datasets: news (`state/news/`), Polymarket (`state/polymarket/`), options skew (`state/options/`), xyz L2 depth
  (`state/xyz_depth/`). Studies are due per `research/BACKLOG.md`.
- ~Late Nov (≈4 weeks live): portfolio review, leverage decision, portfolio-level BTC-beta cap (backlog #6; status line
  "Combined BTC exposure").
