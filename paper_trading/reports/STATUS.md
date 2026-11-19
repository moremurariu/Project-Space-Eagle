# Paper trading status — 2026-11-19 14:04 UTC

BTC 63,982 · ETH 1,812.3 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $82,537 | -17.46% | +0.13% | -12.12% | -22.75% | -7.24 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,035 | -2.97% | +0.01% | -0.60% | -4.36% | -5.50 | 0.05 | +0.05 | 1 | $53 | $-100 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.51 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.43 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,082 | +0.08% | +0.02% | -0.11% | -0.26% | 2.24 | 0.93 | +0.00 | 6 | $296 | $359 |
| trend_breakout_majors | $113,550 | +13.55% | -0.45% | +8.36% | -5.48% | 6.88 | 0.55 | -0.55 | 3 | $121 | $189 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.25 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.41 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,507 | +0.51% | +0.22% | -0.98% | -1.64% | 1.37 | 0.93 | +0.02 | 20 | $608 | $211 |
| xs_momentum | $99,121 | -0.88% | +0.27% | +0.85% | -2.98% | -1.75 | 0.91 | +0.02 | 35 | $214 | $7 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.76 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,113,688 on $1,100,000 → +1.24%**
Combined BTC exposure (7d realised, 4h returns): beta -0.085 (≈ -0.94 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64017.9 | $-17,393 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 63981.5 | $-899 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 946.89 | $-3,981 | +0.165 |
| ZEC | -17.4301 | 1175 | 946.575 | $3,982 | -0.165 |
| HYPE/spot | 233.128 | 83.2675 | 68.685 | $-3,400 | +0.160 |
| HYPE | -233.128 | 82.7772 | 68.6235 | $3,300 | -0.160 |
| PENDLE | -9671.02 | 1.81759 | 1.44485 | $3,605 | -0.140 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.445 | $-3,612 | +0.140 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.400699 | 72966.3 | 63981.5 | $3,600 | -0.226 |
| ETH | -10.8509 | 2187.58 | 1812.35 | $4,072 | -0.173 |
| SOL | -233.753 | 91.199 | 75.1605 | $3,749 | -0.155 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 63981.5 | $628 | -0.092 |
| TRX | 28470.1 | 0.309116 | 0.31239 | $93 | +0.088 |
| ADA | 36975.2 | 0.155319 | 0.155355 | $1 | +0.057 |
| TAO | -25.0244 | 220.686 | 199.325 | $535 | -0.050 |
| AAVE | -44.8727 | 112.188 | 109.915 | $102 | -0.049 |
| JUP | 18056.4 | 0.241363 | 0.2671 | $465 | +0.048 |
| SEI | 123581 | 0.0376927 | 0.038766 | $133 | +0.048 |
| PENDLE | -3236.78 | 1.54767 | 1.44485 | $333 | -0.047 |
| OP | 58569.1 | 0.0891484 | 0.07655 | $-738 | +0.045 |
| UNI | -836.22 | 5.16466 | 5.13975 | $21 | -0.043 |
| APT | 9654.61 | 0.457585 | 0.4327 | $-240 | +0.042 |
| WLD | 15387.1 | 0.31305 | 0.26671 | $-713 | +0.041 |
| RENDER | -2753.67 | 1.56167 | 1.4407 | $333 | -0.039 |
| LDO | -14834.4 | 0.308439 | 0.26719 | $612 | -0.039 |
| INJ | 541.319 | 7.41463 | 7.26245 | $-82 | +0.039 |
| ZEC | -3.83105 | 862.252 | 946.575 | $-323 | -0.036 |
| TIA | 10070 | 0.345986 | 0.35124 | $53 | +0.035 |
| kPEPE | 893923 | 0.0038915 | 0.003954 | $56 | +0.035 |
| NEAR | -997.435 | 3.77006 | 3.38635 | $383 | -0.034 |
| ENA | -22442.6 | 0.146112 | 0.111125 | $785 | -0.025 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 29868.9 | 0.314369 | 0.31239 | $-59 | +0.094 |
| BTC | -0.104843 | 66543.3 | 63981.5 | $269 | -0.068 |
| SOL | -73.6612 | 91.9832 | 75.1605 | $1,239 | -0.056 |
| BNB | -8.83459 | 591.479 | 592.895 | $-13 | -0.053 |
| RENDER | 3386.49 | 1.53278 | 1.4407 | $-312 | +0.049 |
| XRP | -4815.21 | 1.02442 | 0.992935 | $152 | -0.048 |
| INJ | 607.666 | 7.33527 | 7.26245 | $-44 | +0.045 |
| AVAX | 607.041 | 7.89216 | 7.1554 | $-447 | +0.044 |
| JUP | 15235.8 | 0.263176 | 0.2671 | $60 | +0.041 |
| DOGE | -65877 | 0.0643319 | 0.059694 | $306 | -0.040 |
| ETH | -2.15636 | 1819.35 | 1812.35 | $15 | -0.039 |
| ARB | 29979.9 | 0.152074 | 0.12724 | $-745 | +0.038 |
| ATOM | 2372.89 | 1.31579 | 1.3043 | $-27 | +0.031 |
| BCH | -14.0941 | 212.452 | 209.455 | $42 | -0.030 |
| DOT | -3092.91 | 0.841601 | 0.72214 | $369 | -0.023 |
| CRV | -11195.5 | 0.271234 | 0.19866 | $812 | -0.022 |
| kPEPE | 522031 | 0.00484647 | 0.003954 | $-466 | +0.021 |
| NEAR | 530.46 | 3.26892 | 3.38635 | $62 | +0.018 |
| UNI | 341.169 | 7.08573 | 5.13975 | $-664 | +0.018 |
| WLD | -5872.05 | 0.34434 | 0.26671 | $456 | -0.016 |
| TAO | 7.68451 | 200.727 | 199.325 | $-11 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.58343 | $506 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.13827 | $-22 | +0.013 |
| LDO | 4201.67 | 0.267639 | 0.26719 | $-2 | +0.011 |
| HBAR | -16510.2 | 0.0706575 | 0.056499 | $234 | -0.009 |
| ENA | -8394.07 | 0.115324 | 0.111125 | $35 | -0.009 |
| AAVE | 8.4283 | 131.483 | 109.915 | $-182 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35124 | $13 | +0.009 |
| ZEC | -0.884394 | 901.025 | 946.575 | $-40 | -0.008 |
| APT | -1385.4 | 0.626089 | 0.4327 | $268 | -0.006 |
| OP | 7393.63 | 0.107142 | 0.07655 | $-226 | +0.006 |
| PENDLE | -340.102 | 1.58296 | 1.44485 | $47 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.306005 | $-57 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.038766 | $35 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.73096 | $-10 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ATOM": -0.53, "LDO": -0.48, "LTC": -0.29, "SEI": -0.28, "UNI": -1.32}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ATOM": -0.53, "LDO": -0.48, "LTC": -0.29, "SEI": -0.28, "UNI": -1.32}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.6, "PENDLE": 11.0, "ZEC": 9.2}, "last_day": "2026-11-19", "top_apr": {"BNB": 6.7, "BTC": 7.7, "CRV": 7.4, "DOT": 7.1, "HBAR": 6.4, "HYPE": 8.6, "LDO": 11.0, "NEAR": 8.3, "PENDLE": 11.0, "ZEC": 9.2}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-18T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-18T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-19"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-18T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-19T00:04 | xs_momentum | PENDLE | buy | $1,324 | 1.4493 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | TRX | buy | $1,254 | 0.309765 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | NEAR | buy | $1,206 | 3.29491 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | WLD | buy | $1,069 | 0.260435 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | ADA | buy | $476 | 0.151193 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | kPEPE | sell | $1,055 | 0.0037445 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | BNB | sell | $1,153 | 581.362 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | OP | sell | $1,271 | 0.07595 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | trend_breakout_majors | BTC | sell | $2,710 | 62877.4 | signal 2026-11-18 23h |
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
