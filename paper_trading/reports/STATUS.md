# Paper trading status — 2026-11-08 23:04 UTC

BTC 74,734 · ETH 2,135.6 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $96,435 | -3.56% | -1.30% | -4.07% | -7.56% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $98,229 | -1.77% | -0.34% | -1.99% | -3.09% | nan | 0.25 | +0.25 | 1 | $43 | $-75 |
| crash_rebound | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,202 | +0.20% | +0.07% | +0.27% | -0.19% | nan | 1.36 | +0.00 | 10 | $215 | $221 |
| trend_breakout_majors | $101,475 | +1.48% | +1.73% | +1.33% | -5.48% | nan | 0.56 | -0.56 | 3 | $96 | $121 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | -0.05% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | -0.16% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $99,346 | -0.65% | -0.33% | -0.25% | -1.13% | nan | 0.96 | +0.08 | 20 | $378 | $76 |
| xs_momentum | $100,913 | +0.91% | +0.54% | +0.91% | -0.78% | nan | 0.81 | +0.02 | 38 | $128 | $-19 |
| xyz_weekend_fade | $99,688 | -0.31% | -0.31% | -0.31% | -0.31% | nan | 1.00 | -0.01 | 35 | $312 | $0 |

**Combined (ex-benchmark): $1,100,087 on $1,100,000 → +0.01%**
Combined BTC exposure (7d realised, 4h returns): beta -0.049 (≈ -0.54 books of BTC), corr -0.73

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 74782 | $-3,495 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.325465 | 77535.3 | 74734.5 | $-912 | +0.248 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| UNI/spot | 2136.3 | 7.02501 | 6.8401 | $-395 | +0.146 |
| UNI | -2133.42 | 7.03009 | 6.83405 | $418 | -0.146 |
| ZEC/spot | 11.6568 | 1287.44 | 1212.6 | $-872 | +0.141 |
| ZEC | -11.65 | 1287.49 | 1210 | $903 | -0.141 |
| ENA/spot | 93002.9 | 0.188053 | 0.149785 | $-3,559 | +0.139 |
| ENA | -93083.1 | 0.187964 | 0.149565 | $3,574 | -0.139 |
| HYPE/spot | 171.426 | 87.5453 | 74.7231 | $-2,198 | +0.128 |
| HYPE | -171.177 | 87.6241 | 74.6335 | $2,224 | -0.127 |
| PENDLE/spot | 8017.1 | 1.87289 | 1.564 | $-2,476 | +0.125 |
| PENDLE | -8000.21 | 1.87311 | 1.561 | $2,497 | -0.125 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -251.977 | 94.3829 | 86.6955 | $1,937 | -0.215 |
| ETH | -8.50484 | 2246.43 | 2135.55 | $943 | -0.179 |
| BTC | -0.229861 | 76485.5 | 74734.5 | $402 | -0.169 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.126699 | 77678.6 | 74734.5 | $373 | -0.095 |
| TRX | 28470.1 | 0.309116 | 0.319225 | $288 | +0.091 |
| ETH | 4.0133 | 2258.26 | 2135.55 | $-492 | +0.086 |
| XLM | 44454.5 | 0.151093 | 0.14666 | $-197 | +0.066 |
| XRP | -4952.8 | 1.24999 | 1.11535 | $667 | -0.056 |
| ETC | -757.201 | 6.94129 | 6.6428 | $226 | -0.051 |
| ATOM | 3529.46 | 1.49903 | 1.3034 | $-690 | +0.046 |
| SEI | 104421 | 0.0484475 | 0.043047 | $-564 | +0.045 |
| LINK | -413.69 | 11.6357 | 10.4355 | $497 | -0.043 |
| TAO | -19.9244 | 222.943 | 216.125 | $136 | -0.043 |
| JUP | 18056.4 | 0.241363 | 0.232455 | $-161 | +0.042 |
| PENDLE | 2625.06 | 1.63928 | 1.561 | $-206 | +0.041 |
| kPEPE | 801611 | 0.00503502 | 0.004961 | $-59 | +0.040 |
| LDO | -10898 | 0.325861 | 0.31251 | $146 | -0.034 |
| UNI | -496.869 | 7.10867 | 6.83405 | $136 | -0.034 |
| WLD | 10593 | 0.336516 | 0.31259 | $-253 | +0.033 |
| RENDER | -2079.13 | 1.56364 | 1.52565 | $79 | -0.032 |
| TIA | 6583.75 | 0.474572 | 0.41121 | $-417 | +0.027 |
| ENA | -17260.6 | 0.163688 | 0.149565 | $244 | -0.026 |
| NEAR | -660.139 | 3.86026 | 3.5806 | $185 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -75.3595 | 92.8855 | 86.6955 | $466 | -0.065 |
| BCH | 17.2763 | 277.3 | 239.64 | $-651 | +0.041 |
| UNI | 597.681 | 7.08573 | 6.83405 | $-150 | +0.040 |
| TRX | 12738.3 | 0.316337 | 0.319225 | $37 | +0.040 |
| ATOM | -3097.68 | 1.43333 | 1.3034 | $402 | -0.040 |
| kPEPE | 803792 | 0.00484647 | 0.004961 | $92 | +0.040 |
| ADA | -22351.2 | 0.190675 | 0.175515 | $339 | -0.039 |
| ZEC | 3.22979 | 1213.4 | 1210 | $-11 | +0.039 |
| BNB | 5.68814 | 705.088 | 685.025 | $-114 | +0.039 |
| SEI | -87205.6 | 0.0472943 | 0.043047 | $370 | -0.037 |
| ETH | 1.45521 | 2238.8 | 2135.55 | $-150 | +0.031 |
| CRV | -11195.5 | 0.271234 | 0.25514 | $180 | -0.028 |
| TAO | -11.3929 | 219.804 | 216.125 | $42 | -0.024 |
| OP | 24123.2 | 0.107142 | 0.09764 | $-229 | +0.023 |
| ALGO | -25645.7 | 0.0922081 | 0.090828 | $35 | -0.023 |
| ARB | 14157.1 | 0.162612 | 0.16453 | $27 | +0.023 |
| INJ | 290.053 | 7.16191 | 7.9835 | $238 | +0.023 |
| LINK | 197.081 | 11.0659 | 10.4355 | $-124 | +0.020 |
| WLD | -6204.89 | 0.360162 | 0.31259 | $295 | -0.019 |
| APT | -3237.04 | 0.626089 | 0.5715 | $177 | -0.018 |
| XRP | 1596.65 | 1.27041 | 1.11535 | $-248 | +0.018 |
| JUP | -7068.37 | 0.253165 | 0.232455 | $146 | -0.016 |
| RENDER | -1009.69 | 1.55071 | 1.52565 | $25 | -0.015 |
| DOT | -1684.82 | 0.940458 | 0.86445 | $128 | -0.014 |
| PENDLE | -857.706 | 1.66513 | 1.561 | $89 | -0.013 |
| AAVE | 8.98333 | 131.06 | 126.495 | $-41 | +0.011 |
| ENA | 7483.97 | 0.179237 | 0.149565 | $-222 | +0.011 |
| HBAR | -16510.2 | 0.0706575 | 0.065553 | $84 | -0.011 |
| ETC | -159.905 | 7.02745 | 6.6428 | $62 | -0.011 |
| XLM | 5109.4 | 0.162127 | 0.14666 | $-79 | +0.007 |
| FIL | -873.911 | 0.86441 | 0.74206 | $107 | -0.006 |
| NEAR | -155.553 | 3.72366 | 3.5806 | $22 | -0.006 |
| BTC | 0.00601366 | 75900.3 | 74734.5 | $-7 | +0.004 |
| TIA | -902.127 | 0.454813 | 0.41121 | $39 | -0.004 |
| DOGE | -4200.09 | 0.0715214 | 0.069356 | $9 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.352555 | $-26 | +0.002 |
| LTC | -3.23093 | 54.9478 | 50.3395 | $15 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.82353 | $-4 | +0.001 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:ANT | -103.812 | 97.1561 | 98.1375 | $-102 | -0.102 |
| xyz:PURRDAT | 1221.16 | 8.42623 | 8.3428 | $-102 | +0.102 |
| xyz:SOXL | 167.162 | 44.1174 | 44.0995 | $-3 | +0.074 |
| xyz:CXMT | -1040.58 | 6.06853 | 6.0716 | $-3 | -0.063 |
| xyz:MSTR | 44.1296 | 125.03 | 125.01 | $-1 | +0.055 |
| xyz:MRNA | -20.6534 | 257.083 | 257.19 | $-2 | -0.053 |
| xyz:NVDA | 26.5274 | 181.404 | 181.375 | $-1 | +0.048 |
| xyz:SPCX | -29.5825 | 135.197 | 135.205 | $-0 | -0.040 |
| xyz:CRCL | 58.3911 | 66.157 | 66.148 | $-1 | +0.039 |
| xyz:NBIS | 29.7854 | 126.39 | 126.33 | $-2 | +0.038 |
| xyz:COIN | 27.7201 | 131.556 | 131.545 | $-0 | +0.037 |
| xyz:ZHIPU | -65.6359 | 53.4657 | 53.49 | $-2 | -0.035 |
| xyz:META | -5.46302 | 612.795 | 612.85 | $-0 | -0.034 |
| xyz:SNDK | 3.28581 | 818.76 | 818.665 | $-0 | +0.027 |
| xyz:AMD | -4.50186 | 446.974 | 447.03 | $-0 | -0.020 |
| xyz:UNITREE | -20.5421 | 97.5698 | 97.5865 | $-0 | -0.020 |
| xyz:SP500 | -0.270937 | 7369.48 | 7369.85 | $-0 | -0.020 |
| xyz:HOOD | 21.0722 | 79.6781 | 79.673 | $-0 | +0.017 |
| xyz:AMZN | -6.46957 | 258.646 | 258.67 | $-0 | -0.017 |
| xyz:LITE | 2.92587 | 531.045 | 530.855 | $-1 | +0.016 |
| xyz:MRVL | 10.7997 | 139.037 | 139.03 | $-0 | +0.015 |
| xyz:AAPL | -4.23108 | 341.125 | 341.195 | $-0 | -0.014 |
| xyz:XYZ100 | -0.0509435 | 26875 | 26880.5 | $-0 | -0.014 |
| xyz:EWY | -10.0344 | 136.458 | 136.465 | $-0 | -0.014 |
| xyz:STRC | -13.4941 | 99.2653 | 99.307 | $-1 | -0.013 |
| xyz:TSLA | -3.34248 | 356.502 | 356.52 | $-0 | -0.012 |
| xyz:MSFT | -2.09388 | 528.044 | 528.07 | $-0 | -0.011 |
| xyz:BABA | -8.10903 | 108.915 | 108.93 | $-0 | -0.009 |
| xyz:INTC | 15.7998 | 55.5993 | 55.5965 | $-0 | +0.009 |
| xyz:SKHY | 6.31123 | 125.6 | 125.585 | $-0 | +0.008 |
| xyz:SKHX | 0.889091 | 849.39 | 849.14 | $-0 | +0.008 |
| xyz:MU | -0.788268 | 685.225 | 685.265 | $-0 | -0.005 |
| xyz:SMSN | -3.73398 | 141.085 | 141.105 | $-0 | -0.005 |
| xyz:DRAM | -8.8279 | 38.9025 | 38.9105 | $-0 | -0.003 |
| xyz:KORU | 30.5503 | 6.85365 | 6.85185 | $-0 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-08 08:10: hold BTC 0.25, ETH 0. BTC range-bound 75-76k for 48h (no rule hit). Fear & Greed 26-37 (fear, contrarian support) vs persistent put skew (BTC 7d/30d RR25 -4.9/-4.2, ETH -6.0) and the aftermath of the record $6.3B Binance hack (CFO resigned, staged withdrawal reopenings, hackers swapping loot into BTC) as tail risk. No edge to add or cut. Rules unchanged: BTC < 72k -> cut to 0.10; BTC reclaims 78k -> back to 0.30."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ETH": -4.81, "SEI": -4.48, "TAO": -4.4, "WLD": -4.6, "ZEC": -4.98}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ETH": -4.81, "SEI": -4.48, "TAO": -4.4, "WLD": -4.6, "ZEC": -4.98}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 10.8, "HYPE": 13.3, "PENDLE": 11.8, "UNI": 13.6, "ZEC": 12.9}, "last_day": "2026-11-08", "top_apr": {"ENA": 10.8, "ETH": 10.5, "HBAR": 10.9, "HYPE": 13.3, "LDO": 11.0, "PENDLE": 11.8, "RENDER": 11.0, "TAO": 10.4, "UNI": 13.6, "ZEC": 12.9}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-07T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-07T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-08"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-07T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:PURRDAT | buy | $10,290 | 8.42623 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:SOXL | buy | $7,375 | 44.1174 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:MSTR | buy | $5,518 | 125.03 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:NVDA | buy | $4,812 | 181.404 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:CRCL | buy | $3,863 | 66.157 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:NBIS | buy | $3,765 | 126.39 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:COIN | buy | $3,647 | 131.556 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:SNDK | buy | $2,690 | 818.76 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:HOOD | buy | $1,679 | 79.6781 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:LITE | buy | $1,554 | 531.045 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:MRVL | buy | $1,502 | 139.037 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:INTC | buy | $878 | 55.5993 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:SKHY | buy | $793 | 125.6 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:SKHX | buy | $755 | 849.39 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:KORU | buy | $209 | 6.85365 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:DRAM | sell | $343 | 38.9025 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:SMSN | sell | $527 | 141.085 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:MU | sell | $540 | 685.225 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:BABA | sell | $883 | 108.915 | weekend fade entry |
| 2026-11-08T23:04 | xyz_weekend_fade | xyz:MSFT | sell | $1,106 | 528.044 | weekend fade entry |
