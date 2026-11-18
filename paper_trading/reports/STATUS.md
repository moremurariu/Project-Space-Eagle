# Paper trading status — 2026-11-18 02:04 UTC

BTC 64,286 · ETH 1,800.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $82,914 | -17.09% | -0.87% | -12.66% | -22.40% | -7.59 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,060 | -2.94% | -0.05% | -0.75% | -4.33% | -5.69 | 0.05 | +0.05 | 1 | $53 | $-99 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.53 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.58 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,051 | +0.05% | +0.01% | -0.12% | -0.26% | 2.06 | 0.93 | -0.00 | 6 | $296 | $339 |
| trend_breakout_majors | $113,811 | +13.81% | -0.19% | +9.84% | -5.48% | 7.40 | 0.53 | -0.53 | 3 | $120 | $176 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.39 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.55 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,014 | +0.01% | -0.51% | +0.48% | -1.64% | 0.42 | 0.93 | +0.02 | 20 | $608 | $185 |
| xs_momentum | $99,087 | -0.91% | +0.45% | -1.05% | -2.98% | -1.97 | 0.93 | -0.00 | 36 | $208 | $3 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 6.03 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,113,417 on $1,100,000 → +1.22%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.94 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64310 | $-17,016 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64286.5 | $-875 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 950.015 | $-3,926 | +0.166 |
| ZEC | -17.4301 | 1175 | 950.005 | $3,922 | -0.166 |
| HYPE/spot | 233.128 | 83.2675 | 68.6655 | $-3,404 | +0.160 |
| HYPE | -233.128 | 82.7772 | 68.6505 | $3,293 | -0.160 |
| PENDLE | -9671.02 | 1.81759 | 1.4208 | $3,837 | -0.137 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.42144 | $-3,840 | +0.137 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.357592 | 74182.5 | 64286.5 | $3,539 | -0.202 |
| ETH | -10.8509 | 2187.58 | 1800.65 | $4,199 | -0.172 |
| SOL | -233.753 | 91.199 | 74.2695 | $3,957 | -0.153 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64286.5 | $584 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.30771 | $-40 | +0.088 |
| ADA | 36975.2 | 0.155319 | 0.15277 | $-94 | +0.056 |
| AAVE | -44.8727 | 112.188 | 111.085 | $49 | -0.050 |
| TAO | -25.0244 | 220.686 | 195.105 | $640 | -0.049 |
| SEI | 123581 | 0.0376927 | 0.039493 | $222 | +0.049 |
| JUP | 18056.4 | 0.241363 | 0.258525 | $310 | +0.047 |
| PENDLE | -3236.78 | 1.54767 | 1.4208 | $411 | -0.046 |
| OP | 58569.1 | 0.0891484 | 0.0769 | $-717 | +0.045 |
| UNI | -836.22 | 5.16466 | 5.2119 | $-40 | -0.044 |
| APT | 9654.61 | 0.457585 | 0.4418 | $-152 | +0.043 |
| LDO | -14834.4 | 0.308439 | 0.27324 | $522 | -0.041 |
| WLD | 15387.1 | 0.31305 | 0.26308 | $-769 | +0.040 |
| RENDER | -2753.67 | 1.56167 | 1.4432 | $326 | -0.040 |
| INJ | 541.319 | 7.41463 | 7.31575 | $-54 | +0.040 |
| ZEC | -3.83105 | 862.252 | 950.005 | $-336 | -0.036 |
| TIA | 10070 | 0.345986 | 0.35066 | $47 | +0.035 |
| kPEPE | 893923 | 0.0038915 | 0.00381 | $-73 | +0.034 |
| NEAR | -997.435 | 3.77006 | 3.35095 | $418 | -0.033 |
| ENA | -22442.6 | 0.146112 | 0.11437 | $712 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30771 | $-191 | +0.080 |
| BTC | -0.104843 | 66543.3 | 64286.5 | $237 | -0.068 |
| SOL | -73.6612 | 91.9832 | 74.2695 | $1,305 | -0.055 |
| RENDER | 3386.49 | 1.53278 | 1.4432 | $-303 | +0.049 |
| XRP | -4815.21 | 1.02442 | 0.970935 | $258 | -0.047 |
| INJ | 607.666 | 7.33527 | 7.31575 | $-12 | +0.045 |
| AVAX | 607.041 | 7.89216 | 7.1649 | $-441 | +0.044 |
| BNB | -6.85128 | 594.407 | 591.24 | $22 | -0.041 |
| JUP | 15235.8 | 0.263176 | 0.258525 | $-71 | +0.040 |
| ARB | 29979.9 | 0.152074 | 0.13094 | $-634 | +0.040 |
| DOGE | -65877 | 0.0643319 | 0.059182 | $339 | -0.039 |
| ETH | -2.15636 | 1819.35 | 1800.65 | $40 | -0.039 |
| ATOM | 2372.89 | 1.31579 | 1.3151 | $-2 | +0.031 |
| kPEPE | 803792 | 0.00484647 | 0.00381 | $-833 | +0.031 |
| BCH | -14.0941 | 212.452 | 208.625 | $54 | -0.030 |
| WLD | -9976.36 | 0.34434 | 0.26308 | $811 | -0.026 |
| CRV | -11195.5 | 0.271234 | 0.20625 | $728 | -0.023 |
| DOT | -3092.91 | 0.841601 | 0.731 | $342 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.0769 | $-730 | +0.019 |
| PENDLE | -1253.55 | 1.58296 | 1.4208 | $203 | -0.018 |
| UNI | 341.169 | 7.08573 | 5.2119 | $-639 | +0.018 |
| TAO | 7.68451 | 200.727 | 195.105 | $-43 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.59515 | $478 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.13972 | $-9 | +0.013 |
| LDO | 4201.67 | 0.267639 | 0.27324 | $24 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.11437 | $8 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057324 | $220 | -0.010 |
| AAVE | 8.4283 | 131.483 | 111.085 | $-172 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35066 | $12 | +0.009 |
| ZEC | -0.884394 | 901.025 | 950.005 | $-43 | -0.008 |
| APT | -1385.4 | 0.626089 | 0.4418 | $255 | -0.006 |
| NEAR | 164.539 | 3.21111 | 3.35095 | $23 | +0.006 |
| ADA | -3148.06 | 0.190675 | 0.15277 | $119 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.305995 | $-57 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.039493 | $32 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.72626 | $-11 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.47, "ARB": -1.13, "ENA": -1.63, "PENDLE": -1.19, "TIA": -1.3}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.47, "ARB": -1.13, "ENA": -1.63, "PENDLE": -1.19, "TIA": -1.3}}`
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
