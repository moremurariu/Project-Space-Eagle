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
| `trend_breakout_majors` | Donchian breakout L/S on BTC/ETH/SOL, vol-targeted | daily |
| `trend_breakout_universe_lo` | Long-only breakout across 40 liquid coins | daily |
| `trend_ewmac_majors_lo` | Long-only multi-speed EWMA crossover on majors | daily |
| `xs_funding` | Market-neutral: long lowest-funding / short highest-funding perps | daily |
| `dn_funding_carry` | Delta-neutral basis: short perp + long spot on high-funding coins | daily |
| `crash_rebound` | Buy liquidation-cascade overshoots (>10% 4h drop), hold 24h | every 3 min |
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
