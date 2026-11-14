# Paper trading status — 2026-11-14 16:04 UTC

BTC 65,620 · ETH 1,835.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $84,639 | -15.36% | -4.63% | -13.54% | -18.73% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,140 | -2.86% | -0.22% | -1.50% | -4.13% | nan | 0.05 | +0.05 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,198 | +0.20% | -0.03% | +0.06% | -0.19% | nan | 1.33 | +0.00 | 10 | $238 | $304 |
| trend_breakout_majors | $112,801 | +12.80% | +4.34% | +13.35% | -5.48% | nan | 0.66 | -0.66 | 3 | $110 | $166 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,743 | +0.74% | +0.06% | +0.97% | -1.56% | nan | 0.94 | +0.02 | 20 | $539 | $116 |
| xs_momentum | $98,961 | -1.04% | +0.48% | -1.50% | -2.98% | nan | 0.80 | -0.03 | 38 | $176 | $-10 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,111,171 on $1,100,000 → +1.02%**
Combined BTC exposure (7d realised, 4h returns): beta -0.052 (≈ -0.58 books of BTC), corr -0.78

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65646 | $-15,291 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 65620.5 | $-799 | +0.045 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.4711 | $3,351 | -0.142 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4714 | $-3,357 | +0.142 |
| ENA/spot | 111538 | 0.1792 | 0.122279 | $-6,349 | +0.136 |
| ENA | -111613 | 0.179105 | 0.122155 | $6,356 | -0.136 |
| HYPE/spot | 201.997 | 85.5551 | 67.4345 | $-3,660 | +0.136 |
| HYPE | -202.014 | 85.5972 | 67.407 | $3,675 | -0.136 |
| ZEC/spot | 14.3484 | 1242.39 | 884.276 | $-5,138 | +0.127 |
| ZEC | -14.3511 | 1242.13 | 883.805 | $5,142 | -0.127 |
| UNI | -2471.47 | 6.90012 | 5.09185 | $4,469 | -0.126 |
| UNI/spot | 2468.37 | 6.89946 | 5.098 | $-4,447 | +0.126 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 73.5615 | $6,464 | -0.239 |
| ETH | -12.8804 | 2187.58 | 1835.05 | $4,541 | -0.210 |
| BTC | -0.356716 | 75630.7 | 65620.5 | $3,571 | -0.208 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 65620.5 | $392 | -0.094 |
| TRX | 28470.1 | 0.309116 | 0.309235 | $3 | +0.087 |
| XLM | 44454.5 | 0.151093 | 0.138865 | $-544 | +0.061 |
| XRP | -6275.1 | 1.21983 | 0.970285 | $1,566 | -0.060 |
| HBAR | 92374.3 | 0.0652853 | 0.058573 | $-620 | +0.054 |
| TAO | -25.0244 | 220.686 | 197.56 | $579 | -0.049 |
| BCH | 23.4344 | 238.546 | 210.735 | $-652 | +0.049 |
| LINK | -516.66 | 11.3839 | 9.2206 | $1,118 | -0.047 |
| ATOM | 3529.46 | 1.49903 | 1.3166 | $-644 | +0.046 |
| JUP | 18056.4 | 0.241363 | 0.255265 | $251 | +0.046 |
| LDO | -14834.4 | 0.308439 | 0.28018 | $419 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.4636 | $270 | -0.040 |
| ARB | 27302.4 | 0.151348 | 0.13727 | $-384 | +0.037 |
| OP | 44197.2 | 0.0926854 | 0.08479 | $-349 | +0.037 |
| PENDLE | -2505.4 | 1.58072 | 1.4711 | $275 | -0.037 |
| FIL | 5349.98 | 0.760721 | 0.66154 | $-531 | +0.035 |
| NEAR | -997.435 | 3.77006 | 3.1934 | $575 | -0.032 |
| TIA | -8196.2 | 0.382952 | 0.35875 | $198 | -0.029 |
| WLD | 10593 | 0.336516 | 0.27022 | $-702 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.122155 | $538 | -0.027 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 20882.3 | 0.317091 | 0.309235 | $-164 | +0.065 |
| SOL | -87.2593 | 91.9832 | 73.5615 | $1,607 | -0.065 |
| XRP | -4826.67 | 1.05611 | 0.970285 | $414 | -0.047 |
| PENDLE | -2522.74 | 1.58296 | 1.4711 | $282 | -0.038 |
| RENDER | 2461.25 | 1.58063 | 1.4636 | $-288 | +0.036 |
| INJ | 456.574 | 7.51016 | 7.5725 | $28 | +0.035 |
| kPEPE | 803792 | 0.00484647 | 0.004026 | $-659 | +0.033 |
| UNI | 597.681 | 7.08573 | 5.09185 | $-1,192 | +0.031 |
| ARB | 21235.8 | 0.1621 | 0.13727 | $-527 | +0.029 |
| AVAX | 376.81 | 8.37846 | 7.6316 | $-281 | +0.029 |
| DOGE | -46908.3 | 0.0665749 | 0.060729 | $274 | -0.029 |
| WLD | -9976.36 | 0.34434 | 0.27022 | $739 | -0.027 |
| JUP | 10388.9 | 0.261398 | 0.255265 | $-64 | +0.027 |
| ADA | -16388.2 | 0.190675 | 0.15649 | $560 | -0.026 |
| DOT | -3218.79 | 0.900177 | 0.76101 | $448 | -0.025 |
| SEI | -60987.3 | 0.0472943 | 0.039778 | $458 | -0.025 |
| CRV | -11195.5 | 0.271234 | 0.21619 | $616 | -0.024 |
| BTC | -0.0355851 | 70094.1 | 65620.5 | $159 | -0.024 |
| XLM | -16417.2 | 0.145302 | 0.138865 | $106 | -0.023 |
| BNB | 3.50367 | 705.088 | 597.09 | $-378 | +0.021 |
| OP | 24123.2 | 0.107142 | 0.08479 | $-539 | +0.021 |
| AAVE | 18.1796 | 131.483 | 103.605 | $-507 | +0.019 |
| ZEC | 1.94807 | 1213.4 | 883.805 | $-642 | +0.017 |
| FIL | -2342.25 | 0.799282 | 0.66154 | $323 | -0.016 |
| BCH | -6.47026 | 225.866 | 210.735 | $98 | -0.014 |
| HBAR | -16510.2 | 0.0706575 | 0.058573 | $200 | -0.010 |
| ETH | -0.43933 | 1931.65 | 1835.05 | $42 | -0.008 |
| LINK | 77.5637 | 11.0659 | 9.2206 | $-143 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4855 | $195 | -0.007 |
| ATOM | 396.566 | 1.37169 | 1.3166 | $-22 | +0.005 |
| TAO | 2.54822 | 211.979 | 197.56 | $-37 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.1934 | $82 | -0.005 |
| TIA | -1057.05 | 0.421737 | 0.35875 | $67 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.313 | $-52 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.080639 | $-24 | +0.002 |
| LTC | -3.23093 | 54.9478 | 45.8565 | $29 | -0.001 |
| ENA | 664.451 | 0.179237 | 0.122155 | $-38 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.73729 | $-10 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AVAX": -4.37, "LTC": -4.81, "TAO": -4.52, "UNI": -6.83, "kPEPE": -6.26}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AVAX": -4.37, "LTC": -4.81, "TAO": -4.52, "UNI": -6.83, "kPEPE": -6.26}}`
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
