# Paper trading status — 2026-11-13 23:04 UTC

BTC 68,008 · ETH 1,918.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $87,738 | -12.26% | -4.88% | -10.56% | -15.75% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,301 | -2.70% | -0.24% | -1.39% | -3.98% | nan | 0.05 | +0.05 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,177 | +0.18% | -0.00% | +0.07% | -0.19% | nan | 1.37 | -0.00 | 10 | $233 | $301 |
| trend_breakout_majors | $109,529 | +9.53% | +3.85% | +10.22% | -5.48% | nan | 0.71 | -0.71 | 3 | $110 | $165 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,411 | +0.41% | -1.03% | +0.48% | -1.24% | nan | 0.92 | +0.09 | 20 | $518 | $107 |
| xs_momentum | $98,889 | -1.11% | +0.76% | -1.50% | -2.98% | nan | 0.82 | -0.02 | 38 | $169 | $-8 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,107,635 on $1,100,000 → +0.69%**
Combined BTC exposure (7d realised, 4h returns): beta -0.050 (≈ -0.55 books of BTC), corr -0.75

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 68045.9 | $-12,192 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 68008.5 | $-639 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.52205 | $2,858 | -0.147 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.5225 | $-2,863 | +0.147 |
| HYPE | -202.014 | 85.5972 | 71.5585 | $2,836 | -0.144 |
| HYPE/spot | 201.997 | 85.5551 | 71.5615 | $-2,827 | +0.144 |
| UNI | -2471.47 | 6.90012 | 5.7053 | $2,953 | -0.141 |
| UNI/spot | 2468.37 | 6.89946 | 5.708 | $-2,941 | +0.141 |
| ZEC | -14.3511 | 1242.13 | 923.025 | $4,579 | -0.132 |
| ZEC/spot | 14.3484 | 1242.39 | 923.13 | $-4,581 | +0.132 |
| ENA | -93083.1 | 0.187964 | 0.13204 | $5,206 | -0.123 |
| ENA/spot | 93002.9 | 0.188053 | 0.132146 | $-5,199 | +0.123 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 77.2315 | $5,119 | -0.258 |
| ETH | -12.8804 | 2187.58 | 1918.45 | $3,467 | -0.226 |
| BTC | -0.356716 | 75630.7 | 68008.5 | $2,719 | -0.221 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.311905 | $79 | +0.088 |
| XLM | 44454.5 | 0.151093 | 0.145285 | $-258 | +0.064 |
| XRP | -6275.1 | 1.21983 | 1.00455 | $1,351 | -0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.060981 | $-398 | +0.056 |
| TAO | -25.0244 | 220.686 | 211.765 | $223 | -0.053 |
| BCH | 23.4344 | 238.546 | 219.92 | $-436 | +0.051 |
| LINK | -516.66 | 11.3839 | 9.72955 | $855 | -0.050 |
| ATOM | 3529.46 | 1.49903 | 1.3592 | $-494 | +0.048 |
| JUP | 18056.4 | 0.241363 | 0.26156 | $365 | +0.047 |
| LDO | -14834.4 | 0.308439 | 0.29983 | $128 | -0.044 |
| DOT | 5309.02 | 0.856105 | 0.8109 | $-240 | +0.043 |
| RENDER | -2753.67 | 1.56167 | 1.51035 | $141 | -0.041 |
| ARB | 27302.4 | 0.151348 | 0.14379 | $-206 | +0.039 |
| PENDLE | -2505.4 | 1.58072 | 1.52205 | $147 | -0.038 |
| FIL | 5349.98 | 0.760721 | 0.70051 | $-322 | +0.037 |
| INJ | -450.711 | 8.02739 | 7.99115 | $16 | -0.036 |
| NEAR | -997.435 | 3.77006 | 3.40195 | $367 | -0.034 |
| WLD | 10593 | 0.336516 | 0.29201 | $-471 | +0.031 |
| ENA | -22442.6 | 0.146112 | 0.13204 | $316 | -0.030 |
| kPEPE | -646727 | 0.00494453 | 0.004404 | $350 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 77.2315 | $1,287 | -0.068 |
| TRX | 20882.3 | 0.317091 | 0.311905 | $-108 | +0.066 |
| XRP | -4826.67 | 1.05611 | 1.00455 | $249 | -0.049 |
| PENDLE | -2522.74 | 1.58296 | 1.52205 | $154 | -0.039 |
| INJ | 456.574 | 7.51016 | 7.99115 | $220 | +0.037 |
| BNB | 5.68814 | 705.088 | 627.76 | $-440 | +0.036 |
| kPEPE | 803792 | 0.00484647 | 0.004404 | $-356 | +0.036 |
| UNI | 597.681 | 7.08573 | 5.7053 | $-825 | +0.034 |
| AVAX | 376.81 | 8.37846 | 8.1552 | $-84 | +0.031 |
| ARB | 21235.8 | 0.1621 | 0.14379 | $-389 | +0.031 |
| WLD | -9976.36 | 0.34434 | 0.29201 | $522 | -0.029 |
| ADA | -16388.2 | 0.190675 | 0.16659 | $395 | -0.028 |
| RENDER | 1752.76 | 1.59944 | 1.51035 | $-156 | +0.027 |
| DOT | -3218.79 | 0.900177 | 0.8109 | $287 | -0.026 |
| SEI | -60987.3 | 0.0472943 | 0.042254 | $307 | -0.026 |
| CRV | -11195.5 | 0.271234 | 0.2283 | $481 | -0.026 |
| XLM | -16417.2 | 0.145302 | 0.145285 | $0 | -0.024 |
| OP | 24123.2 | 0.107142 | 0.09172 | $-372 | +0.022 |
| AAVE | 18.1796 | 131.483 | 110.765 | $-377 | +0.020 |
| ZEC | 1.94807 | 1213.4 | 923.025 | $-566 | +0.018 |
| APT | -3237.04 | 0.626089 | 0.5295 | $313 | -0.017 |
| FIL | -2342.25 | 0.799282 | 0.70051 | $231 | -0.017 |
| BCH | -6.47026 | 225.866 | 219.92 | $38 | -0.014 |
| DOGE | -21412 | 0.0688816 | 0.063854 | $108 | -0.014 |
| BTC | -0.0200793 | 71453.9 | 68008.5 | $69 | -0.014 |
| ETH | 0.675911 | 2238.8 | 1918.45 | $-217 | +0.013 |
| JUP | 4429.65 | 0.25565 | 0.26156 | $26 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.060981 | $160 | -0.010 |
| LINK | 77.5637 | 11.0659 | 9.72955 | $-104 | +0.008 |
| ATOM | -429.872 | 1.43333 | 1.3592 | $32 | -0.006 |
| TAO | 2.54822 | 211.979 | 211.765 | $-1 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.40195 | $50 | -0.005 |
| TIA | -1057.05 | 0.421737 | 0.3817 | $42 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.33626 | $-37 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.084398 | $-17 | +0.002 |
| LTC | -3.23093 | 54.9478 | 48.9635 | $19 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.13204 | $-31 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.78832 | $-7 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -3.07, "FIL": -3.02, "HBAR": -3.4, "SOL": -3.07, "TIA": -4.24}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ENA": -3.07, "FIL": -3.02, "HBAR": -3.4, "SOL": -3.07, "TIA": -4.24}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 8.3, "HYPE": 8.5, "PENDLE": 11.0, "UNI": 7.0, "ZEC": 10.5}, "last_day": "2026-11-13", "top_apr": {"BTC": 9.9, "CRV": 10.6, "HBAR": 10.3, "LDO": 11.0, "OP": 10.8, "PENDLE": 11.0, "RENDER": 10.8, "SEI": 9.1, "TAO": 9.8, "ZEC": 10.5}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-12T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-12T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-13"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-12T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-13T00:05 | dn_funding_carry | UNI/spot | buy | $2,023 | 6.09179 | daily dn-carry |
| 2026-11-13T00:05 | dn_funding_carry | UNI | sell | $2,055 | 6.0799 | daily dn-carry |
| 2026-11-13T00:05 | xs_momentum | AVAX | buy | $1,476 | 8.35664 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | TAO | buy | $1,429 | 211.979 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | ATOM | buy | $1,262 | 1.43547 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | ADA | buy | $1,044 | 0.17501 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | XRP | sell | $1,143 | 1.0422 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | BCH | sell | $1,191 | 225.38 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | LINK | sell | $1,213 | 10.1475 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | ZEC | sell | $1,228 | 958.035 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_momentum | BTC | sell | $1,864 | 71453.9 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | xs_funding | ETC | buy | $6,620 | 6.67626 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | xs_funding | ARB | buy | $4,132 | 0.151348 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | xs_funding | TAO | sell | $1,081 | 211.868 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | xs_funding | OP | sell | $3,952 | 0.0968266 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | xs_funding | LDO | sell | $4,576 | 0.308439 | daily xs-funding (Binance recon) |
| 2026-11-13T00:05 | trend_breakout_majors | ETH | sell | $2,237 | 2018.65 | signal 2026-11-12 23h |
| 2026-11-13T00:05 | trend_breakout_majors | SOL | sell | $2,388 | 81.6914 | signal 2026-11-12 23h |
| 2026-11-12T12:06 | claude_discretionary | BTC | sell | $9,694 | 72796.9 | discretionary: Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liq |
| 2026-11-12T07:04 | crash_rebound | ZEC | sell | $7,601 | 987.166 | crash-rebound exit t2 (24h) |
