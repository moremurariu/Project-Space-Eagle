# Paper trading status — 2026-11-16 10:04 UTC

BTC 64,316 · ETH 1,796.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $82,958 | -17.04% | -1.90% | -13.83% | -22.40% | -8.37 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,064 | -2.94% | -0.10% | -1.14% | -4.33% | -6.00 | 0.05 | +0.05 | 1 | $53 | $-97 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.56 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.77 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,033 | +0.03% | -0.02% | -0.10% | -0.24% | 2.06 | 0.90 | +0.00 | 6 | $296 | $314 |
| trend_breakout_majors | $114,791 | +14.79% | +0.78% | +13.07% | -5.48% | 8.53 | 0.52 | -0.52 | 3 | $116 | $167 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.57 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.74 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,842 | +0.84% | +0.12% | +1.67% | -1.56% | 2.47 | 0.92 | -0.00 | 20 | $581 | $154 |
| xs_momentum | $98,810 | -1.19% | -0.33% | -2.00% | -2.98% | nan | 0.90 | -0.03 | 37 | $195 | $-2 |
| xyz_weekend_fade | $102,191 | +2.19% | +0.96% | +2.89% | -1.21% | 5.00 | 0.97 | -0.07 | 36 | $668 | $25 |

**Combined (ex-benchmark): $1,113,839 on $1,100,000 → +1.26%**
Combined BTC exposure (7d realised, 4h returns): beta -0.084 (≈ -0.92 books of BTC), corr -0.89

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64344.1 | $-16,972 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64315.5 | $-873 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 233.128 | 83.2675 | 66.8525 | $-3,827 | +0.156 |
| HYPE | -233.128 | 82.7772 | 66.8305 | $3,718 | -0.156 |
| ZEC/spot | 17.43 | 1175.27 | 887.673 | $-5,013 | +0.155 |
| ZEC | -17.4301 | 1175 | 887.385 | $5,013 | -0.155 |
| PENDLE | -9671.02 | 1.81759 | 1.43155 | $3,733 | -0.138 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4323 | $-3,735 | +0.138 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -281.278 | 91.199 | 70.9185 | $5,704 | -0.174 |
| BTC | -0.309603 | 75630.7 | 64315.5 | $3,503 | -0.173 |
| ETH | -10.8509 | 2187.58 | 1796.65 | $4,242 | -0.170 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64315.5 | $580 | -0.092 |
| TRX | 28470.1 | 0.309116 | 0.302405 | $-191 | +0.085 |
| XRP | -7432.17 | 1.17255 | 0.94518 | $1,690 | -0.070 |
| ADA | 36975.2 | 0.155319 | 0.151475 | $-142 | +0.056 |
| SEI | 123581 | 0.0376927 | 0.03903 | $165 | +0.048 |
| BCH | 23.4344 | 238.546 | 205.355 | $-778 | +0.048 |
| TAO | -25.0244 | 220.686 | 188.55 | $804 | -0.047 |
| LINK | -516.66 | 11.3839 | 9.09005 | $1,185 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.25196 | $191 | +0.045 |
| APT | 9654.61 | 0.457585 | 0.4568 | $-8 | +0.044 |
| LDO | -14834.4 | 0.308439 | 0.27614 | $479 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.3683 | $532 | -0.037 |
| ARB | 27302.4 | 0.151348 | 0.13338 | $-491 | +0.036 |
| PENDLE | -2505.4 | 1.58072 | 1.43155 | $374 | -0.036 |
| OP | 44197.2 | 0.0926854 | 0.07983 | $-568 | +0.035 |
| TIA | 10070 | 0.345986 | 0.34725 | $13 | +0.035 |
| ZEC | -3.83105 | 862.252 | 887.385 | $-96 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.187 | $582 | -0.032 |
| WLD | 10593 | 0.336516 | 0.26343 | $-774 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.116675 | $661 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.302405 | $-328 | +0.079 |
| SOL | -87.2593 | 91.9832 | 70.9185 | $1,838 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.94518 | $494 | -0.060 |
| RENDER | 3386.49 | 1.53278 | 1.3683 | $-557 | +0.047 |
| BTC | -0.0697602 | 67434.2 | 64315.5 | $218 | -0.045 |
| AVAX | 607.041 | 7.89216 | 7.1307 | $-462 | +0.044 |
| INJ | 607.666 | 7.33527 | 6.6571 | $-412 | +0.041 |
| ARB | 29979.9 | 0.152074 | 0.13338 | $-560 | +0.040 |
| JUP | 15235.8 | 0.263176 | 0.25196 | $-171 | +0.039 |
| PENDLE | -2522.74 | 1.58296 | 1.43155 | $382 | -0.037 |
| DOT | -4637.48 | 0.841601 | 0.72199 | $555 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.003763 | $-871 | +0.031 |
| BCH | -14.0941 | 212.452 | 205.355 | $100 | -0.029 |
| DOGE | -46908.3 | 0.0665749 | 0.058261 | $390 | -0.028 |
| WLD | -9976.36 | 0.34434 | 0.26343 | $807 | -0.027 |
| ETH | -1.46269 | 1817.83 | 1796.65 | $31 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.20125 | $783 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.07983 | $-659 | +0.019 |
| UNI | 341.169 | 7.08573 | 5.11895 | $-671 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.2922 | $-7 | +0.016 |
| ADA | -9776.31 | 0.190675 | 0.151475 | $383 | -0.015 |
| TAO | 7.68451 | 200.727 | 188.55 | $-94 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.59114 | $488 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.03903 | $275 | -0.013 |
| BNB | -2.14595 | 596.735 | 589.24 | $16 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.137115 | $71 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27614 | $36 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.116675 | $-11 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.05779 | $212 | -0.010 |
| AAVE | 8.4283 | 131.483 | 108.745 | $-192 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.34725 | $3 | +0.009 |
| APT | -1385.4 | 0.626089 | 0.4568 | $235 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 887.385 | $-203 | +0.006 |
| NEAR | 164.539 | 3.21111 | 3.187 | $-4 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.30256 | $-59 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.078333 | $-29 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.71786 | $-11 | +0.000 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:MSTR | 107.175 | 101.7 | 102.695 | $107 | +0.108 |
| xyz:PURRDAT | 1374.3 | 7.94377 | 7.99825 | $75 | +0.108 |
| xyz:CRCL | 141.952 | 58.8308 | 57.904 | $-132 | +0.080 |
| xyz:COIN | 66.2433 | 121.002 | 119.295 | $-113 | +0.077 |
| xyz:NVDA | -35.5169 | 166.557 | 159.795 | $240 | -0.056 |
| xyz:NBIS | -38.985 | 117.666 | 109.515 | $318 | -0.042 |
| xyz:AAPL | -10.0333 | 359.898 | 357.71 | $22 | -0.035 |
| xyz:SKHX | -4.04354 | 905.53 | 838.515 | $271 | -0.033 |
| xyz:SKHY | -25.0067 | 132.85 | 124.83 | $201 | -0.031 |
| xyz:TSLA | -7.31388 | 392.66 | 384.735 | $58 | -0.028 |
| xyz:SPCX | -20.038 | 140 | 139.725 | $6 | -0.027 |
| xyz:HOOD | 36.023 | 76.0676 | 73.957 | $-76 | +0.026 |
| xyz:IBM | -10.5453 | 248.34 | 247.105 | $13 | -0.025 |
| xyz:XYZ100 | -0.092663 | 28069.1 | 27663.5 | $38 | -0.025 |
| xyz:MSFT | -4.84152 | 526.191 | 526.785 | $-3 | -0.025 |
| xyz:GOOGL | -7.9314 | 296.85 | 294.455 | $19 | -0.023 |
| xyz:SMSN | -15.2881 | 153.075 | 146.875 | $95 | -0.022 |
| xyz:AMZN | -7.79693 | 267.121 | 264.885 | $17 | -0.020 |
| xyz:SP500 | -0.272659 | 7575.27 | 7532.55 | $12 | -0.020 |
| xyz:EWY | -14.158 | 142.995 | 139.025 | $56 | -0.019 |
| xyz:ZHIPU | 40.034 | 50.5065 | 44.649 | $-234 | +0.017 |
| xyz:CBRS | 14.3825 | 106.881 | 104.15 | $-39 | +0.015 |
| xyz:ANT | -14.5089 | 102.223 | 101.035 | $17 | -0.014 |
| xyz:DRAM | -35.1899 | 42.3639 | 40.1775 | $77 | -0.014 |
| xyz:UNITREE | -13.6311 | 101.43 | 102.77 | $-18 | -0.014 |
| xyz:META | -2.20162 | 640.883 | 636.055 | $11 | -0.014 |
| xyz:MRVL | -6.61861 | 156.143 | 149.57 | $44 | -0.010 |
| xyz:LITE | 1.26894 | 589.45 | 563.135 | $-33 | +0.007 |
| xyz:SNDK | 0.85415 | 873.385 | 831.675 | $-36 | +0.007 |
| xyz:AMD | -1.18441 | 530.998 | 508.69 | $26 | -0.006 |
| xyz:ORCL | -4.86148 | 124.674 | 122.23 | $12 | -0.006 |
| xyz:INTC | 7.74056 | 58.9717 | 56.836 | $-17 | +0.004 |
| xyz:CXMT | -68.0257 | 5.9968 | 5.7389 | $18 | -0.004 |
| xyz:MRNA | -1.55358 | 221.486 | 219.87 | $3 | -0.003 |
| xyz:MU | -0.369063 | 772.665 | 738 | $13 | -0.003 |
| xyz:SOXL | 4.26559 | 57.705 | 52.0045 | $-24 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -1.44, "FIL": -1.94, "HBAR": -1.4, "INJ": -1.75, "JUP": -2.81}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -1.44, "FIL": -1.94, "HBAR": -1.4, "INJ": -1.75, "JUP": -2.81}}`
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
