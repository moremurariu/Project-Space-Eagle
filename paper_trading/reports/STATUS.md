# Paper trading status — 2026-11-12 13:05 UTC

BTC 72,800 · ETH 2,016.3 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $93,919 | -6.08% | -1.41% | -9.22% | -9.82% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,624 | -2.38% | -0.22% | -3.42% | -3.66% | nan | 0.05 | +0.05 | 1 | $53 | $-93 |
| crash_rebound | $100,123 | +0.12% | +0.09% | +0.12% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,188 | +0.19% | +0.05% | +0.14% | -0.19% | nan | 1.36 | +0.00 | 10 | $230 | $286 |
| trend_breakout_majors | $104,788 | +4.79% | +1.31% | +8.27% | -5.48% | nan | 0.74 | -0.74 | 3 | $108 | $148 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,502 | +1.50% | +1.16% | +2.41% | -1.13% | nan | 0.93 | +0.09 | 20 | $497 | $101 |
| xs_momentum | $98,287 | -1.71% | -0.67% | -1.69% | -2.92% | nan | 0.85 | -0.01 | 38 | $162 | $-22 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,103,716 on $1,100,000 → +0.34%**
Combined BTC exposure (7d realised, 4h returns): beta -0.023 (≈ -0.25 books of BTC), corr -0.55

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 72833.4 | $-6,011 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 72800.5 | $-317 | +0.050 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.49015 | $3,167 | -0.144 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4905 | $-3,172 | +0.144 |
| HYPE/spot | 201.997 | 85.5551 | 70.2645 | $-3,089 | +0.142 |
| HYPE | -202.014 | 85.5972 | 70.2375 | $3,103 | -0.142 |
| ZEC/spot | 14.3484 | 1242.39 | 970.62 | $-3,899 | +0.139 |
| ZEC | -14.3511 | 1242.13 | 969.905 | $3,907 | -0.139 |
| ENA/spot | 93002.9 | 0.188053 | 0.139085 | $-4,554 | +0.129 |
| ENA | -93083.1 | 0.187964 | 0.13896 | $4,561 | -0.129 |
| UNI/spot | 2136.3 | 7.02501 | 6.0275 | $-2,131 | +0.129 |
| UNI | -2133.42 | 7.03009 | 6.02145 | $2,152 | -0.128 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -337.279 | 92.0231 | 82.0405 | $3,367 | -0.264 |
| BTC | -0.356716 | 75630.7 | 72800.5 | $1,010 | -0.248 |
| ETH | -11.7722 | 2203.48 | 2016.35 | $2,203 | -0.227 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.315935 | $194 | +0.089 |
| ETC | -991.516 | 6.59963 | 6.5663 | $33 | -0.064 |
| XRP | -6275.1 | 1.21983 | 1.03405 | $1,166 | -0.064 |
| XLM | 44454.5 | 0.151093 | 0.14488 | $-276 | +0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.064947 | $-31 | +0.059 |
| BCH | 23.4344 | 238.546 | 226.76 | $-276 | +0.052 |
| LINK | -516.66 | 11.3839 | 9.96325 | $734 | -0.051 |
| ATOM | 3529.46 | 1.49903 | 1.3902 | $-384 | +0.048 |
| JUP | 18056.4 | 0.241363 | 0.256265 | $269 | +0.046 |
| DOT | 5309.02 | 0.856105 | 0.82796 | $-149 | +0.043 |
| TAO | -19.9244 | 222.943 | 210.16 | $255 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.4882 | $202 | -0.040 |
| FIL | 5349.98 | 0.760721 | 0.742 | $-100 | +0.039 |
| OP | 40810.8 | 0.0988434 | 0.09436 | $-183 | +0.038 |
| PENDLE | -2505.4 | 1.58072 | 1.49015 | $227 | -0.037 |
| NEAR | -997.435 | 3.77006 | 3.40065 | $368 | -0.033 |
| INJ | -450.711 | 8.02739 | 7.4429 | $263 | -0.033 |
| WLD | 10593 | 0.336516 | 0.30071 | $-379 | +0.031 |
| ENA | -22442.6 | 0.146112 | 0.13896 | $161 | -0.031 |
| kPEPE | -646727 | 0.00494453 | 0.004403 | $350 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 82.0405 | $868 | -0.073 |
| TRX | 20882.3 | 0.317091 | 0.315935 | $-24 | +0.067 |
| XRP | -3730.4 | 1.0602 | 1.03405 | $98 | -0.039 |
| ADA | -22351.2 | 0.190675 | 0.17097 | $440 | -0.039 |
| PENDLE | -2522.74 | 1.58296 | 1.49015 | $234 | -0.038 |
| BNB | 5.68814 | 705.088 | 659.095 | $-262 | +0.038 |
| UNI | 597.681 | 7.08573 | 6.02145 | $-636 | +0.037 |
| kPEPE | 803792 | 0.00484647 | 0.004403 | $-356 | +0.036 |
| INJ | 456.574 | 7.51016 | 7.4429 | $-31 | +0.035 |
| ZEC | 3.22979 | 1213.4 | 969.905 | $-786 | +0.032 |
| ARB | 21235.8 | 0.1621 | 0.14532 | $-356 | +0.031 |
| WLD | -9976.36 | 0.34434 | 0.30071 | $435 | -0.031 |
| DOT | -3218.79 | 0.900177 | 0.82796 | $232 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.2348 | $408 | -0.027 |
| RENDER | 1752.76 | 1.59944 | 1.4882 | $-195 | +0.027 |
| SEI | -60987.3 | 0.0472943 | 0.04253 | $291 | -0.026 |
| XLM | -16417.2 | 0.145302 | 0.14488 | $7 | -0.024 |
| OP | 24123.2 | 0.107142 | 0.09436 | $-308 | +0.023 |
| AAVE | 18.1796 | 131.483 | 112.065 | $-353 | +0.021 |
| LINK | 197.081 | 11.0659 | 9.96325 | $-217 | +0.020 |
| ATOM | -1308.69 | 1.43333 | 1.3902 | $56 | -0.019 |
| APT | -3237.04 | 0.626089 | 0.5467 | $257 | -0.018 |
| FIL | -2342.25 | 0.799282 | 0.742 | $134 | -0.018 |
| AVAX | 200.164 | 8.39772 | 8.373 | $-5 | +0.017 |
| DOGE | -21412 | 0.0688816 | 0.066216 | $57 | -0.014 |
| ETH | 0.675911 | 2238.8 | 2016.35 | $-150 | +0.014 |
| JUP | 4429.65 | 0.25565 | 0.256265 | $3 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.064947 | $94 | -0.011 |
| TAO | -4.19115 | 219.804 | 210.16 | $40 | -0.009 |
| NEAR | -155.553 | 3.72366 | 3.40065 | $50 | -0.005 |
| BTC | 0.00601366 | 75900.3 | 72800.5 | $-19 | +0.004 |
| TIA | -1057.05 | 0.421737 | 0.405 | $18 | -0.004 |
| BCH | -1.1862 | 228.028 | 226.76 | $2 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.349255 | $-29 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.087304 | $-10 | +0.002 |
| LTC | -3.23093 | 54.9478 | 49.066 | $19 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.13896 | $-27 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.79992 | $-6 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -4.16, "AVAX": -2.27, "BNB": -1.93, "LDO": -2.34, "SEI": -2.07}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -4.16, "AVAX": -2.27, "BNB": -1.93, "LDO": -2.34, "SEI": -2.07}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 9.6, "HYPE": 8.7, "PENDLE": 10.9, "UNI": 10.0, "ZEC": 11.4}, "last_day": "2026-11-12", "top_apr": {"BTC": 10.4, "CRV": 10.1, "ETC": 10.5, "HBAR": 10.9, "LDO": 11.0, "PENDLE": 10.9, "RENDER": 10.9, "TAO": 9.7, "UNI": 10.0, "ZEC": 11.4}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-11T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-11T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-12"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-11T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-12T12:06 | claude_discretionary | BTC | sell | $9,694 | 72796.9 | discretionary: Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liq |
| 2026-11-12T07:04 | crash_rebound | ZEC | sell | $7,601 | 987.166 | crash-rebound exit t2 (24h) |
| 2026-11-12T00:05 | xs_momentum | JUP | buy | $1,584 | 0.25565 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | TIA | buy | $1,092 | 0.415502 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | ATOM | buy | $1,027 | 1.39385 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | TRX | buy | $993 | 0.316736 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | RENDER | buy | $990 | 1.55644 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | PENDLE | sell | $1,028 | 1.52406 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | ETH | sell | $1,602 | 2055.95 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | BCH | sell | $1,712 | 228.028 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_momentum | XRP | sell | $1,837 | 1.0511 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | xs_funding | ALGO | buy | $5,080 | 0.0892297 | daily xs-funding (Binance recon) |
| 2026-11-12T00:05 | xs_funding | ZEC | buy | $3,272 | 973.086 | daily xs-funding (Binance recon) |
| 2026-11-12T00:05 | xs_funding | RENDER | sell | $1,049 | 1.5556 | daily xs-funding (Binance recon) |
| 2026-11-12T00:05 | xs_funding | ENA | sell | $3,279 | 0.146112 | daily xs-funding (Binance recon) |
| 2026-11-12T00:05 | xs_funding | ETC | sell | $6,544 | 6.59963 | daily xs-funding (Binance recon) |
| 2026-11-12T00:05 | trend_breakout_majors | BTC | sell | $2,247 | 73949.8 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | trend_breakout_majors | SOL | sell | $3,539 | 83.4303 | signal 2026-11-11 23h |
| 2026-11-12T00:05 | trend_breakout_majors | ETH | sell | $3,579 | 2055.95 | signal 2026-11-11 23h |
| 2026-11-11T19:04 | crash_rebound | ZEC | sell | $7,306 | 988.376 | crash-rebound exit t1 (24h) |
