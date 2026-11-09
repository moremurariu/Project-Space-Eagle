# Paper trading status — 2026-11-09 20:04 UTC

BTC 74,878 · ETH 2,127.9 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $96,590 | -3.41% | -1.34% | -2.81% | -8.59% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $98,270 | -1.73% | -0.35% | -1.61% | -3.37% | nan | 0.25 | +0.25 | 1 | $43 | $-81 |
| crash_rebound | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,120 | +0.12% | -0.01% | +0.14% | -0.19% | nan | 1.47 | +0.00 | 10 | $225 | $235 |
| trend_breakout_majors | $101,566 | +1.57% | +1.87% | +0.66% | -5.48% | nan | 0.59 | -0.59 | 3 | $97 | $120 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.02% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $99,422 | -0.58% | -0.22% | +0.21% | -1.13% | nan | 0.98 | +0.07 | 20 | $418 | $84 |
| xs_momentum | $100,218 | +0.22% | -0.56% | +0.33% | -0.92% | nan | 0.84 | -0.01 | 38 | $135 | $-24 |
| xyz_weekend_fade | $101,210 | +1.21% | +1.21% | +1.21% | -1.21% | nan | 1.05 | +0.00 | 35 | $312 | $3 |

**Combined (ex-benchmark): $1,101,039 on $1,100,000 → +0.09%**
Combined BTC exposure (7d realised, 4h returns): beta -0.044 (≈ -0.49 books of BTC), corr -0.69

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 74902 | $-3,340 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.325465 | 77535.3 | 74878.5 | $-865 | +0.248 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.59625 | $2,141 | -0.154 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.596 | $-2,153 | +0.154 |
| HYPE/spot | 201.997 | 85.5551 | 75.1767 | $-2,096 | +0.152 |
| HYPE | -202.014 | 85.5972 | 75.1345 | $2,114 | -0.152 |
| UNI/spot | 2136.3 | 7.02501 | 7.03092 | $13 | +0.150 |
| UNI | -2133.42 | 7.03009 | 7.02805 | $4 | -0.150 |
| ENA | -93083.1 | 0.187964 | 0.155505 | $3,021 | -0.145 |
| ENA/spot | 93002.9 | 0.188053 | 0.15559 | $-3,019 | +0.145 |
| ZEC/spot | 11.6568 | 1287.44 | 1173.74 | $-1,325 | +0.137 |
| ZEC | -11.65 | 1287.49 | 1173.75 | $1,325 | -0.137 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -251.977 | 94.3829 | 86.402 | $2,011 | -0.214 |
| BTC | -0.271518 | 76194.7 | 74878.5 | $357 | -0.200 |
| ETH | -8.50484 | 2246.43 | 2127.95 | $1,008 | -0.178 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.316565 | $212 | +0.091 |
| XRP | -6275.1 | 1.21983 | 1.09245 | $799 | -0.069 |
| XLM | 44454.5 | 0.151093 | 0.147155 | $-175 | +0.066 |
| ETC | -937.414 | 6.88048 | 6.8828 | $-2 | -0.065 |
| LTC | -121.169 | 49.941 | 51.5915 | $-200 | -0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.067148 | $172 | +0.062 |
| BCH | 23.4344 | 238.546 | 239.51 | $23 | +0.056 |
| LINK | -516.66 | 11.3839 | 10.6685 | $370 | -0.055 |
| ATOM | 3529.46 | 1.49903 | 1.3744 | $-440 | +0.049 |
| SEI | 104421 | 0.0484475 | 0.044483 | $-414 | +0.047 |
| JUP | 18056.4 | 0.241363 | 0.255305 | $252 | +0.046 |
| TAO | -19.9244 | 222.943 | 224.245 | $-26 | -0.045 |
| APT | 7323.07 | 0.569334 | 0.5886 | $141 | +0.043 |
| LDO | -10898 | 0.325861 | 0.33024 | $-48 | -0.036 |
| WLD | 10593 | 0.336516 | 0.32094 | $-165 | +0.034 |
| RENDER | -2079.13 | 1.56364 | 1.62345 | $-124 | -0.034 |
| kPEPE | -646727 | 0.00494453 | 0.005115 | $-110 | -0.033 |
| TIA | 6583.75 | 0.474572 | 0.43755 | $-244 | +0.029 |
| ENA | -17260.6 | 0.163688 | 0.155505 | $141 | -0.027 |
| NEAR | -660.139 | 3.86026 | 3.61385 | $163 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 86.402 | $487 | -0.075 |
| TRX | 17747.4 | 0.317154 | 0.316565 | $-10 | +0.056 |
| ATOM | -3097.68 | 1.43333 | 1.3744 | $183 | -0.042 |
| UNI | 597.681 | 7.08573 | 7.02805 | $-34 | +0.042 |
| kPEPE | 803792 | 0.00484647 | 0.005115 | $216 | +0.041 |
| ADA | -22351.2 | 0.190675 | 0.178095 | $281 | -0.040 |
| BNB | 5.68814 | 705.088 | 687.795 | $-98 | +0.039 |
| SEI | -87205.6 | 0.0472943 | 0.044483 | $245 | -0.039 |
| ZEC | 3.22979 | 1213.4 | 1173.75 | $-128 | +0.038 |
| ETH | 1.45521 | 2238.8 | 2127.95 | $-161 | +0.031 |
| CRV | -11195.5 | 0.271234 | 0.26447 | $76 | -0.030 |
| PENDLE | -1848.04 | 1.60447 | 1.59625 | $15 | -0.029 |
| BCH | 11.4397 | 277.3 | 239.51 | $-432 | +0.027 |
| TAO | -11.3929 | 219.804 | 224.245 | $-51 | -0.025 |
| ARB | 14157.1 | 0.162612 | 0.17604 | $190 | +0.025 |
| OP | 24123.2 | 0.107142 | 0.10136 | $-139 | +0.024 |
| INJ | 290.053 | 7.16191 | 8.14895 | $286 | +0.024 |
| LINK | 197.081 | 11.0659 | 10.6685 | $-78 | +0.021 |
| WLD | -6204.89 | 0.360162 | 0.32094 | $243 | -0.020 |
| APT | -3237.04 | 0.626089 | 0.5886 | $121 | -0.019 |
| JUP | -7068.37 | 0.253165 | 0.255305 | $-15 | -0.018 |
| XRP | 1596.65 | 1.27041 | 1.09245 | $-284 | +0.017 |
| TIA | -3686.1 | 0.421737 | 0.43755 | $-58 | -0.016 |
| DOT | -1684.82 | 0.940458 | 0.86751 | $123 | -0.015 |
| AAVE | 8.98333 | 131.06 | 133.175 | $19 | +0.012 |
| ENA | 7483.97 | 0.179237 | 0.155505 | $-178 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.067148 | $58 | -0.011 |
| ETC | -159.905 | 7.02745 | 6.8828 | $23 | -0.011 |
| ALGO | -8833.48 | 0.0922081 | 0.092536 | $-3 | -0.008 |
| XLM | -5134.89 | 0.145831 | 0.147155 | $-7 | -0.008 |
| FIL | -873.911 | 0.86441 | 0.77843 | $75 | -0.007 |
| NEAR | -155.553 | 3.72366 | 3.61385 | $17 | -0.006 |
| RENDER | -323.133 | 1.55071 | 1.62345 | $-24 | -0.005 |
| BTC | 0.00601366 | 75900.3 | 74878.5 | $-6 | +0.004 |
| DOGE | -4200.09 | 0.0715214 | 0.069934 | $7 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.376075 | $-11 | +0.002 |
| LTC | -3.23093 | 54.9478 | 51.5915 | $11 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.84982 | $-2 | +0.001 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:PURRDAT | 1221.16 | 8.42623 | 8.93045 | $616 | +0.108 |
| xyz:ANT | -103.812 | 97.1561 | 103.705 | $-680 | -0.106 |
| xyz:SOXL | 167.162 | 44.1174 | 55.8905 | $1,968 | +0.092 |
| xyz:CXMT | -1040.58 | 6.06853 | 6.38305 | $-327 | -0.066 |
| xyz:MRNA | -20.6534 | 257.083 | 275.75 | $-386 | -0.056 |
| xyz:MSTR | 44.1296 | 125.03 | 127.6 | $113 | +0.056 |
| xyz:NVDA | 26.5274 | 181.404 | 177.175 | $-112 | +0.046 |
| xyz:NBIS | 29.7854 | 126.39 | 141.425 | $448 | +0.042 |
| xyz:SPCX | -29.5825 | 135.197 | 141.495 | $-186 | -0.041 |
| xyz:CRCL | 58.3911 | 66.157 | 68.065 | $111 | +0.039 |
| xyz:COIN | 27.7201 | 131.556 | 137.8 | $173 | +0.038 |
| xyz:ZHIPU | -65.6359 | 53.4657 | 56.594 | $-205 | -0.037 |
| xyz:META | -5.46302 | 612.795 | 644.68 | $-174 | -0.035 |
| xyz:SNDK | 3.28581 | 818.76 | 886.765 | $223 | +0.029 |
| xyz:AMD | -4.50186 | 446.974 | 509.58 | $-282 | -0.023 |
| xyz:UNITREE | -20.5421 | 97.5698 | 105.52 | $-163 | -0.021 |
| xyz:SP500 | -0.270937 | 7369.48 | 7563.35 | $-53 | -0.020 |
| xyz:HOOD | 21.0722 | 79.6781 | 84.675 | $105 | +0.018 |
| xyz:LITE | 2.92587 | 531.045 | 596.785 | $192 | +0.017 |
| xyz:AMZN | -6.46957 | 258.646 | 268.755 | $-65 | -0.017 |
| xyz:MRVL | 10.7997 | 139.037 | 153.51 | $156 | +0.016 |
| xyz:AAPL | -4.23108 | 341.125 | 351.425 | $-44 | -0.015 |
| xyz:EWY | -10.0344 | 136.458 | 142.14 | $-57 | -0.014 |
| xyz:XYZ100 | -0.0509435 | 26875 | 27922.5 | $-53 | -0.014 |
| xyz:STRC | -13.4941 | 99.2653 | 99.9085 | $-9 | -0.013 |
| xyz:TSLA | -3.34248 | 356.502 | 380.735 | $-81 | -0.013 |
| xyz:MSFT | -2.09388 | 528.044 | 546.885 | $-39 | -0.011 |
| xyz:INTC | 15.7998 | 55.5993 | 58.803 | $51 | +0.009 |
| xyz:BABA | -8.10903 | 108.915 | 105.54 | $27 | -0.008 |
| xyz:SKHY | 6.31123 | 125.6 | 133.6 | $50 | +0.008 |
| xyz:SKHX | 0.889091 | 849.39 | 902.15 | $47 | +0.008 |
| xyz:MU | -0.788268 | 685.225 | 730.5 | $-36 | -0.006 |
| xyz:SMSN | -3.73398 | 141.085 | 150.215 | $-34 | -0.006 |
| xyz:DRAM | -8.8279 | 38.9025 | 41.7145 | $-25 | -0.004 |
| xyz:KORU | 30.5503 | 6.85365 | 7.7013 | $26 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-08 08:10: hold BTC 0.25, ETH 0. BTC range-bound 75-76k for 48h (no rule hit). Fear & Greed 26-37 (fear, contrarian support) vs persistent put skew (BTC 7d/30d RR25 -4.9/-4.2, ETH -6.0) and the aftermath of the record $6.3B Binance hack (CFO resigned, staged withdrawal reopenings, hackers swapping loot into BTC) as tail risk. No edge to add or cut. Rules unchanged: BTC < 72k -> cut to 0.10; BTC reclaims 78k -> back to 0.30."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -3.8, "AVAX": -4.82, "ONDO": -3.32, "ZEC": -4.29, "kPEPE": -3.57}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -3.8, "AVAX": -4.82, "ONDO": -3.32, "ZEC": -4.29, "kPEPE": -3.57}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 10.0, "HYPE": 11.6, "PENDLE": 10.4, "UNI": 11.0, "ZEC": 12.6}, "last_day": "2026-11-09", "top_apr": {"BTC": 10.3, "ENA": 10.0, "ETH": 10.0, "HBAR": 10.9, "HYPE": 11.6, "LDO": 11.0, "PENDLE": 10.4, "RENDER": 11.0, "UNI": 11.0, "ZEC": 12.6}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-09"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-09T00:04 | dn_funding_carry | PENDLE/spot | buy | $2,559 | 1.55528 | daily dn-carry |
| 2026-11-09T00:04 | dn_funding_carry | HYPE/spot | buy | $2,274 | 74.3952 | daily dn-carry |
| 2026-11-09T00:04 | dn_funding_carry | HYPE | sell | $2,293 | 74.346 | daily dn-carry |
| 2026-11-09T00:04 | dn_funding_carry | PENDLE | sell | $2,593 | 1.55175 | daily dn-carry |
| 2026-11-09T00:04 | xs_momentum | TRX | buy | $1,599 | 0.319231 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | ALGO | buy | $1,528 | 0.0909001 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | RENDER | buy | $1,046 | 1.523 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | SOL | sell | $1,027 | 86.2692 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | TIA | sell | $1,144 | 0.411019 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | BCH | sell | $1,392 | 238.446 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | XLM | sell | $1,494 | 0.145831 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_momentum | PENDLE | sell | $1,537 | 1.55193 | signal 2026-11-08 23h |
| 2026-11-09T00:04 | xs_funding | BTC | buy | $9,452 | 74605 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | HBAR | buy | $6,031 | 0.0652853 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | BCH | buy | $5,590 | 238.546 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | APT | buy | $4,169 | 0.569334 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | UNI | buy | $3,354 | 6.75074 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | LINK | sell | $1,068 | 10.3725 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | ETC | sell | $1,194 | 6.62497 | daily xs-funding (Binance recon) |
| 2026-11-09T00:04 | xs_funding | XRP | sell | $1,464 | 1.10689 | daily xs-funding (Binance recon) |
