# Paper trading status — 2026-10-31 19:04 UTC

BTC 77,628 · ETH 2,255.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann.) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $100,089 | +0.09% | +0.09% | +0.09% | -0.12% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $100,047 | +0.05% | +0.05% | +0.05% | -0.02% | nan | 0.45 | +0.45 | 2 | $23 | $-2 |
| crash_rebound | $100,000 | +0.00% | +0.00% | +0.00% | +0.00% | nan | 0.00 | +0.00 | 0 | $0 | $0 |
| crash_rebound_mkt | $100,000 | +0.00% | +0.00% | +0.00% | +0.00% | nan | 0.00 | +0.00 | 0 | $0 | $0 |
| dn_funding_carry | $99,844 | -0.16% | -0.16% | -0.16% | -0.19% | nan | 1.80 | +0.00 | 12 | $184 | $13 |
| trend_breakout_majors | $99,803 | -0.20% | -0.20% | -0.20% | -0.21% | nan | 0.82 | -0.82 | 3 | $41 | $4 |
| trend_breakout_universe_lo | $100,000 | +0.00% | +0.00% | +0.00% | +0.00% | nan | 0.00 | +0.00 | 0 | $0 | $0 |
| trend_ewmac_majors_lo | $100,020 | +0.02% | +0.02% | +0.02% | -0.01% | nan | 0.13 | +0.13 | 1 | $6 | $-1 |
| xs_funding | $99,566 | -0.43% | -0.43% | -0.43% | -0.43% | nan | 0.00 | +0.00 | 0 | $155 | $5 |

**Combined (ex-benchmark): $799,281 on $800,000 → -0.09%**

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 77612 | $159 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.386949 | 77535.3 | 77628.5 | $36 | +0.300 |
| ETH | 6.66681 | 2250.06 | 2255.15 | $34 | +0.150 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| UNI/spot | 2136.3 | 7.02501 | 7.1088 | $179 | +0.152 |
| UNI | -2133.42 | 7.03009 | 7.11775 | $-187 | -0.152 |
| kPEPE/spot | 3.00661e+06 | 0.00499149 | 0.005024 | $98 | +0.151 |
| kPEPE | -3.00661e+06 | 0.00498594 | 0.005021 | $-105 | -0.151 |
| HYPE/spot | 171.426 | 87.5453 | 88.0445 | $86 | +0.151 |
| HYPE | -171.177 | 87.6241 | 88.1425 | $-89 | -0.151 |
| PENDLE/spot | 8017.1 | 1.87289 | 1.8645 | $-67 | +0.150 |
| PENDLE | -8000.21 | 1.87311 | 1.86735 | $46 | -0.150 |
| ZEC | -11.65 | 1287.49 | 1281.15 | $74 | -0.149 |
| ZEC/spot | 11.6568 | 1287.44 | 1280.4 | $-82 | +0.149 |
| ENA | -77597.6 | 0.193295 | 0.191345 | $151 | -0.149 |
| ENA/spot | 77635.7 | 0.193307 | 0.191139 | $-168 | +0.149 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ETH | -18.6848 | 2248.64 | 2255.15 | $-122 | -0.422 |
| SOL | -287.086 | 95.8957 | 95.9505 | $-16 | -0.276 |
| BTC | -0.16119 | 77461.6 | 77628.5 | $-27 | -0.125 |

**trend_ewmac_majors_lo**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.162018 | 77469.4 | 77628.5 | $26 | +0.126 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Record $6.3B Binance hack pushed Fear&Greed 51->30 but BTC is flat ~77.5k (+0.1% 24h) and ETH +1%: resilience under very bad news + fear spike = mild contrarian long. Reassess if BTC < 74k or if stolen-fund selling hits the tape."}`
- **crash_rebound** — Buy coins after a >10% drop over a rolling 4h window (liquidation-cascade overshoot), 15% per position, max gross 100%, hold 24h (re-trigger extends). Checked every few minutes.
  - `{"exit_at": {}, "worst_4h": {"APT": -0.33, "CRV": -0.73, "DOT": -0.21, "TRX": -2.25, "XLM": -0.24}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"exit_at": {}, "worst_4h": {"APT": -0.33, "CRV": -0.73, "DOT": -0.21, "TRX": -2.25, "XLM": -0.24}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 11.6, "HYPE": 19.7, "PENDLE": 14.0, "UNI": 12.5, "ZEC": 11.2, "kPEPE": 11.7}, "last_day": "2026-10-31", "top_apr": {"CRV": 10.4, "ENA": 11.6, "HYPE": 19.7, "LDO": 10.9, "PENDLE": 14.0, "RENDER": 10.2, "TAO": 10.9, "UNI": 12.5, "ZEC": 11.2, "kPEPE": 11.7}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 40% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — Long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d HL funding (quartiles), inverse-vol, gross 1.0, daily at 00 UTC.
  - `{"last_day": "2026-10-31", "paused": "PAUSED 2026-10-31: r11/r12 showed the edge is in Binance funding across 40 coins (OOS Sharpe 1.24); the HL-funding signal available live backtests at Sharpe -1.5 on 15 coins, and OKX is only ~0.5 rank-correlated with Binance. Re-test on HL funding for all 40 coins before resuming."}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-10-31T17:41 | xs_funding | BTC | buy | $11,758 | 77605.4 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | XRP | buy | $7,403 | 1.25481 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | TAO | buy | $4,697 | 238.377 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | UNI | buy | $4,312 | 7.06181 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | PENDLE | buy | $3,991 | 1.87212 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | CRV | buy | $3,875 | 0.285315 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | LDO | buy | $3,773 | 0.336258 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ZEC | buy | $3,704 | 1285.91 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | kPEPE | buy | $3,340 | 0.00499436 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ENA | buy | $3,142 | 0.191605 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | TIA | sell | $2,500 | 0.480264 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | WLD | sell | $3,076 | 0.398489 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | OP | sell | $3,443 | 0.112945 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | JUP | sell | $3,873 | 0.258715 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ALGO | sell | $4,037 | 0.0955458 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ATOM | sell | $4,257 | 1.51051 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ADA | sell | $4,883 | 0.193475 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | ETC | sell | $6,055 | 7.11159 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | TRX | sell | $8,522 | 0.308026 | paused: live funding signal failed validation |
| 2026-10-31T17:41 | xs_funding | BNB | sell | $9,030 | 705.414 | paused: live funding signal failed validation |
