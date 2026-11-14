# Paper trading status — 2026-11-14 08:04 UTC

BTC 67,188 · ETH 1,898.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $86,655 | -13.35% | -2.84% | -11.71% | -17.33% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,246 | -2.75% | -0.13% | -1.45% | -4.06% | nan | 0.05 | +0.05 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,167 | +0.17% | -0.01% | +0.04% | -0.19% | nan | 1.39 | -0.00 | 10 | $238 | $304 |
| trend_breakout_majors | $110,813 | +10.81% | +2.86% | +11.51% | -5.48% | nan | 0.69 | -0.69 | 3 | $110 | $166 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,245 | +0.25% | -0.47% | +0.59% | -1.56% | nan | 0.96 | +0.02 | 20 | $539 | $111 |
| xs_momentum | $99,134 | -0.87% | +0.79% | -1.45% | -2.98% | nan | 0.83 | -0.03 | 38 | $176 | $-9 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,108,932 on $1,100,000 → +0.81%**
Combined BTC exposure (7d realised, 4h returns): beta -0.050 (≈ -0.55 books of BTC), corr -0.75

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 67207 | $-13,275 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 67187.5 | $-694 | +0.046 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.5086 | $2,988 | -0.146 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.5085 | $-2,998 | +0.146 |
| ENA | -111613 | 0.179105 | 0.127325 | $5,779 | -0.142 |
| ENA/spot | 111538 | 0.1792 | 0.1274 | $-5,778 | +0.142 |
| HYPE/spot | 201.997 | 85.5551 | 69.654 | $-3,212 | +0.140 |
| HYPE | -202.014 | 85.5972 | 69.6375 | $3,224 | -0.140 |
| UNI | -2471.47 | 6.90012 | 5.4953 | $3,472 | -0.136 |
| UNI/spot | 2468.37 | 6.89946 | 5.497 | $-3,462 | +0.135 |
| ZEC | -14.3511 | 1242.13 | 909.795 | $4,769 | -0.130 |
| ZEC/spot | 14.3484 | 1242.39 | 909.75 | $-4,773 | +0.130 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 75.2415 | $5,849 | -0.249 |
| ETH | -12.8804 | 2187.58 | 1898.15 | $3,728 | -0.221 |
| BTC | -0.356716 | 75630.7 | 67187.5 | $3,012 | -0.216 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 67187.5 | $165 | -0.097 |
| TRX | 28470.1 | 0.309116 | 0.312885 | $107 | +0.089 |
| XLM | 44454.5 | 0.151093 | 0.142535 | $-380 | +0.063 |
| XRP | -6275.1 | 1.21983 | 0.983375 | $1,484 | -0.062 |
| HBAR | 92374.3 | 0.0652853 | 0.059518 | $-533 | +0.055 |
| TAO | -25.0244 | 220.686 | 206.92 | $344 | -0.052 |
| BCH | 23.4344 | 238.546 | 213.595 | $-585 | +0.050 |
| LINK | -516.66 | 11.3839 | 9.56315 | $941 | -0.049 |
| ATOM | 3529.46 | 1.49903 | 1.3227 | $-622 | +0.047 |
| JUP | 18056.4 | 0.241363 | 0.255955 | $263 | +0.046 |
| LDO | -14834.4 | 0.308439 | 0.29099 | $259 | -0.043 |
| RENDER | -2753.67 | 1.56167 | 1.48805 | $203 | -0.041 |
| OP | 44197.2 | 0.0926854 | 0.08852 | $-184 | +0.039 |
| PENDLE | -2505.4 | 1.58072 | 1.5086 | $181 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13843 | $-353 | +0.038 |
| FIL | 5349.98 | 0.760721 | 0.68 | $-432 | +0.036 |
| NEAR | -997.435 | 3.77006 | 3.31635 | $453 | -0.033 |
| TIA | -8196.2 | 0.382952 | 0.36724 | $129 | -0.030 |
| WLD | 10593 | 0.336516 | 0.28026 | $-596 | +0.030 |
| ENA | -22442.6 | 0.146112 | 0.127325 | $422 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 75.2415 | $1,461 | -0.066 |
| TRX | 20882.3 | 0.317091 | 0.312885 | $-88 | +0.066 |
| XRP | -4826.67 | 1.05611 | 0.983375 | $351 | -0.048 |
| PENDLE | -2522.74 | 1.58296 | 1.5086 | $188 | -0.038 |
| RENDER | 2461.25 | 1.58063 | 1.48805 | $-228 | +0.037 |
| INJ | 456.574 | 7.51016 | 8.00405 | $225 | +0.037 |
| kPEPE | 803792 | 0.00484647 | 0.004272 | $-462 | +0.035 |
| UNI | 597.681 | 7.08573 | 5.4953 | $-951 | +0.033 |
| AVAX | 376.81 | 8.37846 | 8.0105 | $-139 | +0.030 |
| ARB | 21235.8 | 0.1621 | 0.13843 | $-503 | +0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.06253 | $190 | -0.030 |
| WLD | -9976.36 | 0.34434 | 0.28026 | $639 | -0.028 |
| ADA | -16388.2 | 0.190675 | 0.162535 | $461 | -0.027 |
| JUP | 10388.9 | 0.261398 | 0.255955 | $-57 | +0.027 |
| CRV | -11195.5 | 0.271234 | 0.22626 | $503 | -0.026 |
| DOT | -3218.79 | 0.900177 | 0.78167 | $381 | -0.025 |
| SEI | -60987.3 | 0.0472943 | 0.040912 | $389 | -0.025 |
| BTC | -0.0355851 | 70094.1 | 67187.5 | $103 | -0.024 |
| XLM | -16417.2 | 0.145302 | 0.142535 | $45 | -0.024 |
| BNB | 3.50367 | 705.088 | 615.88 | $-313 | +0.022 |
| OP | 24123.2 | 0.107142 | 0.08852 | $-449 | +0.022 |
| AAVE | 18.1796 | 131.483 | 107.815 | $-430 | +0.020 |
| ZEC | 1.94807 | 1213.4 | 909.795 | $-591 | +0.018 |
| FIL | -2342.25 | 0.799282 | 0.68 | $279 | -0.016 |
| BCH | -6.47026 | 225.866 | 213.595 | $79 | -0.014 |
| HBAR | -16510.2 | 0.0706575 | 0.059518 | $184 | -0.010 |
| ETH | -0.43933 | 1931.65 | 1898.15 | $15 | -0.008 |
| LINK | 77.5637 | 11.0659 | 9.56315 | $-117 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.5118 | $158 | -0.007 |
| TAO | 2.54822 | 211.979 | 206.92 | $-13 | +0.005 |
| ATOM | 396.566 | 1.37169 | 1.3227 | $-19 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.31635 | $63 | -0.005 |
| TIA | -1057.05 | 0.421737 | 0.36724 | $58 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.3266 | $-44 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.082813 | $-20 | +0.002 |
| LTC | -3.23093 | 54.9478 | 48.1765 | $22 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.127325 | $-34 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.76848 | $-8 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -1.14, "RENDER": -1.32, "SEI": -1.35, "TAO": -1.29, "WLD": -1.4}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -1.14, "RENDER": -1.32, "SEI": -1.35, "TAO": -1.29, "WLD": -1.4}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 7.5, "HYPE": 8.3, "PENDLE": 11.0, "UNI": 5.9, "ZEC": 9.3}, "last_day": "2026-11-14", "top_apr": {"BTC": 9.5, "CRV": 10.6, "HBAR": 9.6, "LDO": 11.0, "OP": 10.9, "PENDLE": 11.0, "RENDER": 10.2, "SEI": 8.9, "TAO": 9.6, "ZEC": 9.3}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-13T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-13T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-14"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-13T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-14T00:05 | dn_funding_carry | ENA/spot | buy | $2,498 | 0.134782 | daily dn-carry |
| 2026-11-14T00:05 | dn_funding_carry | ENA | sell | $2,494 | 0.134607 | daily dn-carry |
| 2026-11-14T00:05 | xs_momentum | JUP | buy | $1,583 | 0.26567 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | ATOM | buy | $1,134 | 1.37169 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | RENDER | buy | $1,087 | 1.53407 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | APT | buy | $992 | 0.53595 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | BTC | sell | $1,060 | 68333.1 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | BNB | sell | $1,378 | 630.863 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | DOGE | sell | $1,648 | 0.0646378 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_momentum | ETH | sell | $2,154 | 1931.65 | signal 2026-11-13 23h |
| 2026-11-14T00:05 | xs_funding | OP | buy | $4,096 | 0.0926854 | daily xs-funding (Binance recon) |
| 2026-11-14T00:05 | xs_funding | INJ | buy | $3,670 | 8.14204 | daily xs-funding (Binance recon) |
| 2026-11-14T00:05 | xs_funding | kPEPE | buy | $2,856 | 0.0044165 | daily xs-funding (Binance recon) |
| 2026-11-14T00:05 | xs_funding | TIA | sell | $3,139 | 0.382952 | daily xs-funding (Binance recon) |
| 2026-11-14T00:05 | xs_funding | DOT | sell | $4,345 | 0.818505 | daily xs-funding (Binance recon) |
| 2026-11-14T00:05 | xs_funding | BTC | sell | $9,864 | 68333.1 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | dn_funding_carry | UNI/spot | buy | $2,023 | 6.09179 | daily dn-carry |
| 2026-11-13T00:05 | dn_funding_carry | UNI | sell | $2,055 | 6.0799 | daily dn-carry |
| 2026-11-13T00:05 | xs_momentum | AVAX | buy | $1,476 | 8.35664 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | TAO | buy | $1,429 | 211.979 | signal 2026-11-12 23h |
