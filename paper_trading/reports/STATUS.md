# Paper trading status — 2026-11-09 13:07 UTC

BTC 74,858 · ETH 2,123.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $96,564 | -3.44% | -1.46% | -3.86% | -8.59% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $98,265 | -1.73% | -0.37% | -1.93% | -3.37% | nan | 0.25 | +0.25 | 1 | $43 | $-79 |
| crash_rebound | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,099 | +0.10% | -0.03% | +0.09% | -0.19% | nan | 1.47 | +0.00 | 10 | $225 | $231 |
| trend_breakout_majors | $101,565 | +1.56% | +2.04% | +1.69% | -5.48% | nan | 0.59 | -0.59 | 3 | $97 | $120 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | -0.03% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $99,575 | -0.43% | -0.12% | +0.10% | -1.13% | nan | 0.97 | +0.07 | 20 | $418 | $82 |
| xs_momentum | $100,583 | +0.58% | -0.01% | +0.73% | -0.78% | nan | 0.84 | -0.01 | 38 | $135 | $-21 |
| xyz_weekend_fade | $99,875 | -0.12% | -0.12% | -0.12% | -1.21% | nan | 1.02 | -0.01 | 35 | $312 | $-3 |

**Combined (ex-benchmark): $1,100,196 on $1,100,000 → +0.02%**
Combined BTC exposure (7d realised, 4h returns): beta -0.045 (≈ -0.49 books of BTC), corr -0.71

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 74882 | $-3,366 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.325465 | 77535.3 | 74858.5 | $-871 | +0.248 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.613 | $1,979 | -0.156 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.61315 | $-1,987 | +0.156 |
| HYPE/spot | 201.997 | 85.5551 | 73.715 | $-2,392 | +0.149 |
| HYPE | -202.014 | 85.5972 | 73.6825 | $2,407 | -0.149 |
| UNI/spot | 2136.3 | 7.02501 | 6.89709 | $-273 | +0.147 |
| UNI | -2133.42 | 7.03009 | 6.89615 | $286 | -0.147 |
| ENA | -93083.1 | 0.187964 | 0.15377 | $3,183 | -0.143 |
| ENA/spot | 93002.9 | 0.188053 | 0.153817 | $-3,184 | +0.143 |
| ZEC | -11.65 | 1287.49 | 1221.15 | $773 | -0.142 |
| ZEC/spot | 11.6568 | 1287.44 | 1220.2 | $-784 | +0.142 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -251.977 | 94.3829 | 86.5945 | $1,962 | -0.215 |
| BTC | -0.271518 | 76194.7 | 74858.5 | $363 | -0.200 |
| ETH | -8.50484 | 2246.43 | 2123.15 | $1,049 | -0.178 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.317635 | $243 | +0.091 |
| XRP | -6275.1 | 1.21983 | 1.10225 | $738 | -0.069 |
| XLM | 44454.5 | 0.151093 | 0.14741 | $-164 | +0.066 |
| ETC | -937.414 | 6.88048 | 6.7904 | $84 | -0.064 |
| HBAR | 92374.3 | 0.0652853 | 0.068716 | $317 | +0.064 |
| LTC | -121.169 | 49.941 | 51.512 | $-190 | -0.063 |
| BCH | 23.4344 | 238.546 | 244.005 | $128 | +0.057 |
| LINK | -516.66 | 11.3839 | 10.5975 | $406 | -0.055 |
| ATOM | 3529.46 | 1.49903 | 1.369 | $-459 | +0.049 |
| SEI | 104421 | 0.0484475 | 0.044361 | $-427 | +0.047 |
| TAO | -19.9244 | 222.943 | 222.43 | $10 | -0.045 |
| JUP | 18056.4 | 0.241363 | 0.23942 | $-35 | +0.043 |
| APT | 7323.07 | 0.569334 | 0.589 | $144 | +0.043 |
| LDO | -10898 | 0.325861 | 0.32787 | $-22 | -0.036 |
| WLD | 10593 | 0.336516 | 0.32155 | $-159 | +0.034 |
| RENDER | -2079.13 | 1.56364 | 1.6074 | $-91 | -0.034 |
| kPEPE | -646727 | 0.00494453 | 0.005148 | $-132 | -0.033 |
| TIA | 6583.75 | 0.474572 | 0.43026 | $-292 | +0.028 |
| ENA | -17260.6 | 0.163688 | 0.15377 | $171 | -0.027 |
| NEAR | -660.139 | 3.86026 | 3.54525 | $208 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 86.5945 | $470 | -0.075 |
| TRX | 17747.4 | 0.317154 | 0.317635 | $9 | +0.056 |
| ATOM | -3097.68 | 1.43333 | 1.369 | $199 | -0.042 |
| kPEPE | 803792 | 0.00484647 | 0.005148 | $242 | +0.041 |
| UNI | 597.681 | 7.08573 | 6.89615 | $-113 | +0.041 |
| ADA | -22351.2 | 0.190675 | 0.178685 | $268 | -0.040 |
| ZEC | 3.22979 | 1213.4 | 1221.15 | $25 | +0.039 |
| BNB | 5.68814 | 705.088 | 685.995 | $-109 | +0.039 |
| SEI | -87205.6 | 0.0472943 | 0.044361 | $256 | -0.038 |
| ETH | 1.45521 | 2238.8 | 2123.15 | $-168 | +0.031 |
| CRV | -11195.5 | 0.271234 | 0.26797 | $37 | -0.030 |
| PENDLE | -1848.04 | 1.60447 | 1.613 | $-16 | -0.030 |
| BCH | 11.4397 | 277.3 | 244.005 | $-381 | +0.028 |
| ARB | 14157.1 | 0.162612 | 0.18645 | $337 | +0.026 |
| TAO | -11.3929 | 219.804 | 222.43 | $-30 | -0.025 |
| OP | 24123.2 | 0.107142 | 0.10154 | $-135 | +0.024 |
| INJ | 290.053 | 7.16191 | 8.1838 | $296 | +0.024 |
| LINK | 197.081 | 11.0659 | 10.5975 | $-92 | +0.021 |
| WLD | -6204.89 | 0.360162 | 0.32155 | $240 | -0.020 |
| APT | -3237.04 | 0.626089 | 0.589 | $120 | -0.019 |
| XRP | 1596.65 | 1.27041 | 1.10225 | $-268 | +0.017 |
| JUP | -7068.37 | 0.253165 | 0.23942 | $97 | -0.017 |
| TIA | -3686.1 | 0.421737 | 0.43026 | $-31 | -0.016 |
| DOT | -1684.82 | 0.940458 | 0.87882 | $104 | -0.015 |
| AAVE | 8.98333 | 131.06 | 133.435 | $21 | +0.012 |
| ENA | 7483.97 | 0.179237 | 0.15377 | $-191 | +0.011 |
| HBAR | -16510.2 | 0.0706575 | 0.068716 | $32 | -0.011 |
| ETC | -159.905 | 7.02745 | 6.7904 | $38 | -0.011 |
| ALGO | -8833.48 | 0.0922081 | 0.09307 | $-8 | -0.008 |
| XLM | -5134.89 | 0.145831 | 0.14741 | $-8 | -0.008 |
| FIL | -873.911 | 0.86441 | 0.77167 | $81 | -0.007 |
| NEAR | -155.553 | 3.72366 | 3.54525 | $28 | -0.005 |
| RENDER | -323.133 | 1.55071 | 1.6074 | $-18 | -0.005 |
| BTC | 0.00601366 | 75900.3 | 74858.5 | $-6 | +0.004 |
| DOGE | -4200.09 | 0.0715214 | 0.069704 | $8 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.357785 | $-23 | +0.002 |
| LTC | -3.23093 | 54.9478 | 51.512 | $11 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.8448 | $-3 | +0.001 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:PURRDAT | 1221.16 | 8.42623 | 8.51785 | $112 | +0.104 |
| xyz:ANT | -103.812 | 97.1561 | 99.878 | $-283 | -0.104 |
| xyz:SOXL | 167.162 | 44.1174 | 47.9165 | $635 | +0.080 |
| xyz:CXMT | -1040.58 | 6.06853 | 6.10895 | $-42 | -0.064 |
| xyz:MSTR | 44.1296 | 125.03 | 125.985 | $42 | +0.056 |
| xyz:MRNA | -20.6534 | 257.083 | 258.5 | $-29 | -0.053 |
| xyz:NVDA | 26.5274 | 181.404 | 177.325 | $-108 | +0.047 |
| xyz:SPCX | -29.5825 | 135.197 | 137.895 | $-80 | -0.041 |
| xyz:CRCL | 58.3911 | 66.157 | 66.283 | $7 | +0.039 |
| xyz:NBIS | 29.7854 | 126.39 | 128.63 | $67 | +0.038 |
| xyz:COIN | 27.7201 | 131.556 | 133.155 | $44 | +0.037 |
| xyz:ZHIPU | -65.6359 | 53.4657 | 55.087 | $-106 | -0.036 |
| xyz:META | -5.46302 | 612.795 | 626.825 | $-77 | -0.034 |
| xyz:SNDK | 3.28581 | 818.76 | 839.12 | $67 | +0.028 |
| xyz:UNITREE | -20.5421 | 97.5698 | 105.25 | $-158 | -0.022 |
| xyz:AMD | -4.50186 | 446.974 | 468.095 | $-95 | -0.021 |
| xyz:SP500 | -0.270937 | 7369.48 | 7446.55 | $-21 | -0.020 |
| xyz:AMZN | -6.46957 | 258.646 | 267.26 | $-56 | -0.017 |
| xyz:HOOD | 21.0722 | 79.6781 | 81.054 | $29 | +0.017 |
| xyz:LITE | 2.92587 | 531.045 | 545.94 | $44 | +0.016 |
| xyz:MRVL | 10.7997 | 139.037 | 142.965 | $42 | +0.015 |
| xyz:AAPL | -4.23108 | 341.125 | 343.935 | $-12 | -0.015 |
| xyz:XYZ100 | -0.0509435 | 26875 | 27233.5 | $-18 | -0.014 |
| xyz:EWY | -10.0344 | 136.458 | 136.545 | $-1 | -0.014 |
| xyz:STRC | -13.4941 | 99.2653 | 99.639 | $-5 | -0.013 |
| xyz:TSLA | -3.34248 | 356.502 | 360.99 | $-15 | -0.012 |
| xyz:MSFT | -2.09388 | 528.044 | 542.305 | $-30 | -0.011 |
| xyz:INTC | 15.7998 | 55.5993 | 56.8065 | $19 | +0.009 |
| xyz:BABA | -8.10903 | 108.915 | 107.785 | $9 | -0.009 |
| xyz:SKHY | 6.31123 | 125.6 | 126.345 | $5 | +0.008 |
| xyz:SKHX | 0.889091 | 849.39 | 853.625 | $4 | +0.008 |
| xyz:MU | -0.788268 | 685.225 | 699.365 | $-11 | -0.006 |
| xyz:SMSN | -3.73398 | 141.085 | 142.54 | $-5 | -0.005 |
| xyz:DRAM | -8.8279 | 38.9025 | 39.6375 | $-6 | -0.004 |
| xyz:KORU | 30.5503 | 6.85365 | 6.85905 | $0 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-08 08:10: hold BTC 0.25, ETH 0. BTC range-bound 75-76k for 48h (no rule hit). Fear & Greed 26-37 (fear, contrarian support) vs persistent put skew (BTC 7d/30d RR25 -4.9/-4.2, ETH -6.0) and the aftermath of the record $6.3B Binance hack (CFO resigned, staged withdrawal reopenings, hackers swapping loot into BTC) as tail risk. No edge to add or cut. Rules unchanged: BTC < 72k -> cut to 0.10; BTC reclaims 78k -> back to 0.30."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ETH": -0.04, "SOL": -0.4, "TRX": 0.25, "ZEC": -0.48, "kPEPE": -0.97}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ETH": -0.04, "SOL": -0.4, "TRX": 0.25, "ZEC": -0.48, "kPEPE": -0.97}}`
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
