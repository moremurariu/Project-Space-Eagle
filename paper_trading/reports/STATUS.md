# Paper trading status — 2026-11-13 17:04 UTC

BTC 68,620 · ETH 1,924.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $88,502 | -11.50% | -4.41% | -9.55% | -15.51% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,342 | -2.66% | -0.22% | -1.29% | -3.96% | nan | 0.05 | +0.05 | 1 | $53 | $-94 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.17% | -0.69% | nan | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | -0.04% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,163 | +0.16% | -0.02% | +0.10% | -0.19% | nan | 1.39 | -0.00 | 10 | $233 | $296 |
| trend_breakout_majors | $108,406 | +8.41% | +2.97% | +8.97% | -5.48% | nan | 0.72 | -0.72 | 3 | $110 | $163 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,555 | +0.56% | -1.02% | +0.60% | -1.13% | nan | 0.93 | +0.09 | 20 | $518 | $105 |
| xs_momentum | $98,487 | -1.51% | +0.24% | -2.07% | -2.98% | nan | 0.83 | -0.02 | 38 | $169 | $-9 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,106,280 on $1,100,000 → +0.57%**
Combined BTC exposure (7d realised, 4h returns): beta -0.045 (≈ -0.49 books of BTC), corr -0.75

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 68637.6 | $-11,428 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 68619.5 | $-598 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.53975 | $2,687 | -0.149 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.54 | $-2,694 | +0.149 |
| HYPE/spot | 201.997 | 85.5551 | 72.4295 | $-2,651 | +0.146 |
| HYPE | -202.014 | 85.5972 | 72.4055 | $2,665 | -0.146 |
| UNI | -2471.47 | 6.90012 | 5.74205 | $2,862 | -0.142 |
| UNI/spot | 2468.37 | 6.89946 | 5.7455 | $-2,848 | +0.142 |
| ZEC | -14.3511 | 1242.13 | 945.375 | $4,259 | -0.135 |
| ZEC/spot | 14.3484 | 1242.39 | 945.13 | $-4,265 | +0.135 |
| ENA | -93083.1 | 0.187964 | 0.134745 | $4,954 | -0.125 |
| ENA/spot | 93002.9 | 0.188053 | 0.13477 | $-4,955 | +0.125 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -366.515 | 91.199 | 79.4835 | $4,294 | -0.269 |
| ETH | -12.8804 | 2187.58 | 1924.45 | $3,389 | -0.229 |
| BTC | -0.356716 | 75630.7 | 68619.5 | $2,501 | -0.226 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.313095 | $113 | +0.089 |
| XLM | 44454.5 | 0.151093 | 0.14669 | $-196 | +0.065 |
| XRP | -6275.1 | 1.21983 | 1.01555 | $1,282 | -0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.062703 | $-239 | +0.058 |
| TAO | -25.0244 | 220.686 | 214.28 | $160 | -0.053 |
| BCH | 23.4344 | 238.546 | 222.245 | $-382 | +0.052 |
| LINK | -516.66 | 11.3839 | 9.85345 | $791 | -0.051 |
| ATOM | 3529.46 | 1.49903 | 1.3864 | $-398 | +0.049 |
| JUP | 18056.4 | 0.241363 | 0.260475 | $345 | +0.047 |
| LDO | -14834.4 | 0.308439 | 0.30022 | $122 | -0.044 |
| DOT | 5309.02 | 0.856105 | 0.82238 | $-179 | +0.043 |
| RENDER | -2753.67 | 1.56167 | 1.5149 | $129 | -0.041 |
| ARB | 27302.4 | 0.151348 | 0.14538 | $-163 | +0.039 |
| PENDLE | -2505.4 | 1.58072 | 1.53975 | $103 | -0.038 |
| FIL | 5349.98 | 0.760721 | 0.72043 | $-216 | +0.038 |
| INJ | -450.711 | 8.02739 | 8.0892 | $-28 | -0.036 |
| NEAR | -997.435 | 3.77006 | 3.4969 | $272 | -0.035 |
| WLD | 10593 | 0.336516 | 0.29438 | $-446 | +0.031 |
| ENA | -22442.6 | 0.146112 | 0.134745 | $255 | -0.030 |
| kPEPE | -646727 | 0.00494453 | 0.00443 | $333 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 79.4835 | $1,091 | -0.070 |
| TRX | 20882.3 | 0.317091 | 0.313095 | $-83 | +0.066 |
| XRP | -4826.67 | 1.05611 | 1.01555 | $196 | -0.050 |
| PENDLE | -2522.74 | 1.58296 | 1.53975 | $109 | -0.039 |
| INJ | 456.574 | 7.51016 | 8.0892 | $264 | +0.038 |
| BNB | 5.68814 | 705.088 | 630.41 | $-425 | +0.036 |
| kPEPE | 803792 | 0.00484647 | 0.00443 | $-335 | +0.036 |
| UNI | 597.681 | 7.08573 | 5.74205 | $-803 | +0.035 |
| AVAX | 376.81 | 8.37846 | 8.2807 | $-37 | +0.032 |
| ARB | 21235.8 | 0.1621 | 0.14538 | $-355 | +0.031 |
| WLD | -9976.36 | 0.34434 | 0.29438 | $498 | -0.030 |
| ADA | -16388.2 | 0.190675 | 0.17018 | $336 | -0.028 |
| RENDER | 1752.76 | 1.59944 | 1.5149 | $-148 | +0.027 |
| DOT | -3218.79 | 0.900177 | 0.82238 | $250 | -0.027 |
| SEI | -60987.3 | 0.0472943 | 0.043193 | $250 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.22999 | $462 | -0.026 |
| XLM | -16417.2 | 0.145302 | 0.14669 | $-23 | -0.024 |
| OP | 24123.2 | 0.107142 | 0.0927 | $-348 | +0.023 |
| AAVE | 18.1796 | 131.483 | 110.565 | $-380 | +0.020 |
| ZEC | 1.94807 | 1213.4 | 945.375 | $-522 | +0.019 |
| APT | -3237.04 | 0.626089 | 0.5368 | $289 | -0.018 |
| FIL | -2342.25 | 0.799282 | 0.72043 | $185 | -0.017 |
| BCH | -6.47026 | 225.866 | 222.245 | $23 | -0.015 |
| DOGE | -21412 | 0.0688816 | 0.064807 | $87 | -0.014 |
| BTC | -0.0200793 | 71453.9 | 68619.5 | $57 | -0.014 |
| ETH | 0.675911 | 2238.8 | 1924.45 | $-212 | +0.013 |
| JUP | 4429.65 | 0.25565 | 0.260475 | $21 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.062703 | $131 | -0.011 |
| LINK | 77.5637 | 11.0659 | 9.85345 | $-94 | +0.008 |
| ATOM | -429.872 | 1.43333 | 1.3864 | $20 | -0.006 |
| TAO | 2.54822 | 211.979 | 214.28 | $6 | +0.006 |
| NEAR | -155.553 | 3.72366 | 3.4969 | $35 | -0.006 |
| TIA | -1057.05 | 0.421737 | 0.39997 | $23 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.339135 | $-35 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.085832 | $-14 | +0.002 |
| LTC | -3.23093 | 54.9478 | 48.9835 | $19 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.134745 | $-30 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.79934 | $-6 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -2.6, "CRV": -2.12, "ENA": -2.31, "TIA": -2.45, "kPEPE": -2.42}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -2.6, "CRV": -2.12, "ENA": -2.31, "TIA": -2.45, "kPEPE": -2.42}}`
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
