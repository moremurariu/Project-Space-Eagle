# Paper trading status — 2026-11-16 04:04 UTC

BTC 64,982 · ETH 1,821.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,825 | -16.18% | -0.51% | -13.66% | -22.40% | -7.98 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,116 | -2.88% | -0.03% | -1.30% | -4.33% | -5.94 | 0.05 | +0.05 | 1 | $53 | $-97 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.57 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.80 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,034 | +0.03% | -0.08% | -0.09% | -0.24% | 2.08 | 0.90 | +0.00 | 6 | $296 | $312 |
| trend_breakout_majors | $114,055 | +14.05% | +0.12% | +13.01% | -5.48% | 8.22 | 0.53 | -0.53 | 3 | $116 | $168 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.59 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.77 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,942 | +0.94% | +0.25% | +1.63% | -1.56% | 2.75 | 0.93 | -0.00 | 20 | $581 | $150 |
| xs_momentum | $98,864 | -1.14% | -0.10% | -1.93% | -2.98% | nan | 0.91 | -0.03 | 37 | $195 | $-4 |
| xyz_weekend_fade | $102,147 | +2.15% | +0.92% | +2.27% | -1.21% | nan | 0.98 | -0.07 | 36 | $668 | $18 |

**Combined (ex-benchmark): $1,113,264 on $1,100,000 → +1.21%**
Combined BTC exposure (7d realised, 4h returns): beta -0.084 (≈ -0.92 books of BTC), corr -0.89

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65015.4 | $-16,105 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64982.5 | $-822 | +0.052 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 894.001 | $-4,902 | +0.156 |
| ZEC | -17.4301 | 1175 | 893.455 | $4,907 | -0.156 |
| HYPE/spot | 233.128 | 83.2675 | 66.4085 | $-3,930 | +0.155 |
| HYPE | -233.128 | 82.7772 | 66.3565 | $3,828 | -0.155 |
| PENDLE | -9671.02 | 1.81759 | 1.43825 | $3,669 | -0.139 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4381 | $-3,678 | +0.139 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -281.278 | 91.199 | 71.8405 | $5,445 | -0.177 |
| BTC | -0.309603 | 75630.7 | 64982.5 | $3,297 | -0.176 |
| ETH | -10.8509 | 2187.58 | 1821.65 | $3,971 | -0.173 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64982.5 | $484 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.303975 | $-146 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.960885 | $1,573 | -0.071 |
| ADA | 36975.2 | 0.155319 | 0.153825 | $-55 | +0.056 |
| BCH | 23.4344 | 238.546 | 208.3 | $-709 | +0.048 |
| SEI | 123581 | 0.0376927 | 0.039431 | $215 | +0.048 |
| TAO | -25.0244 | 220.686 | 191.95 | $719 | -0.048 |
| LINK | -516.66 | 11.3839 | 9.18905 | $1,134 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.261545 | $364 | +0.047 |
| APT | 9654.61 | 0.457585 | 0.4561 | $-14 | +0.044 |
| LDO | -14834.4 | 0.308439 | 0.27897 | $437 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.3838 | $490 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13471 | $-454 | +0.036 |
| PENDLE | -2505.4 | 1.58072 | 1.43825 | $357 | -0.036 |
| TIA | 10070 | 0.345986 | 0.35586 | $99 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08104 | $-515 | +0.035 |
| ZEC | -3.83105 | 862.252 | 893.455 | $-120 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.1865 | $582 | -0.031 |
| WLD | 10593 | 0.336516 | 0.26671 | $-739 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.118915 | $610 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.303975 | $-287 | +0.079 |
| SOL | -87.2593 | 91.9832 | 71.8405 | $1,758 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.960885 | $396 | -0.061 |
| RENDER | 3386.49 | 1.53278 | 1.3838 | $-505 | +0.047 |
| BTC | -0.0697602 | 67434.2 | 64982.5 | $171 | -0.046 |
| AVAX | 607.041 | 7.89216 | 7.2037 | $-418 | +0.044 |
| INJ | 607.666 | 7.33527 | 6.86655 | $-285 | +0.042 |
| ARB | 29979.9 | 0.152074 | 0.13471 | $-521 | +0.041 |
| JUP | 15235.8 | 0.263176 | 0.261545 | $-25 | +0.040 |
| PENDLE | -2522.74 | 1.58296 | 1.43825 | $365 | -0.037 |
| DOT | -4637.48 | 0.841601 | 0.72768 | $528 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.00381 | $-833 | +0.031 |
| BCH | -14.0941 | 212.452 | 208.3 | $59 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.058915 | $359 | -0.028 |
| ETH | -1.46269 | 1817.83 | 1821.65 | $-6 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26671 | $774 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.2033 | $761 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.08104 | $-630 | +0.020 |
| UNI | 341.169 | 7.08573 | 5.19475 | $-645 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.3008 | $3 | +0.016 |
| ADA | -9776.31 | 0.190675 | 0.153825 | $360 | -0.015 |
| TAO | 7.68451 | 200.727 | 191.95 | $-67 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.60632 | $452 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.039431 | $262 | -0.013 |
| BNB | -2.14595 | 596.735 | 592.02 | $10 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.139505 | $51 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27897 | $48 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.118915 | $-30 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.058876 | $195 | -0.010 |
| AAVE | 8.4283 | 131.483 | 108.885 | $-190 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35586 | $25 | +0.009 |
| APT | -1385.4 | 0.626089 | 0.4561 | $236 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 893.455 | $-199 | +0.006 |
| NEAR | 164.539 | 3.21111 | 3.1865 | $-4 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.30679 | $-57 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.079225 | $-27 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.72766 | $-11 | +0.000 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:MSTR | 107.175 | 101.7 | 103.605 | $204 | +0.109 |
| xyz:PURRDAT | 1374.3 | 7.94377 | 7.97035 | $37 | +0.107 |
| xyz:CRCL | 141.952 | 58.8308 | 58.497 | $-47 | +0.081 |
| xyz:COIN | 66.2433 | 121.002 | 120.29 | $-47 | +0.078 |
| xyz:NVDA | -35.5169 | 166.557 | 162.515 | $144 | -0.057 |
| xyz:NBIS | -38.985 | 117.666 | 111.795 | $229 | -0.043 |
| xyz:AAPL | -10.0333 | 359.898 | 356.685 | $32 | -0.035 |
| xyz:SKHX | -4.04354 | 905.53 | 854.625 | $206 | -0.034 |
| xyz:SKHY | -25.0067 | 132.85 | 127.085 | $144 | -0.031 |
| xyz:TSLA | -7.31388 | 392.66 | 386.78 | $43 | -0.028 |
| xyz:SPCX | -20.038 | 140 | 139.545 | $9 | -0.027 |
| xyz:HOOD | 36.023 | 76.0676 | 74.956 | $-40 | +0.026 |
| xyz:IBM | -10.5453 | 248.34 | 246.855 | $16 | -0.025 |
| xyz:XYZ100 | -0.092663 | 28069.1 | 27778.5 | $27 | -0.025 |
| xyz:MSFT | -4.84152 | 526.191 | 523.195 | $15 | -0.025 |
| xyz:GOOGL | -7.9314 | 296.85 | 293.995 | $23 | -0.023 |
| xyz:SMSN | -15.2881 | 153.075 | 148.23 | $74 | -0.022 |
| xyz:AMZN | -7.79693 | 267.121 | 264.69 | $19 | -0.020 |
| xyz:SP500 | -0.272659 | 7575.27 | 7540.35 | $10 | -0.020 |
| xyz:EWY | -14.158 | 142.995 | 139.94 | $43 | -0.019 |
| xyz:ZHIPU | 40.034 | 50.5065 | 46.408 | $-164 | +0.018 |
| xyz:CBRS | 14.3825 | 106.881 | 105.355 | $-22 | +0.015 |
| xyz:ANT | -14.5089 | 102.223 | 101.62 | $9 | -0.014 |
| xyz:DRAM | -35.1899 | 42.3639 | 40.8115 | $55 | -0.014 |
| xyz:UNITREE | -13.6311 | 101.43 | 103.75 | $-32 | -0.014 |
| xyz:META | -2.20162 | 640.883 | 635.975 | $11 | -0.014 |
| xyz:MRVL | -6.61861 | 156.143 | 152.03 | $27 | -0.010 |
| xyz:LITE | 1.26894 | 589.45 | 576.065 | $-17 | +0.007 |
| xyz:SNDK | 0.85415 | 873.385 | 850.83 | $-19 | +0.007 |
| xyz:AMD | -1.18441 | 530.998 | 518.145 | $15 | -0.006 |
| xyz:ORCL | -4.86148 | 124.674 | 123.575 | $5 | -0.006 |
| xyz:INTC | 7.74056 | 58.9717 | 57.7595 | $-9 | +0.004 |
| xyz:CXMT | -68.0257 | 5.9968 | 5.89075 | $7 | -0.004 |
| xyz:MRNA | -1.55358 | 221.486 | 217.585 | $6 | -0.003 |
| xyz:MU | -0.369063 | 772.665 | 754.275 | $7 | -0.003 |
| xyz:SOXL | 4.26559 | 57.705 | 54.2615 | $-15 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -1.65, "INJ": -1.5, "LTC": -1.43, "PENDLE": -1.44, "ZEC": -1.94}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -1.65, "INJ": -1.5, "LTC": -1.43, "PENDLE": -1.44, "ZEC": -1.94}}`
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
