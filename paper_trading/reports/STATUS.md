# Paper trading status — 2026-11-01 00:04 UTC

BTC 77,500 · ETH 2,254.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann.) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $99,956 | -0.04% | -0.04% | -0.04% | -0.26% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $99,992 | -0.01% | -0.01% | -0.01% | -0.15% | nan | 0.45 | +0.45 | 2 | $23 | $-5 |
| crash_rebound | $100,000 | +0.00% | +0.00% | +0.00% | +0.00% | nan | 0.00 | +0.00 | 0 | $0 | $0 |
| crash_rebound_mkt | $100,000 | +0.00% | +0.00% | +0.00% | +0.00% | nan | 0.00 | +0.00 | 0 | $0 | $0 |
| dn_funding_carry | $99,877 | -0.12% | -0.12% | -0.12% | -0.19% | nan | 1.78 | +0.00 | 12 | $184 | $23 |
| trend_breakout_majors | $99,996 | -0.00% | -0.00% | -0.00% | -0.28% | nan | 0.58 | -0.58 | 3 | $60 | $8 |
| trend_breakout_universe_lo | $99,999 | -0.00% | -0.00% | -0.00% | -0.00% | nan | 0.02 | +0.02 | 1 | $1 | $0 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | -0.02% | -0.02% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_funding | $99,566 | -0.43% | -0.43% | -0.43% | -0.43% | nan | 0.00 | +0.00 | 0 | $155 | $5 |

**Combined (ex-benchmark): $799,409 on $800,000 → -0.07%**

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 77509 | $26 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.386949 | 77535.3 | 77499.5 | $-14 | +0.300 |
| ETH | 6.66681 | 2250.06 | 2254.65 | $31 | +0.150 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| UNI/spot | 2136.3 | 7.02501 | 7.0615 | $78 | +0.151 |
| UNI | -2133.42 | 7.03009 | 7.07075 | $-87 | -0.151 |
| HYPE/spot | 171.426 | 87.5453 | 87.8868 | $59 | +0.151 |
| HYPE | -171.177 | 87.6241 | 87.9555 | $-57 | -0.151 |
| kPEPE/spot | 3.00661e+06 | 0.00499149 | 0.00496 | $-95 | +0.149 |
| kPEPE | -3.00661e+06 | 0.00498594 | 0.004956 | $90 | -0.149 |
| PENDLE/spot | 8017.1 | 1.87289 | 1.8407 | $-258 | +0.148 |
| PENDLE | -8000.21 | 1.87311 | 1.8435 | $237 | -0.148 |
| ENA | -77597.6 | 0.193295 | 0.18898 | $335 | -0.147 |
| ENA/spot | 77635.7 | 0.193307 | 0.188813 | $-349 | +0.147 |
| ZEC/spot | 11.6568 | 1287.44 | 1252.52 | $-407 | +0.146 |
| ZEC | -11.65 | 1287.49 | 1252.15 | $412 | -0.146 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ETH | -11.6722 | 2248.64 | 2254.65 | $-70 | -0.263 |
| SOL | -209.351 | 95.827 | 95.4055 | $88 | -0.200 |
| BTC | -0.145851 | 77472.1 | 77499.5 | $-4 | -0.113 |

**trend_breakout_universe_lo**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| kPEPE | 445303 | 0.0049565 | 0.004956 | $-0 | +0.022 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Record $6.3B Binance hack pushed Fear&Greed 51->30 but BTC is flat ~77.5k (+0.1% 24h) and ETH +1%: resilience under very bad news + fear spike = mild contrarian long. Reassess if BTC < 74k or if stolen-fund selling hits the tape."}`
- **crash_rebound** — Buy coins after a >10% drop over a rolling 4h window (liquidation-cascade overshoot), 15% per position, max gross 100%, hold 24h (re-trigger extends). Checked every few minutes.
  - `{"exit_at": {}, "worst_4h": {"ATOM": -0.99, "LINK": -1.99, "PENDLE": -1.12, "TRX": -1.43, "ZEC": -2.0}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"exit_at": {}, "worst_4h": {"ATOM": -0.99, "LINK": -1.99, "PENDLE": -1.12, "TRX": -1.43, "ZEC": -2.0}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 11.6, "HYPE": 20.9, "PENDLE": 14.5, "UNI": 14.2, "ZEC": 11.2, "kPEPE": 11.5}, "last_day": "2026-11-01", "top_apr": {"BTC": 10.1, "ENA": 11.6, "HYPE": 20.9, "LDO": 10.9, "PENDLE": 14.5, "RENDER": 10.4, "TAO": 10.9, "UNI": 14.2, "ZEC": 11.2, "kPEPE": 11.5}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-10-31T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily.
  - `{"last_signal": "2026-10-31T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d HL funding (quartiles), inverse-vol, gross 1.0, daily at 00 UTC.
  - `{"last_day": "2026-10-31", "paused": "PAUSED 2026-10-31: r11/r12 showed the edge is in Binance funding across 40 coins (OOS Sharpe 1.24); the HL-funding signal available live backtests at Sharpe -1.5 on 15 coins, and OKX is only ~0.5 rank-correlated with Binance. Re-test on HL funding for all 40 coins before resuming."}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-01T00:04 | trend_breakout_universe_lo | kPEPE | buy | $2,207 | 0.0049565 | signal 2026-10-31 23h |
| 2026-11-01T00:04 | trend_breakout_majors | SOL | sell | $2,773 | 95.4007 | signal 2026-10-31 23h |
| 2026-11-01T00:04 | trend_breakout_majors | BTC | sell | $3,487 | 77495.6 | signal 2026-10-31 23h |
| 2026-10-31T21:05 | trend_ewmac_majors_lo | BTC | sell | $12,542 | 77413.6 | retired: RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with |
| 2026-10-31T21:05 | trend_breakout_majors | ETH | buy | $15,786 | 2251.06 | signal 2026-10-30 23h |
| 2026-10-31T21:05 | trend_breakout_majors | SOL | buy | $10,199 | 95.4973 | signal 2026-10-30 23h |
| 2026-10-31T21:05 | trend_breakout_majors | BTC | buy | $4,671 | 77422 | signal 2026-10-30 23h |
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
