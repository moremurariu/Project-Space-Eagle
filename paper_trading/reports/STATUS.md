# Paper trading status — 2026-11-14 03:04 UTC

BTC 67,602 · ETH 1,908.3 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $87,215 | -12.78% | -4.60% | -11.07% | -16.25% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,274 | -2.73% | -0.23% | -1.41% | -4.00% | nan | 0.05 | +0.05 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,192 | +0.19% | +0.01% | +0.06% | -0.19% | nan | 1.41 | -0.00 | 10 | $238 | $303 |
| trend_breakout_majors | $110,237 | +10.24% | +4.10% | +11.00% | -5.48% | nan | 0.69 | -0.69 | 3 | $110 | $165 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,099 | +0.10% | -1.07% | +0.35% | -1.56% | nan | 0.98 | +0.02 | 20 | $539 | $109 |
| xs_momentum | $98,995 | -1.01% | +0.56% | -1.65% | -2.98% | nan | 0.84 | -0.03 | 38 | $176 | $-8 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,108,125 on $1,100,000 → +0.74%**
Combined BTC exposure (7d realised, 4h returns): beta -0.050 (≈ -0.55 books of BTC), corr -0.75

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 67641.2 | $-12,715 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 67602.5 | $-666 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.51925 | $2,885 | -0.147 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.5196 | $-2,891 | +0.147 |
| ENA/spot | 111538 | 0.1792 | 0.13125 | $-5,348 | +0.146 |
| ENA | -111613 | 0.179105 | 0.131115 | $5,356 | -0.146 |
| HYPE/spot | 201.997 | 85.5551 | 70.4925 | $-3,043 | +0.142 |
| HYPE | -202.014 | 85.5972 | 70.4495 | $3,060 | -0.142 |
| UNI | -2471.47 | 6.90012 | 5.62235 | $3,158 | -0.139 |
| UNI/spot | 2468.37 | 6.89946 | 5.626 | $-3,143 | +0.139 |
| ZEC/spot | 14.3484 | 1242.39 | 932.7 | $-4,444 | +0.134 |
| ZEC | -14.3511 | 1242.13 | 932.335 | $4,446 | -0.134 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 76.0485 | $5,553 | -0.253 |
| ETH | -12.8804 | 2187.58 | 1908.35 | $3,597 | -0.223 |
| BTC | -0.356716 | 75630.7 | 67602.5 | $2,864 | -0.219 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 67602.5 | $105 | -0.097 |
| TRX | 28470.1 | 0.309116 | 0.311535 | $69 | +0.089 |
| XLM | 44454.5 | 0.151093 | 0.144025 | $-314 | +0.064 |
| XRP | -6275.1 | 1.21983 | 0.999705 | $1,381 | -0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.060549 | $-438 | +0.056 |
| TAO | -25.0244 | 220.686 | 213.58 | $178 | -0.053 |
| BCH | 23.4344 | 238.546 | 218.875 | $-461 | +0.051 |
| LINK | -516.66 | 11.3839 | 9.69505 | $873 | -0.050 |
| ATOM | 3529.46 | 1.49903 | 1.3434 | $-549 | +0.047 |
| JUP | 18056.4 | 0.241363 | 0.26133 | $361 | +0.047 |
| LDO | -14834.4 | 0.308439 | 0.29902 | $140 | -0.044 |
| RENDER | -2753.67 | 1.56167 | 1.523 | $106 | -0.042 |
| OP | 44197.2 | 0.0926854 | 0.09017 | $-111 | +0.040 |
| ARB | 27302.4 | 0.151348 | 0.14177 | $-261 | +0.039 |
| PENDLE | -2505.4 | 1.58072 | 1.51925 | $154 | -0.038 |
| FIL | 5349.98 | 0.760721 | 0.69305 | $-362 | +0.037 |
| NEAR | -997.435 | 3.77006 | 3.41985 | $349 | -0.034 |
| TIA | -8196.2 | 0.382952 | 0.37403 | $73 | -0.031 |
| WLD | 10593 | 0.336516 | 0.2886 | $-508 | +0.031 |
| ENA | -22442.6 | 0.146112 | 0.131115 | $337 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 76.0485 | $1,390 | -0.067 |
| TRX | 20882.3 | 0.317091 | 0.311535 | $-116 | +0.066 |
| XRP | -4826.67 | 1.05611 | 0.999705 | $272 | -0.049 |
| PENDLE | -2522.74 | 1.58296 | 1.51925 | $161 | -0.039 |
| RENDER | 2461.25 | 1.58063 | 1.523 | $-142 | +0.038 |
| INJ | 456.574 | 7.51016 | 7.9837 | $216 | +0.037 |
| kPEPE | 803792 | 0.00484647 | 0.004267 | $-466 | +0.035 |
| UNI | 597.681 | 7.08573 | 5.62235 | $-875 | +0.034 |
| AVAX | 376.81 | 8.37846 | 8.1159 | $-99 | +0.031 |
| ARB | 21235.8 | 0.1621 | 0.14177 | $-432 | +0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.063163 | $160 | -0.030 |
| WLD | -9976.36 | 0.34434 | 0.2886 | $556 | -0.029 |
| JUP | 10388.9 | 0.261398 | 0.26133 | $-1 | +0.027 |
| ADA | -16388.2 | 0.190675 | 0.16487 | $423 | -0.027 |
| DOT | -3218.79 | 0.900177 | 0.79751 | $330 | -0.026 |
| CRV | -11195.5 | 0.271234 | 0.22849 | $479 | -0.026 |
| SEI | -60987.3 | 0.0472943 | 0.041875 | $331 | -0.026 |
| BTC | -0.0355851 | 70094.1 | 67602.5 | $89 | -0.024 |
| XLM | -16417.2 | 0.145302 | 0.144025 | $21 | -0.024 |
| BNB | 3.50367 | 705.088 | 624.865 | $-281 | +0.022 |
| OP | 24123.2 | 0.107142 | 0.09017 | $-409 | +0.022 |
| AAVE | 18.1796 | 131.483 | 110.19 | $-387 | +0.020 |
| ZEC | 1.94807 | 1213.4 | 932.335 | $-548 | +0.018 |
| FIL | -2342.25 | 0.799282 | 0.69305 | $249 | -0.016 |
| BCH | -6.47026 | 225.866 | 218.875 | $45 | -0.014 |
| HBAR | -16510.2 | 0.0706575 | 0.060549 | $167 | -0.010 |
| ETH | -0.43933 | 1931.65 | 1908.35 | $10 | -0.008 |
| LINK | 77.5637 | 11.0659 | 9.69505 | $-106 | +0.008 |
| APT | -1385.4 | 0.626089 | 0.5246 | $141 | -0.007 |
| TAO | 2.54822 | 211.979 | 213.58 | $4 | +0.005 |
| ATOM | 396.566 | 1.37169 | 1.3434 | $-11 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.41985 | $47 | -0.005 |
| TIA | -1057.05 | 0.421737 | 0.37403 | $50 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.33306 | $-39 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.083879 | $-18 | +0.002 |
| LTC | -3.23093 | 54.9478 | 48.922 | $19 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.131115 | $-32 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.78297 | $-7 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"DOT": -1.72, "OP": -1.95, "SOL": -1.87, "TIA": -2.4, "kPEPE": -2.04}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"DOT": -1.72, "OP": -1.95, "SOL": -1.87, "TIA": -2.4, "kPEPE": -2.04}}`
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
