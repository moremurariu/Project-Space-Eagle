# Paper trading status — 2026-11-09 21:04 UTC

BTC 74,930 · ETH 2,129.8 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $96,660 | -3.34% | -0.99% | -2.42% | -8.59% | nan | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $98,286 | -1.71% | -0.26% | -1.51% | -3.37% | nan | 0.25 | +0.25 | 1 | $43 | $-82 |
| crash_rebound | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.25% | -0.20% | nan | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,145 | +0.14% | +0.02% | +0.14% | -0.19% | nan | 1.47 | +0.00 | 10 | $225 | $236 |
| trend_breakout_majors | $101,464 | +1.46% | +1.55% | +0.40% | -5.48% | nan | 0.59 | -0.59 | 3 | $97 | $120 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.02% | -0.18% | nan | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | nan | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $99,446 | -0.55% | -0.20% | +0.14% | -1.13% | nan | 0.97 | +0.07 | 20 | $418 | $85 |
| xs_momentum | $100,285 | +0.29% | -0.45% | +0.43% | -0.92% | nan | 0.84 | -0.01 | 38 | $135 | $-24 |
| xyz_weekend_fade | $101,220 | +1.22% | +1.22% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,101,080 on $1,100,000 → +0.10%**
Combined BTC exposure (7d realised, 4h returns): beta -0.044 (≈ -0.49 books of BTC), corr -0.69

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 74956 | $-3,270 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.325465 | 77535.3 | 74930.5 | $-848 | +0.248 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -9671.02 | 1.81759 | 1.59495 | $2,153 | -0.154 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.59595 | $-2,153 | +0.154 |
| HYPE/spot | 201.997 | 85.5551 | 75.156 | $-2,101 | +0.152 |
| HYPE | -202.014 | 85.5972 | 75.1285 | $2,115 | -0.152 |
| UNI/spot | 2136.3 | 7.02501 | 7.0685 | $93 | +0.151 |
| UNI | -2133.42 | 7.03009 | 7.06355 | $-71 | -0.150 |
| ENA | -93083.1 | 0.187964 | 0.15521 | $3,049 | -0.144 |
| ENA/spot | 93002.9 | 0.188053 | 0.155314 | $-3,045 | +0.144 |
| ZEC/spot | 11.6568 | 1287.44 | 1171.59 | $-1,351 | +0.136 |
| ZEC | -11.65 | 1287.49 | 1170.85 | $1,359 | -0.136 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -251.977 | 94.3829 | 86.6915 | $1,938 | -0.215 |
| BTC | -0.271518 | 76194.7 | 74930.5 | $343 | -0.201 |
| ETH | -8.50484 | 2246.43 | 2129.75 | $992 | -0.179 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.316405 | $208 | +0.091 |
| XRP | -6275.1 | 1.21983 | 1.09385 | $791 | -0.069 |
| XLM | 44454.5 | 0.151093 | 0.147815 | $-146 | +0.066 |
| ETC | -937.414 | 6.88048 | 6.8609 | $18 | -0.065 |
| LTC | -121.169 | 49.941 | 51.5845 | $-199 | -0.063 |
| HBAR | 92374.3 | 0.0652853 | 0.067281 | $184 | +0.062 |
| BCH | 23.4344 | 238.546 | 239.725 | $28 | +0.056 |
| LINK | -516.66 | 11.3839 | 10.6705 | $369 | -0.055 |
| ATOM | 3529.46 | 1.49903 | 1.3755 | $-436 | +0.049 |
| SEI | 104421 | 0.0484475 | 0.044469 | $-415 | +0.047 |
| JUP | 18056.4 | 0.241363 | 0.25571 | $259 | +0.046 |
| TAO | -19.9244 | 222.943 | 223.24 | $-6 | -0.045 |
| APT | 7323.07 | 0.569334 | 0.5858 | $121 | +0.043 |
| LDO | -10898 | 0.325861 | 0.32811 | $-25 | -0.036 |
| WLD | 10593 | 0.336516 | 0.32035 | $-171 | +0.034 |
| RENDER | -2079.13 | 1.56364 | 1.62135 | $-120 | -0.034 |
| kPEPE | -646727 | 0.00494453 | 0.005155 | $-136 | -0.034 |
| TIA | 6583.75 | 0.474572 | 0.43225 | $-279 | +0.029 |
| ENA | -17260.6 | 0.163688 | 0.15521 | $146 | -0.027 |
| NEAR | -660.139 | 3.86026 | 3.62215 | $157 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -87.2593 | 91.9832 | 86.6915 | $462 | -0.075 |
| TRX | 17747.4 | 0.317154 | 0.316405 | $-13 | +0.056 |
| ATOM | -3097.68 | 1.43333 | 1.3755 | $179 | -0.042 |
| UNI | 597.681 | 7.08573 | 7.06355 | $-13 | +0.042 |
| kPEPE | 803792 | 0.00484647 | 0.005155 | $248 | +0.041 |
| ADA | -22351.2 | 0.190675 | 0.17842 | $274 | -0.040 |
| BNB | 5.68814 | 705.088 | 690.5 | $-83 | +0.039 |
| SEI | -87205.6 | 0.0472943 | 0.044469 | $246 | -0.039 |
| ZEC | 3.22979 | 1213.4 | 1170.85 | $-137 | +0.038 |
| ETH | 1.45521 | 2238.8 | 2129.75 | $-159 | +0.031 |
| PENDLE | -1848.04 | 1.60447 | 1.59495 | $18 | -0.029 |
| CRV | -11195.5 | 0.271234 | 0.26238 | $99 | -0.029 |
| BCH | 11.4397 | 277.3 | 239.725 | $-430 | +0.027 |
| TAO | -11.3929 | 219.804 | 223.24 | $-39 | -0.025 |
| ARB | 14157.1 | 0.162612 | 0.17549 | $182 | +0.025 |
| OP | 24123.2 | 0.107142 | 0.10092 | $-150 | +0.024 |
| INJ | 290.053 | 7.16191 | 8.14725 | $286 | +0.024 |
| LINK | 197.081 | 11.0659 | 10.6705 | $-78 | +0.021 |
| WLD | -6204.89 | 0.360162 | 0.32035 | $247 | -0.020 |
| APT | -3237.04 | 0.626089 | 0.5858 | $130 | -0.019 |
| JUP | -7068.37 | 0.253165 | 0.25571 | $-18 | -0.018 |
| XRP | 1596.65 | 1.27041 | 1.09385 | $-282 | +0.017 |
| TIA | -3686.1 | 0.421737 | 0.43225 | $-39 | -0.016 |
| DOT | -1684.82 | 0.940458 | 0.86647 | $125 | -0.015 |
| AAVE | 8.98333 | 131.06 | 132.335 | $11 | +0.012 |
| ENA | 7483.97 | 0.179237 | 0.15521 | $-180 | +0.012 |
| HBAR | -16510.2 | 0.0706575 | 0.067281 | $56 | -0.011 |
| ETC | -159.905 | 7.02745 | 6.8609 | $27 | -0.011 |
| ALGO | -8833.48 | 0.0922081 | 0.092563 | $-3 | -0.008 |
| XLM | -5134.89 | 0.145831 | 0.147815 | $-10 | -0.008 |
| FIL | -873.911 | 0.86441 | 0.77413 | $79 | -0.007 |
| NEAR | -155.553 | 3.72366 | 3.62215 | $16 | -0.006 |
| RENDER | -323.133 | 1.55071 | 1.62135 | $-23 | -0.005 |
| BTC | 0.00601366 | 75900.3 | 74930.5 | $-6 | +0.004 |
| DOGE | -4200.09 | 0.0715214 | 0.070002 | $6 | -0.003 |
| ONDO | 657.524 | 0.392809 | 0.373155 | $-13 | +0.002 |
| LTC | -3.23093 | 54.9478 | 51.5845 | $11 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.84749 | $-3 | +0.001 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-08 08:10: hold BTC 0.25, ETH 0. BTC range-bound 75-76k for 48h (no rule hit). Fear & Greed 26-37 (fear, contrarian support) vs persistent put skew (BTC 7d/30d RR25 -4.9/-4.2, ETH -6.0) and the aftermath of the record $6.3B Binance hack (CFO resigned, staged withdrawal reopenings, hackers swapping loot into BTC) as tail risk. No edge to add or cut. Rules unchanged: BTC < 72k -> cut to 0.10; BTC reclaims 78k -> back to 0.30."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -2.28, "AVAX": -1.95, "DOT": -2.15, "ZEC": -2.35, "kPEPE": -2.28}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -2.28, "AVAX": -1.95, "DOT": -2.15, "ZEC": -2.35, "kPEPE": -2.28}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"ENA": 10.0, "HYPE": 11.6, "PENDLE": 10.4, "UNI": 11.0, "ZEC": 12.6}, "last_day": "2026-11-09", "top_apr": {"BTC": 10.3, "ENA": 10.0, "ETH": 10.0, "HBAR": 10.9, "HYPE": 11.6, "LDO": 11.0, "PENDLE": 10.4, "RENDER": 11.0, "UNI": 11.0, "ZEC": 12.6}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-09"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-08T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:ANT | buy | $10,799 | 104.02 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:CXMT | buy | $6,614 | 6.35614 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:MRNA | buy | $5,579 | 270.107 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:SPCX | buy | $4,204 | 142.122 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:ZHIPU | buy | $3,739 | 56.9704 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:META | buy | $3,501 | 640.912 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:AMD | buy | $2,294 | 509.643 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:UNITREE | buy | $2,173 | 105.77 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:SP500 | buy | $2,046 | 7550.63 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:AMZN | buy | $1,729 | 267.258 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:AAPL | buy | $1,482 | 350.353 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:EWY | buy | $1,431 | 142.582 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:XYZ100 | buy | $1,422 | 27910.9 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:STRC | buy | $1,349 | 99.967 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:TSLA | buy | $1,262 | 377.474 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:MSFT | buy | $1,140 | 544.262 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:BABA | buy | $856 | 105.601 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:MU | buy | $579 | 734.512 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:SMSN | buy | $563 | 150.825 | weekend fade exit (Mon US close) |
| 2026-11-09T21:04 | xyz_weekend_fade | xyz:DRAM | buy | $370 | 41.9026 | weekend fade exit (Mon US close) |
