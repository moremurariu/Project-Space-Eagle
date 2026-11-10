# Paper trading status — 2026-11-10 14:07 UTC

BTC 73,813 · ETH 2,069.1 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $95,243 | -4.76% | -0.92% | -4.32% | -8.59% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,917 | -2.08% | -0.25% | -2.01% | -3.37% | nan | 0.25 | +0.25 | 1 | $43 | $-87 |
| crash_rebound | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,173 | +0.17% | +0.07% | +0.15% | -0.19% | nan | 1.46 | +0.00 | 10 | $225 | $251 |
| trend_breakout_majors | $103,127 | +3.13% | +1.41% | +2.85% | -5.48% | nan | 0.64 | -0.64 | 3 | $101 | $126 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $99,608 | -0.39% | +0.08% | +0.38% | -1.13% | nan | 0.99 | +0.07 | 20 | $441 | $98 |
| xs_momentum | $100,521 | +0.52% | +0.18% | +0.56% | -0.96% | nan | 0.82 | -0.02 | 38 | $143 | $-30 |
| xyz_weekend_fade | $101,220 | +1.22% | +1.49% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,102,800 on $1,100,000 → +0.25%**
Combined BTC exposure (7d realised, 4h returns): beta -0.044 (≈ -0.48 books of BTC), corr -0.68

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 73858.8 | $-4,687 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.325465 | 77535.3 | 73813 | $-1,211 | +0.245 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 9662.31 | 1.81881 | 1.5795 | $-2,312 | +0.152 |
| PENDLE | -9671.02 | 1.81759 | 1.57765 | $2,320 | -0.152 |
| UNI/spot | 2136.3 | 7.02501 | 7.0426 | $38 | +0.150 |
| UNI | -2133.42 | 7.03009 | 7.03685 | $-14 | -0.150 |
| HYPE/spot | 201.997 | 85.5551 | 73.402 | $-2,455 | +0.148 |
| HYPE | -202.014 | 85.5972 | 73.3495 | $2,474 | -0.148 |
| ENA | -93083.1 | 0.187964 | 0.154105 | $3,152 | -0.143 |
| ENA/spot | 93002.9 | 0.188053 | 0.15421 | $-3,147 | +0.143 |
| ZEC/spot | 11.6568 | 1287.44 | 1174.89 | $-1,312 | +0.137 |
| ZEC | -11.65 | 1287.49 | 1174.35 | $1,318 | -0.137 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -294.856 | 93.2594 | 84.1675 | $2,681 | -0.241 |
| ETH | -10.0314 | 2229.09 | 2069.05 | $1,605 | -0.201 |
| BTC | -0.271518 | 76194.7 | 73813 | $647 | -0.194 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.31937 | $292 | +0.091 |
| XRP | -6275.1 | 1.21983 | 1.06585 | $966 | -0.067 |
| XLM | 44454.5 | 0.151093 | 0.144405 | $-297 | +0.064 |
| ETC | -937.414 | 6.88048 | 6.818 | $59 | -0.064 |
| LTC | -121.169 | 49.941 | 51.084 | $-138 | -0.062 |
| HBAR | 92374.3 | 0.0652853 | 0.066543 | $116 | +0.062 |
| BCH | 23.4344 | 238.546 | 234.12 | $-104 | +0.055 |
| LINK | -516.66 | 11.3839 | 10.4035 | $507 | -0.054 |
| ATOM | 3529.46 | 1.49903 | 1.3832 | $-409 | +0.049 |
| JUP | 18056.4 | 0.241363 | 0.253755 | $224 | +0.046 |
| DOT | 5309.02 | 0.856105 | 0.85407 | $-11 | +0.046 |
| TAO | -19.9244 | 222.943 | 219.185 | $75 | -0.044 |
| APT | 7323.07 | 0.569334 | 0.5764 | $52 | +0.042 |
| FIL | 5349.98 | 0.760721 | 0.7608 | $0 | +0.041 |
| PENDLE | -2505.4 | 1.58072 | 1.57765 | $8 | -0.040 |
| ARB | -20975.9 | 0.17451 | 0.16587 | $181 | -0.035 |
| kPEPE | -646727 | 0.00494453 | 0.005364 | $-271 | -0.035 |
| WLD | 10593 | 0.336516 | 0.31395 | $-239 | +0.033 |
| RENDER | -2079.13 | 1.56364 | 1.57885 | $-32 | -0.033 |
| NEAR | -660.139 | 3.86026 | 3.65865 | $133 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 84.1675 | $682 | -0.073 |
| TRX | 17747.4 | 0.317154 | 0.31937 | $39 | +0.056 |
| kPEPE | 803792 | 0.00484647 | 0.005364 | $416 | +0.043 |
| ATOM | -3097.68 | 1.43333 | 1.3832 | $155 | -0.043 |
| UNI | 597.681 | 7.08573 | 7.03685 | $-29 | +0.042 |
| ADA | -22351.2 | 0.190675 | 0.17656 | $315 | -0.039 |
| BNB | 5.68814 | 705.088 | 685.13 | $-114 | +0.039 |
| SEI | -87205.6 | 0.0472943 | 0.044131 | $276 | -0.038 |
| ZEC | 3.22979 | 1213.4 | 1174.35 | $-126 | +0.038 |
| INJ | 456.574 | 7.51016 | 8.06085 | $251 | +0.037 |
| WLD | -9976.36 | 0.34434 | 0.31395 | $303 | -0.031 |
| ETH | 1.45521 | 2238.8 | 2069.05 | $-247 | +0.030 |
| PENDLE | -1848.04 | 1.60447 | 1.57765 | $50 | -0.029 |
| CRV | -11195.5 | 0.271234 | 0.25762 | $152 | -0.029 |
| DOT | -3218.79 | 0.900177 | 0.85407 | $148 | -0.027 |
| OP | 24123.2 | 0.107142 | 0.09953 | $-184 | +0.024 |
| ARB | 14157.1 | 0.162612 | 0.16587 | $46 | +0.023 |
| AAVE | 18.1796 | 131.483 | 125.13 | $-116 | +0.023 |
| LINK | 197.081 | 11.0659 | 10.4035 | $-131 | +0.020 |
| APT | -3237.04 | 0.626089 | 0.5764 | $161 | -0.019 |
| FIL | -2342.25 | 0.799282 | 0.7608 | $90 | -0.018 |
| TIA | -3686.1 | 0.421737 | 0.43117 | $-35 | -0.016 |
| BCH | 6.32139 | 277.3 | 234.12 | $-273 | +0.015 |
| HBAR | -16510.2 | 0.0706575 | 0.066543 | $68 | -0.011 |
| ETC | -159.905 | 7.02745 | 6.818 | $33 | -0.011 |
| TAO | -4.19115 | 219.804 | 219.185 | $3 | -0.009 |
| XLM | -5134.89 | 0.145831 | 0.144405 | $7 | -0.007 |
| NEAR | -155.553 | 3.72366 | 3.65865 | $10 | -0.006 |
| RENDER | -323.133 | 1.55071 | 1.57885 | $-9 | -0.005 |
| XRP | -423.72 | 1.10109 | 1.06585 | $15 | -0.004 |
| JUP | -1766.46 | 0.253165 | 0.253755 | $-1 | -0.004 |
| BTC | 0.00601366 | 75900.3 | 73813 | $-13 | +0.004 |
| DOGE | -4200.09 | 0.0715214 | 0.069029 | $10 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.371215 | $-14 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.091374 | $-2 | +0.002 |
| LTC | -3.23093 | 54.9478 | 51.084 | $12 | -0.002 |
| ENA | 664.451 | 0.179237 | 0.154105 | $-17 | +0.001 |
| SUI | 66.7499 | 0.886044 | 0.83175 | $-4 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-08 08:10: hold BTC 0.25, ETH 0. BTC range-bound 75-76k for 48h (no rule hit). Fear & Greed 26-37 (fear, contrarian support) vs persistent put skew (BTC 7d/30d RR25 -4.9/-4.2, ETH -6.0) and the aftermath of the record $6.3B Binance hack (CFO resigned, staged withdrawal reopenings, hackers swapping loot into BTC) as tail risk. No edge to add or cut. Rules unchanged: BTC < 72k -> cut to 0.10; BTC reclaims 78k -> back to 0.30."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -1.91, "ETH": -1.84, "RENDER": -1.81, "XLM": -2.3, "XRP": -2.25}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -1.91, "ETH": -1.84, "RENDER": -1.81, "XLM": -2.3, "XRP": -2.25}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 9.8, "HYPE": 9.2, "PENDLE": 10.7, "UNI": 11.0, "ZEC": 12.0}, "last_day": "2026-11-10", "top_apr": {"BTC": 10.2, "ENA": 9.8, "HBAR": 10.9, "LDO": 11.0, "PENDLE": 10.7, "RENDER": 10.9, "SEI": 10.0, "TAO": 10.0, "UNI": 11.0, "ZEC": 12.0}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-09T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-09T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-10"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-09T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-10T00:05 | xs_momentum | TAO | buy | $1,606 | 223.017 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | INJ | buy | $1,352 | 8.11675 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | JUP | buy | $1,336 | 0.252004 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | AAVE | buy | $1,213 | 131.897 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | ALGO | buy | $1,008 | 0.0922958 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | ENA | sell | $1,045 | 0.153175 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | FIL | sell | $1,117 | 0.76052 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | WLD | sell | $1,200 | 0.318308 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | BCH | sell | $1,223 | 239.011 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | DOT | sell | $1,313 | 0.855936 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_momentum | XRP | sell | $2,225 | 1.10109 | signal 2026-11-09 23h |
| 2026-11-10T00:05 | xs_funding | DOT | buy | $4,545 | 0.856105 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | FIL | buy | $4,070 | 0.760721 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | LDO | buy | $3,569 | 0.327523 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | ENA | buy | $2,645 | 0.153248 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | TIA | sell | $2,828 | 0.42948 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | ARB | sell | $3,661 | 0.17451 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | PENDLE | sell | $3,960 | 1.58072 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | xs_funding | SEI | sell | $4,593 | 0.0439826 | daily xs-funding (Binance recon) |
| 2026-11-10T00:05 | trend_breakout_majors | ETH | sell | $3,255 | 2132.44 | signal 2026-11-09 23h |
