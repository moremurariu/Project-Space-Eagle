# Paper trading status — 2026-11-14 21:04 UTC

BTC 64,684 · ETH 1,783.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,462 | -16.54% | -6.26% | -14.61% | -19.86% | -10.42 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,077 | -2.92% | -0.30% | -1.52% | -4.20% | -6.33 | 0.04 | +0.04 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.59 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.97 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,212 | +0.21% | +0.04% | +0.08% | -0.19% | 4.21 | 1.25 | +0.00 | 10 | $238 | $302 |
| trend_breakout_majors | $114,870 | +14.87% | +6.76% | +15.28% | -5.48% | 10.13 | 0.63 | -0.63 | 3 | $110 | $164 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.75 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.94 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,803 | +0.80% | +0.42% | +1.05% | -1.56% | 2.74 | 0.89 | +0.03 | 20 | $539 | $122 |
| xs_momentum | $99,141 | -0.86% | +0.69% | -1.29% | -2.98% | nan | 0.76 | -0.03 | 38 | $176 | $-9 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,113,431 on $1,100,000 → +1.22%**
Combined BTC exposure (7d realised, 4h returns): beta -0.055 (≈ -0.61 books of BTC), corr -0.77

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64734 | $-16,468 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 64683.5 | $-862 | +0.045 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 9662.31 | 1.81881 | 1.389 | $-4,153 | +0.134 |
| PENDLE | -9671.02 | 1.81759 | 1.38725 | $4,162 | -0.134 |
| ENA/spot | 111538 | 0.1792 | 0.11432 | $-7,237 | +0.127 |
| ENA | -111613 | 0.179105 | 0.11413 | $7,252 | -0.127 |
| HYPE | -202.014 | 85.5972 | 62.77 | $4,611 | -0.127 |
| HYPE/spot | 201.997 | 85.5551 | 62.728 | $-4,611 | +0.126 |
| ZEC/spot | 14.3484 | 1242.39 | 831.287 | $-5,899 | +0.119 |
| ZEC | -14.3511 | 1242.13 | 830.675 | $5,905 | -0.119 |
| UNI/spot | 2468.37 | 6.89946 | 4.789 | $-5,209 | +0.118 |
| UNI | -2471.47 | 6.90012 | 4.78175 | $5,235 | -0.118 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 70.6295 | $7,539 | -0.225 |
| BTC | -0.356716 | 75630.7 | 64683.5 | $3,905 | -0.201 |
| ETH | -12.8804 | 2187.58 | 1783.65 | $5,203 | -0.200 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64683.5 | $527 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.307485 | $-46 | +0.087 |
| XLM | 44454.5 | 0.151093 | 0.13274 | $-816 | +0.059 |
| XRP | -6275.1 | 1.21983 | 0.926705 | $1,839 | -0.058 |
| HBAR | 92374.3 | 0.0652853 | 0.056386 | $-822 | +0.052 |
| BCH | 23.4344 | 238.546 | 201.905 | $-859 | +0.047 |
| TAO | -25.0244 | 220.686 | 183.16 | $939 | -0.045 |
| LINK | -516.66 | 11.3839 | 8.8155 | $1,327 | -0.045 |
| ATOM | 3529.46 | 1.49903 | 1.2475 | $-888 | +0.044 |
| JUP | 18056.4 | 0.241363 | 0.242435 | $19 | +0.043 |
| LDO | -14834.4 | 0.308439 | 0.26297 | $675 | -0.039 |
| RENDER | -2753.67 | 1.56167 | 1.36345 | $546 | -0.037 |
| ARB | 27302.4 | 0.151348 | 0.12779 | $-643 | +0.035 |
| PENDLE | -2505.4 | 1.58072 | 1.38725 | $485 | -0.034 |
| OP | 44197.2 | 0.0926854 | 0.07796 | $-651 | +0.034 |
| FIL | 5349.98 | 0.760721 | 0.61092 | $-801 | +0.032 |
| NEAR | -997.435 | 3.77006 | 2.9677 | $800 | -0.029 |
| TIA | -8196.2 | 0.382952 | 0.33644 | $381 | -0.027 |
| WLD | 10593 | 0.336516 | 0.24991 | $-917 | +0.026 |
| ENA | -22442.6 | 0.146112 | 0.11413 | $718 | -0.025 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 20882.3 | 0.317091 | 0.307485 | $-201 | +0.065 |
| SOL | -87.2593 | 91.9832 | 70.6295 | $1,863 | -0.062 |
| XRP | -4826.67 | 1.05611 | 0.926705 | $625 | -0.045 |
| PENDLE | -2522.74 | 1.58296 | 1.38725 | $494 | -0.035 |
| RENDER | 2461.25 | 1.58063 | 1.36345 | $-535 | +0.034 |
| INJ | 456.574 | 7.51016 | 6.98035 | $-242 | +0.032 |
| kPEPE | 803792 | 0.00484647 | 0.003828 | $-819 | +0.031 |
| UNI | 597.681 | 7.08573 | 4.78175 | $-1,377 | +0.029 |
| ARB | 21235.8 | 0.1621 | 0.12779 | $-729 | +0.027 |
| DOGE | -46908.3 | 0.0665749 | 0.057816 | $411 | -0.027 |
| AVAX | 376.81 | 8.37846 | 7.1705 | $-455 | +0.027 |
| JUP | 10388.9 | 0.261398 | 0.242435 | $-197 | +0.025 |
| WLD | -9976.36 | 0.34434 | 0.24991 | $942 | -0.025 |
| ADA | -16388.2 | 0.190675 | 0.148565 | $690 | -0.025 |
| BTC | -0.0355851 | 70094.1 | 64683.5 | $193 | -0.023 |
| SEI | -60987.3 | 0.0472943 | 0.037478 | $599 | -0.023 |
| DOT | -3218.79 | 0.900177 | 0.70777 | $619 | -0.023 |
| CRV | -11195.5 | 0.271234 | 0.20242 | $770 | -0.023 |
| XLM | -16417.2 | 0.145302 | 0.13274 | $206 | -0.022 |
| BNB | 3.50367 | 705.088 | 583.35 | $-427 | +0.021 |
| OP | 24123.2 | 0.107142 | 0.07796 | $-704 | +0.019 |
| AAVE | 18.1796 | 131.483 | 99.4985 | $-581 | +0.018 |
| ZEC | 1.94807 | 1213.4 | 830.675 | $-746 | +0.016 |
| FIL | -2342.25 | 0.799282 | 0.61092 | $441 | -0.014 |
| BCH | -6.47026 | 225.866 | 201.905 | $155 | -0.013 |
| HBAR | -16510.2 | 0.0706575 | 0.056386 | $236 | -0.009 |
| ETH | -0.43933 | 1931.65 | 1783.65 | $65 | -0.008 |
| LINK | 77.5637 | 11.0659 | 8.8155 | $-175 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4456 | $250 | -0.006 |
| ATOM | 396.566 | 1.37169 | 1.2475 | $-49 | +0.005 |
| TAO | 2.54822 | 211.979 | 183.16 | $-73 | +0.005 |
| NEAR | -155.553 | 3.72366 | 2.9677 | $118 | -0.005 |
| TIA | -1057.05 | 0.421737 | 0.33644 | $90 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.29507 | $-64 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.076774 | $-32 | +0.002 |
| LTC | -3.23093 | 54.9478 | 44.4585 | $34 | -0.001 |
| ENA | 664.451 | 0.179237 | 0.11413 | $-43 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.69918 | $-12 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -6.02, "DOT": -5.47, "FIL": -6.51, "LDO": -4.83, "OP": -6.24}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -6.02, "DOT": -5.47, "FIL": -6.51, "LDO": -4.83, "OP": -6.24}}`
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
