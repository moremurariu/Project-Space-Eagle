# Paper trading status — 2026-11-13 12:05 UTC

BTC 69,358 · ETH 1,962.4 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $89,456 | -10.54% | -4.87% | -8.84% | -15.51% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,392 | -2.61% | -0.26% | -1.38% | -3.96% | nan | 0.05 | +0.05 | 1 | $53 | $-94 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.10% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.03% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,121 | +0.12% | -0.06% | +0.03% | -0.19% | nan | 1.41 | -0.00 | 10 | $233 | $292 |
| trend_breakout_majors | $107,197 | +7.20% | +2.47% | +7.75% | -5.48% | nan | 0.74 | -0.74 | 3 | $110 | $159 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,774 | +0.77% | -0.66% | +1.03% | -1.13% | nan | 0.94 | +0.09 | 20 | $518 | $103 |
| xs_momentum | $98,313 | -1.69% | -0.08% | -2.39% | -2.98% | nan | 0.84 | -0.02 | 38 | $169 | $-9 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,105,124 on $1,100,000 → +0.47%**
Combined BTC exposure (7d realised, 4h returns): beta -0.044 (≈ -0.48 books of BTC), corr -0.76

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 69377 | $-10,474 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 69357.5 | $-548 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 201.997 | 85.5551 | 74.158 | $-2,302 | +0.150 |
| HYPE | -202.014 | 85.5972 | 74.0985 | $2,323 | -0.150 |
| PENDLE | -9671.02 | 1.81759 | 1.5075 | $2,999 | -0.146 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.5078 | $-3,005 | +0.146 |
| UNI | -2471.47 | 6.90012 | 5.8287 | $2,648 | -0.144 |
| UNI/spot | 2468.37 | 6.89946 | 5.82937 | $-2,641 | +0.144 |
| ZEC | -14.3511 | 1242.13 | 967.26 | $3,945 | -0.139 |
| ZEC/spot | 14.3484 | 1242.39 | 964.625 | $-3,985 | +0.138 |
| ENA | -93083.1 | 0.187964 | 0.13831 | $4,622 | -0.129 |
| ENA/spot | 93002.9 | 0.188053 | 0.13829 | $-4,628 | +0.128 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 80.7215 | $3,840 | -0.276 |
| ETH | -12.8804 | 2187.58 | 1962.4 | $2,900 | -0.236 |
| BTC | -0.356716 | 75630.7 | 69357.5 | $2,238 | -0.231 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.312485 | $96 | +0.088 |
| XLM | 44454.5 | 0.151093 | 0.147775 | $-147 | +0.065 |
| XRP | -6275.1 | 1.21983 | 1.02525 | $1,221 | -0.064 |
| HBAR | 92374.3 | 0.0652853 | 0.062877 | $-222 | +0.058 |
| TAO | -25.0244 | 220.686 | 212.755 | $198 | -0.053 |
| BCH | 23.4344 | 238.546 | 224.435 | $-331 | +0.052 |
| LINK | -516.66 | 11.3839 | 9.9995 | $715 | -0.051 |
| ATOM | 3529.46 | 1.49903 | 1.3936 | $-372 | +0.049 |
| JUP | 18056.4 | 0.241363 | 0.26297 | $390 | +0.047 |
| LDO | -14834.4 | 0.308439 | 0.30667 | $26 | -0.045 |
| DOT | 5309.02 | 0.856105 | 0.83181 | $-129 | +0.044 |
| RENDER | -2753.67 | 1.56167 | 1.534 | $76 | -0.042 |
| ARB | 27302.4 | 0.151348 | 0.14763 | $-101 | +0.040 |
| FIL | 5349.98 | 0.760721 | 0.73569 | $-134 | +0.039 |
| PENDLE | -2505.4 | 1.58072 | 1.5075 | $183 | -0.037 |
| NEAR | -997.435 | 3.77006 | 3.56155 | $208 | -0.035 |
| INJ | -450.711 | 8.02739 | 7.78515 | $109 | -0.035 |
| WLD | 10593 | 0.336516 | 0.30234 | $-362 | +0.032 |
| ENA | -22442.6 | 0.146112 | 0.13831 | $175 | -0.031 |
| kPEPE | -646727 | 0.00494453 | 0.004512 | $280 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 80.7215 | $983 | -0.072 |
| TRX | 20882.3 | 0.317091 | 0.312485 | $-96 | +0.066 |
| XRP | -4826.67 | 1.05611 | 1.02525 | $149 | -0.050 |
| PENDLE | -2522.74 | 1.58296 | 1.5075 | $190 | -0.039 |
| BNB | 5.68814 | 705.088 | 639.545 | $-373 | +0.037 |
| kPEPE | 803792 | 0.00484647 | 0.004512 | $-269 | +0.037 |
| INJ | 456.574 | 7.51016 | 7.78515 | $126 | +0.036 |
| UNI | 597.681 | 7.08573 | 5.8287 | $-751 | +0.035 |
| AVAX | 376.81 | 8.37846 | 8.4066 | $11 | +0.032 |
| ARB | 21235.8 | 0.1621 | 0.14763 | $-307 | +0.032 |
| WLD | -9976.36 | 0.34434 | 0.30234 | $419 | -0.031 |
| ADA | -16388.2 | 0.190675 | 0.172055 | $305 | -0.029 |
| RENDER | 1752.76 | 1.59944 | 1.534 | $-115 | +0.027 |
| DOT | -3218.79 | 0.900177 | 0.83181 | $220 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.23742 | $379 | -0.027 |
| SEI | -60987.3 | 0.0472943 | 0.043321 | $242 | -0.027 |
| XLM | -16417.2 | 0.145302 | 0.147775 | $-41 | -0.025 |
| OP | 24123.2 | 0.107142 | 0.09391 | $-319 | +0.023 |
| AAVE | 18.1796 | 131.483 | 111.8 | $-358 | +0.021 |
| ZEC | 1.94807 | 1213.4 | 967.26 | $-479 | +0.019 |
| APT | -3237.04 | 0.626089 | 0.5478 | $253 | -0.018 |
| FIL | -2342.25 | 0.799282 | 0.73569 | $149 | -0.018 |
| BCH | -6.47026 | 225.866 | 224.435 | $9 | -0.015 |
| DOGE | -21412 | 0.0688816 | 0.065413 | $74 | -0.014 |
| BTC | -0.0200793 | 71453.9 | 69357.5 | $42 | -0.014 |
| ETH | 0.675911 | 2238.8 | 1962.4 | $-187 | +0.013 |
| JUP | 4429.65 | 0.25565 | 0.26297 | $32 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.062877 | $128 | -0.011 |
| LINK | 77.5637 | 11.0659 | 9.9995 | $-83 | +0.008 |
| ATOM | -429.872 | 1.43333 | 1.3936 | $17 | -0.006 |
| NEAR | -155.553 | 3.72366 | 3.56155 | $25 | -0.006 |
| TAO | 2.54822 | 211.979 | 212.755 | $2 | +0.006 |
| TIA | -1057.05 | 0.421737 | 0.41658 | $5 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.345225 | $-31 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.087962 | $-9 | +0.002 |
| LTC | -3.23093 | 54.9478 | 49.1655 | $19 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.13831 | $-27 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.80438 | $-5 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"JUP": -1.21, "OP": -0.32, "TIA": -1.28, "XRP": -0.2, "kPEPE": -1.32}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"JUP": -1.21, "OP": -0.32, "TIA": -1.28, "XRP": -0.2, "kPEPE": -1.32}}`
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
