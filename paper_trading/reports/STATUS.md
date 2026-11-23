# Paper trading status — 2026-11-23 17:04 UTC

BTC 62,062 · ETH 1,765.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $80,075 | -19.93% | +1.68% | -3.65% | -30.99% | -5.62 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,886 | -3.11% | +0.08% | -0.19% | -4.87% | -5.14 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.11 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,120 | +0.12% | +0.00% | +0.10% | -0.26% | 2.20 | 0.92 | +0.00 | 6 | $302 | $401 |
| trend_breakout_majors | $115,120 | +15.12% | -1.69% | +0.45% | -7.51% | 5.73 | 0.56 | -0.56 | 3 | $128 | $182 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.95 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.09 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.83 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,637 | +0.64% | +0.55% | -0.24% | -1.64% | 1.26 | 0.98 | +0.00 | 20 | $663 | $298 |
| xs_momentum | $98,516 | -1.48% | +0.19% | -0.24% | -3.13% | -2.31 | 0.96 | -0.02 | 35 | $243 | $52 |
| xyz_weekend_fade | $98,098 | -1.90% | -5.02% | -4.67% | -5.64% | -1.24 | 1.06 | +0.03 | 36 | $907 | $111 |

**Combined (ex-benchmark): $1,109,485 on $1,100,000 → +0.86%**
Combined BTC exposure (7d realised, 4h returns): beta -0.085 (≈ -0.93 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 62111 | $-19,855 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 62062.5 | $-1,046 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.35017 | $-4,377 | +0.159 |
| PENDLE | -11765.6 | 1.72106 | 1.3491 | $4,376 | -0.159 |
| HYPE/spot | 233.128 | 83.2675 | 65.9277 | $-4,042 | +0.154 |
| HYPE | -233.128 | 82.7772 | 65.891 | $3,937 | -0.153 |
| ZEC/spot | 17.43 | 1175.27 | 841.01 | $-5,826 | +0.146 |
| ZEC | -17.4301 | 1175 | 840.745 | $5,826 | -0.146 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 62062.5 | $3,908 | -0.215 |
| ETH | -11.3433 | 2117.81 | 1765.65 | $3,995 | -0.174 |
| SOL | -265.603 | 89.0415 | 74.371 | $3,897 | -0.172 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 62062.5 | $905 | -0.089 |
| TRX | 28470.1 | 0.309116 | 0.311765 | $75 | +0.088 |
| LINK | -662.607 | 8.53769 | 9.21565 | $-449 | -0.061 |
| ADA | 39260.6 | 0.144249 | 0.15257 | $327 | +0.060 |
| BCH | 27.9335 | 199.151 | 205.505 | $177 | +0.057 |
| APT | 12637.8 | 0.436568 | 0.3954 | $-520 | +0.050 |
| TAO | -25.0244 | 220.686 | 196.29 | $610 | -0.049 |
| RENDER | -3551.19 | 1.49617 | 1.3484 | $525 | -0.048 |
| JUP | 18056.4 | 0.241363 | 0.24942 | $145 | +0.045 |
| AAVE | -44.8727 | 112.188 | 100.325 | $532 | -0.045 |
| SEI | 123581 | 0.0376927 | 0.035845 | $-228 | +0.044 |
| PENDLE | -3236.78 | 1.54767 | 1.3491 | $643 | -0.043 |
| NEAR | -1371.79 | 3.51678 | 3.11365 | $553 | -0.042 |
| OP | 58569.1 | 0.0891484 | 0.07255 | $-972 | +0.042 |
| UNI | -836.22 | 5.16466 | 5.0393 | $105 | -0.042 |
| WLD | 15387.1 | 0.31305 | 0.24596 | $-1,032 | +0.038 |
| ENA | -33871.5 | 0.132226 | 0.10573 | $897 | -0.036 |
| TIA | 10070 | 0.345986 | 0.34367 | $-23 | +0.034 |
| ZEC | -3.83105 | 862.252 | 840.745 | $82 | -0.032 |
| kPEPE | 893923 | 0.0038915 | 0.003568 | $-289 | +0.032 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.311765 | $-44 | +0.105 |
| BTC | -0.122056 | 65912.7 | 62062.5 | $470 | -0.077 |
| BNB | -9.13414 | 580.021 | 565.01 | $137 | -0.052 |
| ETH | -2.88908 | 1804.39 | 1765.65 | $112 | -0.052 |
| RENDER | 3386.49 | 1.53278 | 1.3484 | $-624 | +0.046 |
| FIL | -7861.93 | 0.620713 | 0.57176 | $385 | -0.046 |
| ATOM | 3503.24 | 1.28664 | 1.2564 | $-106 | +0.045 |
| INJ | 607.666 | 7.33527 | 6.7966 | $-327 | +0.042 |
| JUP | 15235.8 | 0.263176 | 0.24942 | $-210 | +0.039 |
| CRV | -19201.4 | 0.232521 | 0.18811 | $853 | -0.037 |
| AAVE | 32.3234 | 108.591 | 100.325 | $-267 | +0.033 |
| APT | -7937.59 | 0.43262 | 0.3954 | $295 | -0.032 |
| AVAX | 424.946 | 7.89216 | 6.9583 | $-397 | +0.030 |
| BCH | -14.0941 | 212.452 | 205.505 | $98 | -0.029 |
| LTC | -67.4945 | 40.3115 | 42.3355 | $-137 | -0.029 |
| NEAR | 890.083 | 3.11366 | 3.11365 | $-0 | +0.028 |
| DOGE | -44257.5 | 0.0643319 | 0.057984 | $281 | -0.026 |
| TAO | 12.9296 | 197.549 | 196.29 | $-16 | +0.026 |
| ENA | -21996.7 | 0.108918 | 0.10573 | $70 | -0.024 |
| HBAR | -38109.1 | 0.0598269 | 0.054541 | $201 | -0.021 |
| TIA | 5612.94 | 0.336231 | 0.34367 | $42 | +0.020 |
| SOL | -21.0355 | 91.9832 | 74.371 | $370 | -0.016 |
| WLD | -5872.05 | 0.34434 | 0.24596 | $578 | -0.015 |
| XRP | -1446.41 | 1.02442 | 0.970655 | $78 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.13311 | $-68 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11565 | $-332 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.3491 | $57 | +0.010 |
| SEI | 23468.7 | 0.0381037 | 0.035845 | $-53 | +0.009 |
| DOT | -1201.93 | 0.841601 | 0.6976 | $173 | -0.009 |
| kPEPE | 173237 | 0.00484647 | 0.003568 | $-221 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07255 | $-14 | -0.005 |
| UNI | 96.3288 | 7.08573 | 5.0393 | $-197 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.24812 | $-26 | -0.005 |
| ZEC | 0.47487 | 796.75 | 840.745 | $21 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.29858 | $-62 | +0.002 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:SOXL | -264.497 | 33.7478 | 43.112 | $-2,477 | -0.116 |
| xyz:COST | 5.37885 | 1103.42 | 1076.25 | $-146 | +0.059 |
| xyz:NBIS | -56.3175 | 86.8826 | 101.04 | $-797 | -0.058 |
| xyz:SP500 | 0.655074 | 7002.81 | 7247.75 | $160 | +0.048 |
| xyz:NVDA | -27.6065 | 133.548 | 166.335 | $-905 | -0.047 |
| xyz:UNITREE | 42.2589 | 90.2361 | 108.43 | $769 | +0.047 |
| xyz:STRC | 43.9632 | 95.0508 | 97.9995 | $130 | +0.044 |
| xyz:ANT | 46.3349 | 89.5172 | 87.257 | $-105 | +0.041 |
| xyz:AAPL | 11.2936 | 348.333 | 347.725 | $-7 | +0.040 |
| xyz:META | 6.33532 | 571.237 | 596.15 | $158 | +0.039 |
| xyz:XYZ100 | 0.138214 | 25535.2 | 26776.5 | $172 | +0.038 |
| xyz:IBM | 15.4686 | 227.775 | 233.245 | $85 | +0.037 |
| xyz:MSTR | -40.5579 | 82.455 | 87.053 | $-186 | -0.036 |
| xyz:CBRS | -32.3279 | 86.9154 | 98.893 | $-387 | -0.033 |
| xyz:SKHY | -23.7567 | 109.14 | 125.065 | $-378 | -0.030 |
| xyz:AMZN | 11.2909 | 252.591 | 256.55 | $45 | +0.030 |
| xyz:SPCX | 20.186 | 130.902 | 136.68 | $117 | +0.028 |
| xyz:CRCL | -51.3591 | 50.1159 | 52.563 | $-126 | -0.028 |
| xyz:MSFT | 5.37955 | 495.48 | 498.885 | $18 | +0.027 |
| xyz:BABA | 30.2312 | 83.0533 | 86.861 | $115 | +0.027 |
| xyz:SKHX | -2.87179 | 779.493 | 881.565 | $-293 | -0.026 |
| xyz:IREN | -166.006 | 12.8106 | 15.157 | $-390 | -0.026 |
| xyz:ZHIPU | -45.2724 | 38.8205 | 46.243 | $-336 | -0.021 |
| xyz:PURRDAT | -209.653 | 7.87569 | 8.4534 | $-121 | -0.018 |
| xyz:SNDK | -18.9852 | 83.0642 | 91.558 | $-161 | -0.018 |
| xyz:HOOD | -24.3395 | 65.1215 | 67.643 | $-61 | -0.017 |
| xyz:GOOGL | 5.77104 | 273.323 | 283.635 | $60 | +0.017 |
| xyz:INTC | -19.5296 | 53.6475 | 57.869 | $-82 | -0.012 |
| xyz:AMD | -2.18996 | 442.79 | 460.25 | $-38 | -0.010 |
| xyz:TSLA | 2.4212 | 357.605 | 389.895 | $78 | +0.010 |
| xyz:MU | 1.06539 | 615.696 | 709.44 | $100 | +0.008 |
| xyz:MRVL | -4.33833 | 127.25 | 144.905 | $-77 | -0.006 |
| xyz:EWY | 4.12068 | 127.505 | 136.415 | $37 | +0.006 |
| xyz:DRAM | -13.9588 | 35.0429 | 39.799 | $-66 | -0.006 |
| xyz:CXMT | -83.1059 | 5.0613 | 5.71005 | $-54 | -0.005 |
| xyz:ORCL | -3.21642 | 104.62 | 114.325 | $-31 | -0.004 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 1.7, "BTC": 1.5, "OP": 2.38, "RENDER": 1.73, "TRX": 1.4}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 1.7, "BTC": 1.5, "OP": 2.38, "RENDER": 1.73, "TRX": 1.4}}`
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
