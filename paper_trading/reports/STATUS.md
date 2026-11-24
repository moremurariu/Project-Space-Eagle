# Paper trading status — 2026-11-24 23:04 UTC

BTC 59,970 · ETH 1,683.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $77,357 | -22.64% | -2.08% | -7.54% | -30.99% | -6.24 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,724 | -3.28% | -0.10% | -0.39% | -4.87% | -5.27 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.45 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.03 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,133 | +0.13% | +0.01% | +0.09% | -0.26% | 2.22 | 0.83 | +0.00 | 6 | $302 | $412 |
| trend_breakout_majors | $118,056 | +18.06% | +1.34% | +4.00% | -7.51% | 6.46 | 0.52 | -0.52 | 3 | $128 | $180 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.87 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.01 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.65 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,001 | +1.00% | +0.25% | +0.85% | -1.64% | 1.71 | 0.94 | +0.03 | 20 | $683 | $330 |
| xs_momentum | $98,332 | -1.67% | -0.26% | -0.57% | -3.13% | -2.50 | 0.84 | -0.00 | 35 | $247 | $56 |
| xyz_weekend_fade | $98,477 | -1.52% | +0.00% | -4.66% | -5.64% | -0.93 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,112,831 on $1,100,000 → +1.17%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.96 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60005.9 | $-22,573 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 59970.5 | $-1,208 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.2395 | $-5,678 | +0.146 |
| PENDLE | -11765.6 | 1.72106 | 1.23745 | $5,690 | -0.145 |
| HYPE | -233.128 | 82.7772 | 60.316 | $5,236 | -0.140 |
| HYPE/spot | 233.128 | 83.2675 | 60.313 | $-5,351 | +0.140 |
| ZEC/spot | 17.43 | 1175.27 | 743.39 | $-7,528 | +0.129 |
| ZEC | -17.4301 | 1175 | 743.235 | $7,526 | -0.129 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 59970.5 | $4,741 | -0.202 |
| ETH | -11.3433 | 2117.81 | 1683.45 | $4,927 | -0.162 |
| SOL | -265.603 | 89.0415 | 69.9575 | $5,069 | -0.157 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.308355 | $-22 | +0.087 |
| BTC | -0.144359 | 68333.1 | 59970.5 | $1,207 | -0.086 |
| LINK | -662.607 | 8.53769 | 8.86165 | $-215 | -0.058 |
| ATOM | 4628.98 | 1.28373 | 1.2517 | $-148 | +0.057 |
| BCH | 27.9335 | 199.151 | 197.385 | $-49 | +0.055 |
| SEI | 153686 | 0.0369478 | 0.032719 | $-650 | +0.050 |
| APT | 12637.8 | 0.436568 | 0.3651 | $-903 | +0.046 |
| TAO | -25.0244 | 220.686 | 181.375 | $984 | -0.045 |
| RENDER | -3551.19 | 1.49617 | 1.274 | $789 | -0.045 |
| AAVE | -44.8727 | 112.188 | 94.3475 | $801 | -0.042 |
| TIA | 13095.4 | 0.343495 | 0.32219 | $-279 | +0.042 |
| JUP | 18056.4 | 0.241363 | 0.223115 | $-329 | +0.040 |
| PENDLE | -3236.78 | 1.54767 | 1.23745 | $1,004 | -0.040 |
| OP | 58569.1 | 0.0891484 | 0.06832 | $-1,220 | +0.040 |
| NEAR | -1371.79 | 3.51678 | 2.8864 | $865 | -0.039 |
| UNI | -836.22 | 5.16466 | 4.73095 | $363 | -0.039 |
| WLD | 15387.1 | 0.31305 | 0.23577 | $-1,189 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.098271 | $1,150 | -0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003483 | $-365 | +0.031 |
| ZEC | -3.83105 | 862.252 | 743.235 | $456 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.308355 | $-157 | +0.104 |
| BTC | -0.122056 | 65912.7 | 59970.5 | $725 | -0.074 |
| BNB | -9.13414 | 580.021 | 549.695 | $277 | -0.051 |
| ATOM | 3503.24 | 1.28664 | 1.2517 | $-122 | +0.045 |
| RENDER | 3386.49 | 1.53278 | 1.274 | $-876 | +0.044 |
| FIL | -7861.93 | 0.620713 | 0.53617 | $665 | -0.043 |
| ETH | -2.08109 | 1804.39 | 1683.45 | $252 | -0.036 |
| JUP | 15235.8 | 0.263176 | 0.223115 | $-610 | +0.035 |
| CRV | -19201.4 | 0.232521 | 0.17452 | $1,114 | -0.034 |
| APT | -7937.59 | 0.43262 | 0.3651 | $536 | -0.029 |
| AVAX | 424.946 | 7.89216 | 6.5891 | $-554 | +0.028 |
| BCH | -14.0941 | 212.452 | 197.385 | $212 | -0.028 |
| LTC | -67.4945 | 40.3115 | 40.9495 | $-43 | -0.028 |
| INJ | 450.546 | 7.33527 | 5.84665 | $-671 | +0.027 |
| NEAR | 890.083 | 3.11366 | 2.8864 | $-202 | +0.026 |
| TAO | 12.9296 | 197.549 | 181.375 | $-209 | +0.024 |
| ENA | -21996.7 | 0.108918 | 0.098271 | $234 | -0.022 |
| AAVE | 21.5968 | 108.591 | 94.3475 | $-308 | +0.021 |
| HBAR | -38109.1 | 0.0598269 | 0.050632 | $350 | -0.020 |
| TIA | 5612.94 | 0.336231 | 0.32219 | $-79 | +0.018 |
| WLD | -5872.05 | 0.34434 | 0.23577 | $638 | -0.014 |
| DOGE | -21725 | 0.0643319 | 0.055382 | $194 | -0.012 |
| XLM | 9051.42 | 0.140675 | 0.124595 | $-146 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.23745 | $-27 | +0.009 |
| DOT | -1201.93 | 0.841601 | 0.65298 | $227 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.032719 | $-126 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003483 | $-236 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.06832 | $15 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.73095 | $-227 | +0.005 |
| SOL | -6.37858 | 91.9832 | 69.9575 | $140 | -0.005 |
| LDO | -1790.74 | 0.233401 | 0.22812 | $9 | -0.004 |
| ARB | -3323.04 | 0.1124 | 0.11015 | $7 | -0.004 |
| ZEC | 0.47487 | 796.75 | 743.235 | $-25 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.27785 | $-76 | +0.002 |
| XRP | -174.541 | 1.02442 | 0.926725 | $17 | -0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -1.5, "APT": -1.37, "LDO": -1.64, "SEI": -1.86, "TIA": -1.41}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -1.5, "APT": -1.37, "LDO": -1.64, "SEI": -1.86, "TIA": -1.41}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.4, "PENDLE": 9.6, "ZEC": 10.4}, "last_day": "2026-11-24", "top_apr": {"BTC": 5.2, "DOGE": 4.1, "ENA": 9.0, "ETC": 7.3, "HYPE": 8.4, "LDO": 8.3, "LINK": 6.3, "PENDLE": 9.6, "RENDER": 8.4, "ZEC": 10.4}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-24"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-22 23:04: 36 trades, 38 names, weekend xs-dispersion 2.05%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-24T00:04 | xs_momentum | ETH | buy | $1,378 | 1705.04 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | DOGE | buy | $1,267 | 0.0562481 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | XRP | buy | $1,193 | 0.938082 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | SOL | buy | $1,053 | 71.8201 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | INJ | sell | $988 | 6.29079 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | AAVE | sell | $1,038 | 96.7782 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | ARB | sell | $1,398 | 0.1124 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_funding | ATOM | buy | $5,942 | 1.28373 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | SEI | buy | $1,020 | 0.0338899 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | TIA | buy | $1,014 | 0.335205 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | ADA | sell | $5,816 | 0.148126 | daily xs-funding (Binance recon) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SOXL | buy | $11,404 | 43.1142 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NBIS | buy | $5,534 | 98.2613 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NVDA | buy | $4,544 | 164.588 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:MSTR | buy | $3,438 | 84.756 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CBRS | buy | $3,199 | 98.9671 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHY | buy | $2,953 | 124.321 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CRCL | buy | $2,652 | 51.6426 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHX | buy | $2,516 | 876.234 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:IREN | buy | $2,450 | 14.7573 | weekend fade exit (Mon US close) |
