# Paper trading status — 2026-11-18 16:04 UTC

BTC 63,016 · ETH 1,765.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $81,270 | -18.73% | -2.05% | -14.41% | -22.40% | -8.22 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,961 | -3.04% | -0.11% | -0.85% | -4.33% | -5.79 | 0.05 | +0.05 | 1 | $53 | $-100 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.22% | -0.69% | 0.53 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.52 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,060 | +0.06% | +0.04% | -0.07% | -0.26% | 2.11 | 0.90 | -0.00 | 6 | $296 | $347 |
| trend_breakout_majors | $114,894 | +14.89% | +0.47% | +11.08% | -5.48% | 7.77 | 0.51 | -0.51 | 3 | $120 | $180 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.33 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.50 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,312 | +0.31% | +0.24% | -0.43% | -1.64% | 1.04 | 0.91 | +0.02 | 20 | $608 | $196 |
| xs_momentum | $98,817 | -1.18% | -0.15% | +0.09% | -2.98% | -2.53 | 0.91 | -0.00 | 36 | $208 | $5 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.92 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,114,439 on $1,100,000 → +1.31%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.95 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 63036.8 | $-18,660 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 63016.5 | $-973 | +0.050 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 927.91 | $-4,311 | +0.162 |
| ZEC | -17.4301 | 1175 | 927.835 | $4,308 | -0.162 |
| HYPE/spot | 233.128 | 83.2675 | 65.8234 | $-4,067 | +0.153 |
| HYPE | -233.128 | 82.7772 | 65.812 | $3,955 | -0.153 |
| PENDLE | -9671.02 | 1.81759 | 1.39075 | $4,128 | -0.134 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.3915 | $-4,129 | +0.134 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.357592 | 74182.5 | 63016.5 | $3,993 | -0.196 |
| ETH | -10.8509 | 2187.58 | 1765.55 | $4,579 | -0.167 |
| SOL | -233.753 | 91.199 | 73.2255 | $4,201 | -0.149 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 63016.5 | $767 | -0.091 |
| TRX | 28470.1 | 0.309116 | 0.30923 | $3 | +0.088 |
| ADA | 36975.2 | 0.155319 | 0.1507 | $-171 | +0.056 |
| AAVE | -44.8727 | 112.188 | 107.605 | $206 | -0.048 |
| TAO | -25.0244 | 220.686 | 191.55 | $729 | -0.048 |
| SEI | 123581 | 0.0376927 | 0.038753 | $131 | +0.048 |
| PENDLE | -3236.78 | 1.54767 | 1.39075 | $508 | -0.045 |
| JUP | 18056.4 | 0.241363 | 0.248055 | $121 | +0.045 |
| OP | 58569.1 | 0.0891484 | 0.07624 | $-756 | +0.045 |
| UNI | -836.22 | 5.16466 | 5.10205 | $52 | -0.043 |
| APT | 9654.61 | 0.457585 | 0.4294 | $-272 | +0.041 |
| LDO | -14834.4 | 0.308439 | 0.26868 | $590 | -0.040 |
| WLD | 15387.1 | 0.31305 | 0.25875 | $-836 | +0.040 |
| RENDER | -2753.67 | 1.56167 | 1.40585 | $429 | -0.039 |
| INJ | 541.319 | 7.41463 | 6.9749 | $-238 | +0.038 |
| ZEC | -3.83105 | 862.252 | 927.835 | $-251 | -0.035 |
| TIA | 10070 | 0.345986 | 0.34705 | $11 | +0.035 |
| kPEPE | 893923 | 0.0038915 | 0.003708 | $-164 | +0.033 |
| NEAR | -997.435 | 3.77006 | 3.2076 | $561 | -0.032 |
| ENA | -22442.6 | 0.146112 | 0.10892 | $835 | -0.024 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30923 | $-151 | +0.081 |
| BTC | -0.104843 | 66543.3 | 63016.5 | $370 | -0.067 |
| SOL | -73.6612 | 91.9832 | 73.2255 | $1,382 | -0.055 |
| RENDER | 3386.49 | 1.53278 | 1.40585 | $-430 | +0.048 |
| XRP | -4815.21 | 1.02442 | 0.975575 | $235 | -0.048 |
| INJ | 607.666 | 7.33527 | 6.9749 | $-219 | +0.043 |
| AVAX | 607.041 | 7.89216 | 6.9513 | $-571 | +0.043 |
| BNB | -6.85128 | 594.407 | 580.855 | $93 | -0.040 |
| DOGE | -65877 | 0.0643319 | 0.058657 | $374 | -0.039 |
| ETH | -2.15636 | 1819.35 | 1765.55 | $116 | -0.039 |
| ARB | 29979.9 | 0.152074 | 0.12693 | $-754 | +0.039 |
| JUP | 15235.8 | 0.263176 | 0.248055 | $-230 | +0.038 |
| ATOM | 2372.89 | 1.31579 | 1.2786 | $-88 | +0.031 |
| kPEPE | 803792 | 0.00484647 | 0.003708 | $-915 | +0.030 |
| BCH | -14.0941 | 212.452 | 206.45 | $85 | -0.029 |
| WLD | -9976.36 | 0.34434 | 0.25875 | $854 | -0.026 |
| CRV | -11195.5 | 0.271234 | 0.19885 | $810 | -0.023 |
| DOT | -3092.91 | 0.841601 | 0.71669 | $386 | -0.022 |
| OP | 24123.2 | 0.107142 | 0.07624 | $-745 | +0.019 |
| PENDLE | -1253.55 | 1.58296 | 1.39075 | $241 | -0.018 |
| UNI | 341.169 | 7.08573 | 5.10205 | $-677 | +0.018 |
| TAO | 7.68451 | 200.727 | 191.55 | $-71 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.57178 | $533 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.13898 | $-15 | +0.013 |
| LDO | 4201.67 | 0.267639 | 0.26868 | $4 | +0.011 |
| HBAR | -16510.2 | 0.0706575 | 0.056204 | $239 | -0.009 |
| ENA | -8394.07 | 0.115324 | 0.10892 | $54 | -0.009 |
| AAVE | 8.4283 | 131.483 | 107.605 | $-201 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.34705 | $3 | +0.009 |
| ZEC | -0.884394 | 901.025 | 927.835 | $-24 | -0.008 |
| APT | -1385.4 | 0.626089 | 0.4294 | $272 | -0.006 |
| NEAR | 164.539 | 3.21111 | 3.2076 | $-1 | +0.005 |
| ADA | -3148.06 | 0.190675 | 0.1507 | $126 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.299565 | $-61 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.038753 | $35 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.71173 | $-12 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -3.5, "FIL": -3.42, "INJ": -4.44, "NEAR": -4.15, "UNI": -2.65}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -3.5, "FIL": -3.42, "INJ": -4.44, "NEAR": -4.15, "UNI": -2.65}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.6, "PENDLE": 11.0, "ZEC": 9.2}, "last_day": "2026-11-18", "top_apr": {"BNB": 7.6, "BTC": 7.6, "CRV": 7.4, "DOT": 7.2, "HYPE": 8.6, "LDO": 11.0, "NEAR": 9.0, "PENDLE": 11.0, "TAO": 6.6, "ZEC": 9.2}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-17T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-17T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-18"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-17T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-18T00:04 | xs_momentum | XRP | buy | $1,396 | 0.980332 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | XLM | buy | $1,073 | 0.141294 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | ADA | buy | $1,028 | 0.155059 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | SOL | buy | $1,020 | 75.0127 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | BTC | sell | $1,155 | 64839.3 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | ETH | sell | $1,264 | 1822.56 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_momentum | BNB | sell | $1,402 | 596.608 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | xs_funding | XRP | buy | $7,286 | 0.980314 | daily xs-funding (Binance recon) |
| 2026-11-18T00:04 | xs_funding | INJ | buy | $4,014 | 7.41463 | daily xs-funding (Binance recon) |
| 2026-11-18T00:04 | xs_funding | OP | buy | $1,125 | 0.0782715 | daily xs-funding (Binance recon) |
| 2026-11-18T00:04 | xs_funding | ARB | sell | $3,640 | 0.133333 | daily xs-funding (Binance recon) |
| 2026-11-18T00:04 | xs_funding | AAVE | sell | $5,034 | 112.188 | daily xs-funding (Binance recon) |
| 2026-11-18T00:04 | trend_breakout_majors | SOL | buy | $3,565 | 75.0113 | signal 2026-11-17 23h |
| 2026-11-18T00:04 | trend_breakout_majors | BTC | sell | $3,112 | 64839.3 | signal 2026-11-17 23h |
| 2026-11-17T00:04 | xs_momentum | PENDLE | buy | $1,821 | 1.43505 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | ATOM | buy | $1,581 | 1.33336 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | XLM | buy | $1,400 | 0.137445 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | SEI | buy | $1,137 | 0.0389305 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | DOT | buy | $1,129 | 0.730795 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | ALGO | sell | $166 | 0.0793595 | signal 2026-11-16 23h |
