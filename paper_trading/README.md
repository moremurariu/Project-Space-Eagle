# Paper trading lab

Self-contained, continuously running paper-trading system (independent from the rest of this repo).
Several strategies run in parallel, each in its own $100k paper book, executed against **live Hyperliquid
prices** (perps incl. HIP-3 TradFi perps; spot hedge legs priced at the HL oracle) with realistic costs:
taker fees, order-book-walk slippage, and exact hourly funding from the HL API.

**Live status:** [`reports/STATUS.md`](reports/STATUS.md) (regenerated every hour) ·
**Research log:** [`research/JOURNAL.md`](research/JOURNAL.md) · **Ideas queue:** [`research/BACKLOG.md`](research/BACKLOG.md)

## Live books

| Book | Idea | Cadence |
|---|---|---|
| `trend_breakout_majors` | Donchian breakout L/S on BTC/ETH/SOL, 25% vol target | daily |
| `trend_breakout_universe_lo` | Long-only breakout across 40 liquid coins, scaled by 30d efficiency-ratio (trendiness) gate | daily |
| `trend_ewmac_majors_lo` | **Retired** (r15: weakest, redundant with other trend books) | — |
| `xs_funding` | Market-neutral: long lowest / short highest 7d *Binance* funding (reconstructed from 1m premium archives, r21-r22) | daily |
| `xs_momentum` | Market-neutral: long winners / short losers by 7/14/21d return (lookback ensemble, staggered weekly), r38 | daily |
| `xs_bab` | ~~Experimental (r46) betting-against-beta~~ — **retired 2026-11-08** (r47: residual beta, not additive OOS) | — |
| `dn_funding_carry` | Delta-neutral basis: short perp + long spot on coins with 7d funding >15% APR | daily |
| `crash_rebound` | Buy liquidation-cascade overshoots (>10% 4h drop on closed hourly bars): 7.5% at signal + 7.5% on a further −5% within 12h (r30), each held 24h | hourly |
| `crash_rebound_mkt` | Same, only when BTC also fell >3% in those 4h (market-wide cascades) | hourly |
| `xyz_weekend_fade` | **Experimental** (r32): fade the weekend move of HL xyz 24/7 stock perps Sun 23:00 → Mon US close, dollar-neutral | weekly |
| `claude_discretionary` | Claude's own judgment from news / positioning (`state/discretionary.json`) | ad hoc |
| `benchmark_btc_hold` | BTC buy & hold benchmark | — |

## Layout
```
ptrade/
  data/         binance.vision klines + funding archives, Hyperliquid info API, sentiment (F&G, RSS)
  backtest/     unit-based (position-drift) backtester with fees, slippage, funding
  strategies/   signal library (same code drives backtests and live)
  live/         paper books, market snapshot, strategy adapters, runner, daemon, report, git sync
  universe.py   Binance symbol <-> HL coin mapping
research/       research scripts (r01..), journal, backlog
state/          committed state: books/*.json, trades.csv, equity.csv, funding.csv, discretionary.json
reports/        STATUS.md
cache/          (git-ignored) market data cache
```

## Running
```bash
cd paper_trading
pip install -r requirements.txt
python -m ptrade.live.runner          # one full cycle
python -m ptrade.live.runner --fast   # event-driven scan only
nohup python -m ptrade.live.daemon > cache/daemon.log 2>&1 &   # continuous: hourly full + 3-min fast cycles
python -m ptrade.tools_brief          # market brief
python research/r01_trend.py          # example backtest
```
Claude is woken hourly by a scheduled task and follows [`HOURLY.md`](HOURLY.md).

## Data sources (all public, no keys)
Hyperliquid info API (prices, funding, order books, HIP-3 TradFi perps) · Binance public data mirrors
(`data-api.binance.vision`, `data.binance.vision`) for long history · alternative.me Fear & Greed ·
Cointelegraph RSS.
