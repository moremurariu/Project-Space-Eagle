# Paper trading status — 2026-11-16 19:04 UTC

BTC 64,936 · ETH 1,816.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,749 | -16.25% | +0.47% | -13.18% | -22.40% | -7.74 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,112 | -2.89% | +0.02% | -1.14% | -4.33% | -5.82 | 0.05 | +0.05 | 1 | $53 | $-97 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.56 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.72 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,011 | +0.01% | -0.02% | -0.11% | -0.26% | 1.78 | 0.90 | -0.00 | 6 | $296 | $321 |
| trend_breakout_majors | $114,322 | +14.32% | +0.00% | +12.56% | -5.48% | 8.14 | 0.52 | -0.52 | 3 | $116 | $171 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.52 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.69 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,864 | +0.86% | -0.23% | +1.44% | -1.56% | 2.45 | 0.92 | -0.00 | 20 | $581 | $160 |
| xs_momentum | $98,780 | -1.22% | -0.41% | -1.58% | -2.98% | -2.90 | 0.90 | -0.03 | 37 | $195 | $-2 |
| xyz_weekend_fade | $103,364 | +3.36% | +2.12% | +2.51% | -1.21% | 6.49 | 0.95 | -0.05 | 36 | $668 | $30 |

**Combined (ex-benchmark): $1,114,559 on $1,100,000 → +1.32%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.95 books of BTC), corr -0.90

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64957 | $-16,181 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64935.5 | $-825 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE | -233.128 | 82.7772 | 67.2085 | $3,629 | -0.157 |
| HYPE/spot | 233.128 | 83.2675 | 67.1745 | $-3,752 | +0.157 |
| ZEC | -17.4301 | 1175 | 892.265 | $4,928 | -0.156 |
| ZEC/spot | 17.43 | 1175.27 | 891.853 | $-4,940 | +0.155 |
| PENDLE | -9671.02 | 1.81759 | 1.4342 | $3,708 | -0.139 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4345 | $-3,713 | +0.139 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.309603 | 75630.7 | 64935.5 | $3,311 | -0.176 |
| SOL | -281.278 | 91.199 | 71.1625 | $5,636 | -0.175 |
| ETH | -10.8509 | 2187.58 | 1816.25 | $4,029 | -0.172 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64935.5 | $490 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.30665 | $-70 | +0.087 |
| XRP | -7432.17 | 1.17255 | 0.956275 | $1,607 | -0.070 |
| ADA | 36975.2 | 0.155319 | 0.15325 | $-76 | +0.056 |
| BCH | 23.4344 | 238.546 | 208.275 | $-709 | +0.048 |
| SEI | 123581 | 0.0376927 | 0.039102 | $174 | +0.048 |
| LINK | -516.66 | 11.3839 | 9.18535 | $1,136 | -0.047 |
| TAO | -25.0244 | 220.686 | 189.465 | $781 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.248185 | $123 | +0.044 |
| APT | 9654.61 | 0.457585 | 0.4585 | $9 | +0.044 |
| LDO | -14834.4 | 0.308439 | 0.27872 | $441 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.37455 | $515 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13238 | $-518 | +0.036 |
| TIA | 10070 | 0.345986 | 0.35722 | $113 | +0.036 |
| PENDLE | -2505.4 | 1.58072 | 1.4342 | $367 | -0.036 |
| OP | 44197.2 | 0.0926854 | 0.08023 | $-550 | +0.035 |
| ZEC | -3.83105 | 862.252 | 892.265 | $-115 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.16105 | $607 | -0.031 |
| WLD | 10593 | 0.336516 | 0.26274 | $-782 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.115805 | $680 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30665 | $-218 | +0.080 |
| SOL | -87.2593 | 91.9832 | 71.1625 | $1,817 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.956275 | $425 | -0.060 |
| RENDER | 3386.49 | 1.53278 | 1.37455 | $-536 | +0.047 |
| BTC | -0.0697602 | 67434.2 | 64935.5 | $174 | -0.046 |
| AVAX | 607.041 | 7.89216 | 7.1493 | $-451 | +0.044 |
| INJ | 607.666 | 7.33527 | 6.86185 | $-288 | +0.042 |
| ARB | 29979.9 | 0.152074 | 0.13238 | $-590 | +0.040 |
| JUP | 15235.8 | 0.263176 | 0.248185 | $-228 | +0.038 |
| PENDLE | -2522.74 | 1.58296 | 1.4342 | $375 | -0.037 |
| DOT | -4637.48 | 0.841601 | 0.72997 | $518 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.0039 | $-761 | +0.032 |
| BCH | -14.0941 | 212.452 | 208.275 | $59 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.059297 | $341 | -0.028 |
| ETH | -1.46269 | 1817.83 | 1816.25 | $2 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26274 | $814 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.20628 | $727 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.08023 | $-649 | +0.020 |
| UNI | 341.169 | 7.08573 | 5.0891 | $-681 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.3312 | $39 | +0.016 |
| ADA | -9776.31 | 0.190675 | 0.15325 | $366 | -0.015 |
| TAO | 7.68451 | 200.727 | 189.465 | $-87 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.60688 | $451 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.039102 | $273 | -0.013 |
| BNB | -2.14595 | 596.735 | 590.38 | $14 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.137445 | $69 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27872 | $47 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.115805 | $-4 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057699 | $214 | -0.010 |
| AAVE | 8.4283 | 131.483 | 107.365 | $-203 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35722 | $28 | +0.009 |
| APT | -1385.4 | 0.626089 | 0.4585 | $232 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 892.265 | $-200 | +0.006 |
| NEAR | 164.539 | 3.21111 | 3.16105 | $-8 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.308005 | $-56 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.079894 | $-26 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.72136 | $-11 | +0.000 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:PURRDAT | 1374.3 | 7.94377 | 8.2965 | $485 | +0.110 |
| xyz:MSTR | 107.175 | 101.7 | 103.1 | $150 | +0.107 |
| xyz:CRCL | 141.952 | 58.8308 | 57.711 | $-159 | +0.079 |
| xyz:COIN | 66.2433 | 121.002 | 119.91 | $-72 | +0.077 |
| xyz:NVDA | -35.5169 | 166.557 | 155.995 | $375 | -0.054 |
| xyz:NBIS | -38.985 | 117.666 | 107.17 | $409 | -0.040 |
| xyz:AAPL | -10.0333 | 359.898 | 352.685 | $72 | -0.034 |
| xyz:SKHX | -4.04354 | 905.53 | 829.36 | $308 | -0.032 |
| xyz:SKHY | -25.0067 | 132.85 | 124.075 | $219 | -0.030 |
| xyz:TSLA | -7.31388 | 392.66 | 377.49 | $111 | -0.027 |
| xyz:SPCX | -20.038 | 140 | 137.555 | $49 | -0.027 |
| xyz:HOOD | 36.023 | 76.0676 | 72.085 | $-143 | +0.025 |
| xyz:IBM | -10.5453 | 248.34 | 243.915 | $47 | -0.025 |
| xyz:XYZ100 | -0.092663 | 28069.1 | 27254.5 | $75 | -0.024 |
| xyz:MSFT | -4.84152 | 526.191 | 519.82 | $31 | -0.024 |
| xyz:GOOGL | -7.9314 | 296.85 | 285.535 | $90 | -0.022 |
| xyz:SMSN | -15.2881 | 153.075 | 145.77 | $112 | -0.022 |
| xyz:SP500 | -0.272659 | 7575.27 | 7426.65 | $41 | -0.020 |
| xyz:AMZN | -7.79693 | 267.121 | 257.74 | $73 | -0.019 |
| xyz:EWY | -14.158 | 142.995 | 137.605 | $76 | -0.019 |
| xyz:ZHIPU | 40.034 | 50.5065 | 44.079 | $-257 | +0.017 |
| xyz:CBRS | 14.3825 | 106.881 | 102.32 | $-66 | +0.014 |
| xyz:ANT | -14.5089 | 102.223 | 98.5025 | $54 | -0.014 |
| xyz:DRAM | -35.1899 | 42.3639 | 39.6695 | $95 | -0.014 |
| xyz:UNITREE | -13.6311 | 101.43 | 101.975 | $-7 | -0.013 |
| xyz:META | -2.20162 | 640.883 | 625.725 | $33 | -0.013 |
| xyz:MRVL | -6.61861 | 156.143 | 146.825 | $62 | -0.009 |
| xyz:SNDK | 0.85415 | 873.385 | 818.415 | $-47 | +0.007 |
| xyz:LITE | 1.26894 | 589.45 | 546.775 | $-54 | +0.007 |
| xyz:AMD | -1.18441 | 530.998 | 506.69 | $29 | -0.006 |
| xyz:ORCL | -4.86148 | 124.674 | 119.205 | $27 | -0.006 |
| xyz:INTC | 7.74056 | 58.9717 | 55.4805 | $-27 | +0.004 |
| xyz:CXMT | -68.0257 | 5.9968 | 5.6475 | $24 | -0.004 |
| xyz:MRNA | -1.55358 | 221.486 | 214.78 | $10 | -0.003 |
| xyz:MU | -0.369063 | 772.665 | 726.405 | $17 | -0.003 |
| xyz:SOXL | 4.26559 | 57.705 | 49.888 | $-33 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -1.9, "ENA": -1.44, "ETH": -1.36, "JUP": -1.88, "RENDER": -1.22}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -1.9, "ENA": -1.44, "ETH": -1.36, "JUP": -1.88, "RENDER": -1.22}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 6.5, "PENDLE": 11.0, "ZEC": 9.7}, "last_day": "2026-11-16", "top_apr": {"BNB": 7.0, "BTC": 8.3, "CRV": 9.4, "ETC": 6.4, "HYPE": 6.5, "LDO": 11.0, "NEAR": 9.0, "PENDLE": 11.0, "TAO": 6.6, "ZEC": 9.7}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-16"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-16T00:04 | xs_momentum | RENDER | buy | $1,300 | 1.40551 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | JUP | buy | $1,294 | 0.266987 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | NEAR | buy | $1,028 | 3.21111 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | ADA | buy | $1,027 | 0.155296 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | TAO | buy | $1,002 | 195.145 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | LTC | buy | $147 | 45.4998 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | LINK | sell | $721 | 9.29306 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | BTC | sell | $1,028 | 65324.2 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | AAVE | sell | $1,071 | 109.844 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | UNI | sell | $1,326 | 5.16872 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | BNB | sell | $1,828 | 596.735 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_funding | ADA | buy | $5,743 | 0.155319 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | APT | buy | $4,418 | 0.457585 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | ATOM | sell | $5,739 | 1.3098 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | XLM | sell | $6,275 | 0.141149 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | trend_breakout_majors | BTC | buy | $3,078 | 65330.8 | signal 2026-11-15 23h |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:PURRDAT | buy | $10,917 | 7.94377 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MSTR | buy | $10,900 | 101.7 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:CRCL | buy | $8,351 | 58.8308 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:COIN | buy | $8,016 | 121.002 | weekend fade entry |
