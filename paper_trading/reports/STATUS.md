# Paper trading status — 2026-11-23 07:04 UTC

BTC 61,488 · ETH 1,724.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $79,291 | -20.71% | +4.08% | -4.75% | -30.99% | -6.17 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,842 | -3.16% | +0.19% | -0.25% | -4.87% | -5.28 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.47 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.14 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,123 | +0.12% | +0.02% | +0.11% | -0.26% | 2.25 | 0.88 | +0.00 | 6 | $302 | $398 |
| trend_breakout_majors | $116,432 | +16.43% | -2.14% | +1.78% | -5.48% | 6.44 | 0.54 | -0.54 | 3 | $128 | $180 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.97 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.12 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.90 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,451 | +0.45% | +0.22% | -0.55% | -1.64% | 1.02 | 0.95 | +0.00 | 20 | $663 | $289 |
| xs_momentum | $98,219 | -1.78% | -0.42% | -0.47% | -3.01% | -2.95 | 0.93 | -0.02 | 35 | $243 | $53 |
| xyz_weekend_fade | $103,148 | +3.15% | -0.13% | +0.98% | -1.21% | 4.65 | 0.94 | +0.08 | 36 | $907 | $65 |

**Combined (ex-benchmark): $1,115,322 on $1,100,000 → +1.39%**
Combined BTC exposure (7d realised, 4h returns): beta -0.067 (≈ -0.74 books of BTC), corr -0.92

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 61504 | $-20,639 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 61487.5 | $-1,091 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.3235 | $-4,691 | +0.155 |
| PENDLE | -11765.6 | 1.72106 | 1.32185 | $4,697 | -0.155 |
| HYPE/spot | 233.128 | 83.2675 | 62.7485 | $-4,784 | +0.146 |
| HYPE | -233.128 | 82.7772 | 62.7075 | $4,679 | -0.146 |
| ZEC/spot | 17.43 | 1175.27 | 786.288 | $-6,780 | +0.137 |
| ZEC | -17.4301 | 1175 | 786.165 | $6,777 | -0.137 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 61487.5 | $4,137 | -0.210 |
| ETH | -11.3433 | 2117.81 | 1724.05 | $4,467 | -0.168 |
| SOL | -265.603 | 89.0415 | 72.0625 | $4,510 | -0.164 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 61487.5 | $988 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.305205 | $-111 | +0.087 |
| LINK | -662.607 | 8.53769 | 8.89345 | $-236 | -0.059 |
| ADA | 39260.6 | 0.144249 | 0.14606 | $71 | +0.057 |
| BCH | 27.9335 | 199.151 | 199.59 | $12 | +0.056 |
| APT | 12637.8 | 0.436568 | 0.386 | $-639 | +0.049 |
| TAO | -25.0244 | 220.686 | 191.485 | $731 | -0.048 |
| RENDER | -3551.19 | 1.49617 | 1.3355 | $571 | -0.047 |
| AAVE | -44.8727 | 112.188 | 98.2255 | $627 | -0.044 |
| JUP | 18056.4 | 0.241363 | 0.23769 | $-66 | +0.043 |
| PENDLE | -3236.78 | 1.54767 | 1.32185 | $731 | -0.043 |
| SEI | 123581 | 0.0376927 | 0.034489 | $-396 | +0.042 |
| OP | 58569.1 | 0.0891484 | 0.07162 | $-1,027 | +0.042 |
| NEAR | -1371.79 | 3.51678 | 3.00845 | $697 | -0.041 |
| UNI | -836.22 | 5.16466 | 4.775 | $326 | -0.040 |
| WLD | 15387.1 | 0.31305 | 0.23788 | $-1,157 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.103455 | $975 | -0.035 |
| TIA | 10070 | 0.345986 | 0.3373 | $-87 | +0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003428 | $-414 | +0.031 |
| ZEC | -3.83105 | 862.252 | 786.165 | $291 | -0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.305205 | $-262 | +0.103 |
| BTC | -0.122056 | 65912.7 | 61487.5 | $540 | -0.076 |
| BNB | -9.13414 | 580.021 | 557.53 | $205 | -0.052 |
| ETH | -2.88908 | 1804.39 | 1724.05 | $232 | -0.051 |
| RENDER | 3386.49 | 1.53278 | 1.3355 | $-668 | +0.046 |
| FIL | -7861.93 | 0.620713 | 0.54146 | $623 | -0.043 |
| ATOM | 3503.24 | 1.28664 | 1.181 | $-370 | +0.042 |
| INJ | 607.666 | 7.33527 | 6.2193 | $-678 | +0.038 |
| JUP | 15235.8 | 0.263176 | 0.23769 | $-388 | +0.037 |
| CRV | -19201.4 | 0.232521 | 0.18346 | $942 | -0.036 |
| AAVE | 32.3234 | 108.591 | 98.2255 | $-335 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.386 | $370 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.7108 | $-502 | +0.029 |
| BCH | -14.0941 | 212.452 | 199.59 | $181 | -0.029 |
| LTC | -67.4945 | 40.3115 | 40.591 | $-19 | -0.028 |
| NEAR | 890.083 | 3.11366 | 3.00845 | $-94 | +0.027 |
| TAO | 12.9296 | 197.549 | 191.485 | $-78 | +0.025 |
| DOGE | -44257.5 | 0.0643319 | 0.055637 | $385 | -0.025 |
| ENA | -21996.7 | 0.108918 | 0.103455 | $120 | -0.023 |
| HBAR | -38109.1 | 0.0598269 | 0.052176 | $292 | -0.020 |
| TIA | 5612.94 | 0.336231 | 0.3373 | $6 | +0.019 |
| SOL | -21.0355 | 91.9832 | 72.0625 | $419 | -0.015 |
| WLD | -5872.05 | 0.34434 | 0.23788 | $625 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.94045 | $121 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.12857 | $-110 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11605 | $-328 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.32185 | $36 | +0.010 |
| SEI | 23468.7 | 0.0381037 | 0.034489 | $-85 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.67336 | $202 | -0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003428 | $-246 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07162 | $-8 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.775 | $-223 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.24058 | $-13 | -0.004 |
| ZEC | 0.47487 | 796.75 | 786.165 | $-5 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.28775 | $-69 | +0.002 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:SOXL | -264.497 | 33.7478 | 35.3635 | $-427 | -0.091 |
| xyz:COST | 5.37885 | 1103.42 | 1089.75 | $-74 | +0.057 |
| xyz:NBIS | -56.3175 | 86.8826 | 87.657 | $-44 | -0.048 |
| xyz:SP500 | 0.655074 | 7002.81 | 7069.75 | $44 | +0.045 |
| xyz:UNITREE | 42.2589 | 90.2361 | 106.29 | $678 | +0.044 |
| xyz:ANT | 46.3349 | 89.5172 | 91.9155 | $111 | +0.041 |
| xyz:STRC | 43.9632 | 95.0508 | 95.7235 | $30 | +0.041 |
| xyz:AAPL | 11.2936 | 348.333 | 345.605 | $-31 | +0.038 |
| xyz:NVDA | -27.6065 | 133.548 | 134.225 | $-19 | -0.036 |
| xyz:META | 6.33532 | 571.237 | 578.375 | $45 | +0.036 |
| xyz:XYZ100 | 0.138214 | 25535.2 | 25781.5 | $34 | +0.035 |
| xyz:IBM | 15.4686 | 227.775 | 227.52 | $-4 | +0.034 |
| xyz:MSTR | -40.5579 | 82.455 | 84.208 | $-71 | -0.033 |
| xyz:AMZN | 11.2909 | 252.591 | 254.895 | $26 | +0.028 |
| xyz:CBRS | -32.3279 | 86.9154 | 86.77 | $5 | -0.027 |
| xyz:SKHY | -23.7567 | 109.14 | 112.425 | $-78 | -0.026 |
| xyz:MSFT | 5.37955 | 495.48 | 496.1 | $3 | +0.026 |
| xyz:SPCX | 20.186 | 130.902 | 131.725 | $17 | +0.026 |
| xyz:CRCL | -51.3591 | 50.1159 | 50.5 | $-20 | -0.025 |
| xyz:BABA | 30.2312 | 83.0533 | 84.858 | $55 | +0.025 |
| xyz:SKHX | -2.87179 | 779.493 | 802.015 | $-65 | -0.022 |
| xyz:IREN | -166.006 | 12.8106 | 12.6255 | $31 | -0.020 |
| xyz:ZHIPU | -45.2724 | 38.8205 | 44.357 | $-251 | -0.019 |
| xyz:PURRDAT | -209.653 | 7.87569 | 7.9278 | $-11 | -0.016 |
| xyz:HOOD | -24.3395 | 65.1215 | 66.017 | $-22 | -0.016 |
| xyz:SNDK | -18.9852 | 83.0642 | 84.229 | $-22 | -0.016 |
| xyz:GOOGL | 5.77104 | 273.323 | 275.58 | $13 | +0.015 |
| xyz:INTC | -19.5296 | 53.6475 | 53.9955 | $-7 | -0.010 |
| xyz:AMD | -2.18996 | 442.79 | 448.175 | $-12 | -0.010 |
| xyz:TSLA | 2.4212 | 357.605 | 359.13 | $4 | +0.008 |
| xyz:MU | 1.06539 | 615.696 | 634.325 | $20 | +0.007 |
| xyz:MRVL | -4.33833 | 127.25 | 128.765 | $-7 | -0.005 |
| xyz:EWY | 4.12068 | 127.505 | 127.5 | $-0 | +0.005 |
| xyz:DRAM | -13.9588 | 35.0429 | 36.0815 | $-14 | -0.005 |
| xyz:CXMT | -83.1059 | 5.0613 | 5.35535 | $-24 | -0.004 |
| xyz:ORCL | -3.21642 | 104.62 | 105.15 | $-2 | -0.003 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 0.76, "ETH": 1.06, "INJ": 1.41, "TRX": 0.23, "ZEC": 0.46}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 0.76, "ETH": 1.06, "INJ": 1.41, "TRX": 0.23, "ZEC": 0.46}}`
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
