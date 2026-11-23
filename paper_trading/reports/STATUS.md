# Paper trading status — 2026-11-23 19:04 UTC

BTC 61,670 · ETH 1,747.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $79,526 | -20.47% | +1.50% | -5.04% | -30.99% | -5.79 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,855 | -3.14% | +0.07% | -0.26% | -4.87% | -5.18 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.10 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,124 | +0.12% | +0.02% | +0.11% | -0.26% | 2.23 | 0.91 | +0.00 | 6 | $302 | $402 |
| trend_breakout_majors | $115,692 | +15.69% | -1.35% | +1.20% | -7.51% | 5.90 | 0.55 | -0.55 | 3 | $128 | $182 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.94 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.08 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.82 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,598 | +0.60% | +0.38% | -0.26% | -1.64% | 1.19 | 0.96 | +0.00 | 20 | $663 | $301 |
| xs_momentum | $98,604 | -1.40% | +0.20% | -0.18% | -3.13% | -2.16 | 0.95 | -0.01 | 35 | $243 | $52 |
| xyz_weekend_fade | $98,694 | -1.31% | -4.45% | -4.52% | -5.64% | -0.80 | 1.05 | +0.03 | 36 | $907 | $122 |

**Combined (ex-benchmark): $1,110,674 on $1,100,000 → +0.97%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.95 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 61686.3 | $-20,403 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 61669.5 | $-1,077 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.3364 | $-4,539 | +0.157 |
| PENDLE | -11765.6 | 1.72106 | 1.3343 | $4,550 | -0.157 |
| HYPE | -233.128 | 82.7772 | 65.4305 | $4,044 | -0.152 |
| HYPE/spot | 233.128 | 83.2675 | 65.4185 | $-4,161 | +0.152 |
| ZEC/spot | 17.43 | 1175.27 | 823.472 | $-6,132 | +0.143 |
| ZEC | -17.4301 | 1175 | 823.095 | $6,134 | -0.143 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 61669.5 | $4,065 | -0.212 |
| ETH | -11.3433 | 2117.81 | 1747.45 | $4,201 | -0.171 |
| SOL | -265.603 | 89.0415 | 73.5865 | $4,105 | -0.169 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 61669.5 | $962 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.310935 | $52 | +0.088 |
| LINK | -662.607 | 8.53769 | 9.09285 | $-368 | -0.060 |
| ADA | 39260.6 | 0.144249 | 0.150515 | $246 | +0.059 |
| BCH | 27.9335 | 199.151 | 202.695 | $99 | +0.056 |
| APT | 12637.8 | 0.436568 | 0.3906 | $-581 | +0.049 |
| TAO | -25.0244 | 220.686 | 194.805 | $648 | -0.048 |
| RENDER | -3551.19 | 1.49617 | 1.3252 | $607 | -0.047 |
| AAVE | -44.8727 | 112.188 | 99.684 | $561 | -0.044 |
| JUP | 18056.4 | 0.241363 | 0.242615 | $23 | +0.044 |
| SEI | 123581 | 0.0376927 | 0.035284 | $-298 | +0.043 |
| PENDLE | -3236.78 | 1.54767 | 1.3343 | $691 | -0.043 |
| NEAR | -1371.79 | 3.51678 | 3.08665 | $590 | -0.042 |
| OP | 58569.1 | 0.0891484 | 0.07145 | $-1,037 | +0.042 |
| UNI | -836.22 | 5.16466 | 4.9479 | $181 | -0.041 |
| WLD | 15387.1 | 0.31305 | 0.24333 | $-1,073 | +0.037 |
| ENA | -33871.5 | 0.132226 | 0.1038 | $963 | -0.035 |
| TIA | 10070 | 0.345986 | 0.3397 | $-63 | +0.034 |
| ZEC | -3.83105 | 862.252 | 823.095 | $150 | -0.031 |
| kPEPE | 893923 | 0.0038915 | 0.003521 | $-331 | +0.031 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.310935 | $-71 | +0.105 |
| BTC | -0.122056 | 65912.7 | 61669.5 | $518 | -0.076 |
| BNB | -9.13414 | 580.021 | 561.925 | $165 | -0.052 |
| ETH | -2.88908 | 1804.39 | 1747.45 | $165 | -0.051 |
| RENDER | 3386.49 | 1.53278 | 1.3252 | $-703 | +0.046 |
| ATOM | 3503.24 | 1.28664 | 1.2699 | $-59 | +0.045 |
| FIL | -7861.93 | 0.620713 | 0.56572 | $432 | -0.045 |
| INJ | 607.666 | 7.33527 | 6.66335 | $-408 | +0.041 |
| JUP | 15235.8 | 0.263176 | 0.242615 | $-313 | +0.037 |
| CRV | -19201.4 | 0.232521 | 0.18538 | $905 | -0.036 |
| AAVE | 32.3234 | 108.591 | 99.684 | $-288 | +0.033 |
| APT | -7937.59 | 0.43262 | 0.3906 | $334 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.8965 | $-423 | +0.030 |
| BCH | -14.0941 | 212.452 | 202.695 | $138 | -0.029 |
| LTC | -67.4945 | 40.3115 | 41.9075 | $-108 | -0.029 |
| NEAR | 890.083 | 3.11366 | 3.08665 | $-24 | +0.028 |
| DOGE | -44257.5 | 0.0643319 | 0.057406 | $307 | -0.026 |
| TAO | 12.9296 | 197.549 | 194.805 | $-35 | +0.026 |
| ENA | -21996.7 | 0.108918 | 0.1038 | $113 | -0.023 |
| HBAR | -38109.1 | 0.0598269 | 0.053487 | $242 | -0.021 |
| TIA | 5612.94 | 0.336231 | 0.3397 | $19 | +0.019 |
| SOL | -21.0355 | 91.9832 | 73.5865 | $387 | -0.016 |
| WLD | -5872.05 | 0.34434 | 0.24333 | $593 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.953465 | $103 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.131625 | $-82 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11444 | $-343 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.3343 | $45 | +0.010 |
| DOT | -1201.93 | 0.841601 | 0.69289 | $179 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.035284 | $-66 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003521 | $-230 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07145 | $-6 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.9479 | $-206 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.24448 | $-20 | -0.004 |
| ZEC | 0.47487 | 796.75 | 823.095 | $13 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.294535 | $-65 | +0.002 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:SOXL | -264.497 | 33.7478 | 43.0085 | $-2,449 | -0.115 |
| xyz:COST | 5.37885 | 1103.42 | 1082.25 | $-114 | +0.059 |
| xyz:NBIS | -56.3175 | 86.8826 | 98.512 | $-655 | -0.056 |
| xyz:SP500 | 0.655074 | 7002.81 | 7255.95 | $166 | +0.048 |
| xyz:UNITREE | 42.2589 | 90.2361 | 108.43 | $769 | +0.046 |
| xyz:NVDA | -27.6065 | 133.548 | 165.025 | $-869 | -0.046 |
| xyz:STRC | 43.9632 | 95.0508 | 97.8975 | $125 | +0.044 |
| xyz:ANT | 46.3349 | 89.5172 | 87.899 | $-75 | +0.041 |
| xyz:AAPL | 11.2936 | 348.333 | 349.525 | $13 | +0.040 |
| xyz:META | 6.33532 | 571.237 | 599.815 | $181 | +0.039 |
| xyz:XYZ100 | 0.138214 | 25535.2 | 26792.5 | $174 | +0.038 |
| xyz:IBM | 15.4686 | 227.775 | 234.115 | $98 | +0.037 |
| xyz:MSTR | -40.5579 | 82.455 | 85.291 | $-115 | -0.035 |
| xyz:CBRS | -32.3279 | 86.9154 | 98.045 | $-360 | -0.032 |
| xyz:SKHY | -23.7567 | 109.14 | 124.405 | $-363 | -0.030 |
| xyz:AMZN | 11.2909 | 252.591 | 258.365 | $65 | +0.030 |
| xyz:SPCX | 20.186 | 130.902 | 135.215 | $87 | +0.028 |
| xyz:MSFT | 5.37955 | 495.48 | 497.01 | $8 | +0.027 |
| xyz:CRCL | -51.3591 | 50.1159 | 51.552 | $-74 | -0.027 |
| xyz:BABA | 30.2312 | 83.0533 | 86.619 | $108 | +0.027 |
| xyz:SKHX | -2.87179 | 779.493 | 878.73 | $-285 | -0.026 |
| xyz:IREN | -166.006 | 12.8106 | 14.7365 | $-320 | -0.025 |
| xyz:ZHIPU | -45.2724 | 38.8205 | 46.551 | $-350 | -0.021 |
| xyz:PURRDAT | -209.653 | 7.87569 | 8.37085 | $-104 | -0.018 |
| xyz:SNDK | -18.9852 | 83.0642 | 89.972 | $-131 | -0.017 |
| xyz:HOOD | -24.3395 | 65.1215 | 67.755 | $-64 | -0.017 |
| xyz:GOOGL | 5.77104 | 273.323 | 283.91 | $61 | +0.017 |
| xyz:INTC | -19.5296 | 53.6475 | 58.4025 | $-93 | -0.012 |
| xyz:AMD | -2.18996 | 442.79 | 454.6 | $-26 | -0.010 |
| xyz:TSLA | 2.4212 | 357.605 | 387.805 | $73 | +0.010 |
| xyz:MU | 1.06539 | 615.696 | 710.5 | $101 | +0.008 |
| xyz:MRVL | -4.33833 | 127.25 | 144.04 | $-73 | -0.006 |
| xyz:EWY | 4.12068 | 127.505 | 136.025 | $35 | +0.006 |
| xyz:DRAM | -13.9588 | 35.0429 | 39.703 | $-65 | -0.006 |
| xyz:CXMT | -83.1059 | 5.0613 | 5.7028 | $-53 | -0.005 |
| xyz:ORCL | -3.21642 | 104.62 | 113.35 | $-28 | -0.004 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -6.51, "SUI": -4.42, "WLD": -4.8, "ZEC": -5.17, "kPEPE": -5.15}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -6.51, "SUI": -4.42, "WLD": -4.8, "ZEC": -5.17, "kPEPE": -5.15}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 10.2, "PENDLE": 9.9, "ZEC": 10.0}, "last_day": "2026-11-23", "top_apr": {"BTC": 4.9, "CRV": 5.8, "ENA": 9.0, "ETC": 5.3, "HYPE": 10.2, "LDO": 8.7, "LINK": 5.2, "PENDLE": 9.9, "RENDER": 7.4, "ZEC": 10.0}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-23"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-22 23:04: 36 trades, 38 names, weekend xs-dispersion 2.05%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-23T00:04 | xs_momentum | BNB | buy | $1,238 | 559.664 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | ZEC | buy | $1,083 | 796.75 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | SOL | buy | $1,066 | 70.632 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | SUI | sell | $44 | 0.655817 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | HBAR | sell | $1,113 | 0.0515479 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | kPEPE | sell | $1,179 | 0.00337961 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | LTC | sell | $1,674 | 40.1422 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_funding | BCH | buy | $5,563 | 199.151 | daily xs-funding (Binance recon) |
| 2026-11-23T00:04 | xs_funding | ETC | sell | $6,775 | 5.68563 | daily xs-funding (Binance recon) |
| 2026-11-23T00:04 | trend_breakout_majors | BTC | buy | $2,875 | 61184.6 | signal 2026-11-22 23h |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:COST | buy | $5,935 | 1103.42 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:SP500 | buy | $4,587 | 7002.81 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:STRC | buy | $4,179 | 95.0508 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:ANT | buy | $4,148 | 89.5172 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:AAPL | buy | $3,934 | 348.333 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:UNITREE | buy | $3,813 | 90.2361 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:META | buy | $3,619 | 571.237 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:XYZ100 | buy | $3,529 | 25535.2 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:IBM | buy | $3,523 | 227.775 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:AMZN | buy | $2,852 | 252.591 | weekend fade entry |
