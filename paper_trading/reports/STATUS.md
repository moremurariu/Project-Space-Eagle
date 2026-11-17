# Paper trading status — 2026-11-17 05:04 UTC

BTC 64,866 · ETH 1,830.8 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,667 | -16.33% | +0.27% | -13.30% | -22.40% | -7.60 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,106 | -2.89% | +0.01% | -1.16% | -4.33% | -5.76 | 0.05 | +0.05 | 1 | $53 | $-98 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.55 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.67 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,014 | +0.01% | -0.01% | -0.11% | -0.26% | 1.78 | 0.93 | -0.00 | 6 | $296 | $327 |
| trend_breakout_majors | $113,705 | +13.70% | -0.57% | +11.81% | -5.48% | 7.67 | 0.53 | -0.53 | 3 | $116 | $172 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.48 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.65 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,425 | +0.42% | -0.58% | +0.84% | -1.56% | 1.38 | 0.94 | -0.01 | 20 | $593 | $167 |
| xs_momentum | $99,012 | -0.99% | +0.24% | -1.19% | -2.98% | -2.25 | 0.91 | -0.01 | 36 | $204 | $-3 |
| xyz_weekend_fade | $103,286 | +3.29% | +1.18% | +2.04% | -1.21% | 6.21 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,113,655 on $1,100,000 → +1.24%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.95 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64893 | $-16,263 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64865.5 | $-831 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 233.128 | 83.2675 | 70.263 | $-3,032 | +0.164 |
| HYPE | -233.128 | 82.7772 | 70.251 | $2,920 | -0.164 |
| ZEC | -17.4301 | 1175 | 924.86 | $4,360 | -0.161 |
| ZEC/spot | 17.43 | 1175.27 | 924.21 | $-4,376 | +0.161 |
| PENDLE | -9671.02 | 1.81759 | 1.45145 | $3,541 | -0.140 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4508 | $-3,556 | +0.140 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| SOL | -281.278 | 91.199 | 72.8745 | $5,154 | -0.180 |
| BTC | -0.309603 | 75630.7 | 64865.5 | $3,333 | -0.177 |
| ETH | -10.8509 | 2187.58 | 1830.85 | $3,871 | -0.175 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64865.5 | $501 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.306115 | $-85 | +0.087 |
| XRP | -7432.17 | 1.17255 | 0.964205 | $1,548 | -0.071 |
| ADA | 36975.2 | 0.155319 | 0.154245 | $-40 | +0.057 |
| TAO | -25.0244 | 220.686 | 193.97 | $669 | -0.048 |
| SEI | 123581 | 0.0376927 | 0.039175 | $183 | +0.048 |
| PENDLE | -3236.78 | 1.54767 | 1.45145 | $311 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.248495 | $129 | +0.045 |
| APT | 9654.61 | 0.457585 | 0.4594 | $18 | +0.044 |
| UNI | -836.22 | 5.16466 | 5.2698 | $-88 | -0.044 |
| LDO | -14834.4 | 0.308439 | 0.27962 | $428 | -0.041 |
| WLD | 15387.1 | 0.31305 | 0.26468 | $-744 | +0.041 |
| RENDER | -2753.67 | 1.56167 | 1.3917 | $468 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13262 | $-511 | +0.036 |
| TIA | 10070 | 0.345986 | 0.35776 | $119 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08056 | $-536 | +0.035 |
| ZEC | -3.83105 | 862.252 | 924.86 | $-240 | -0.035 |
| kPEPE | 893923 | 0.0038915 | 0.003923 | $28 | +0.035 |
| NEAR | -997.435 | 3.77006 | 3.26965 | $499 | -0.032 |
| ENA | -22442.6 | 0.146112 | 0.116305 | $669 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.306115 | $-232 | +0.080 |
| SOL | -87.2593 | 91.9832 | 72.8745 | $1,667 | -0.064 |
| XRP | -6239.14 | 1.02442 | 0.964205 | $376 | -0.061 |
| BTC | -0.0870366 | 66891.9 | 64865.5 | $176 | -0.057 |
| RENDER | 3386.49 | 1.53278 | 1.3917 | $-478 | +0.048 |
| INJ | 607.666 | 7.33527 | 7.3839 | $30 | +0.045 |
| AVAX | 607.041 | 7.89216 | 7.1883 | $-427 | +0.044 |
| ARB | 29979.9 | 0.152074 | 0.13262 | $-583 | +0.040 |
| DOGE | -65877 | 0.0643319 | 0.059111 | $344 | -0.039 |
| JUP | 15235.8 | 0.263176 | 0.248495 | $-224 | +0.038 |
| kPEPE | 803792 | 0.00484647 | 0.003923 | $-742 | +0.032 |
| ATOM | 2372.89 | 1.31579 | 1.3284 | $30 | +0.032 |
| BCH | -14.0941 | 212.452 | 208.92 | $50 | -0.030 |
| ETH | -1.46269 | 1817.83 | 1830.85 | $-19 | -0.027 |
| BNB | -4.50154 | 593.258 | 593.905 | $-3 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26468 | $795 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.2066 | $724 | -0.023 |
| DOT | -3092.91 | 0.841601 | 0.74016 | $314 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.08056 | $-641 | +0.020 |
| PENDLE | -1253.55 | 1.58296 | 1.45145 | $165 | -0.018 |
| UNI | 341.169 | 7.08573 | 5.2698 | $-620 | +0.018 |
| ADA | -9776.31 | 0.190675 | 0.154245 | $356 | -0.015 |
| TAO | 7.68451 | 200.727 | 193.97 | $-52 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.60958 | $444 | -0.014 |
| LDO | 4201.67 | 0.267639 | 0.27962 | $50 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.116305 | $-8 | -0.010 |
| AAVE | 8.4283 | 131.483 | 112.95 | $-156 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057223 | $222 | -0.010 |
| TIA | 2451.15 | 0.345826 | 0.35776 | $29 | +0.009 |
| ZEC | -0.884394 | 901.025 | 924.86 | $-21 | -0.008 |
| APT | -1385.4 | 0.626089 | 0.4594 | $231 | -0.006 |
| NEAR | 164.539 | 3.21111 | 3.26965 | $10 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.30819 | $-56 | +0.002 |
| XLM | 1456 | 0.137445 | 0.139025 | $2 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.039175 | $33 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.7275 | $-11 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": 0.87, "BNB": 0.78, "BTC": 0.64, "CRV": 0.88, "TRX": 0.13}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": 0.87, "BNB": 0.78, "BTC": 0.64, "CRV": 0.88, "TRX": 0.13}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.5, "PENDLE": 11.0, "ZEC": 9.1}, "last_day": "2026-11-17", "top_apr": {"BNB": 7.6, "BTC": 7.8, "CRV": 7.4, "HBAR": 6.4, "HYPE": 8.5, "LDO": 11.0, "NEAR": 9.0, "PENDLE": 11.0, "TAO": 6.5, "ZEC": 9.1}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-16T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-16T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-17"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-16T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-17T00:04 | xs_momentum | PENDLE | buy | $1,821 | 1.43505 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | ATOM | buy | $1,581 | 1.33336 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | XLM | buy | $1,400 | 0.137445 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | SEI | buy | $1,137 | 0.0389305 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | DOT | buy | $1,129 | 0.730795 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | ALGO | sell | $166 | 0.0793595 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | DOGE | sell | $1,115 | 0.0587851 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | BTC | sell | $1,118 | 64702.3 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | ZEC | sell | $1,358 | 901.025 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_momentum | BNB | sell | $1,390 | 590.09 | signal 2026-11-16 23h |
| 2026-11-17T00:04 | xs_funding | LINK | buy | $4,703 | 9.10204 | daily xs-funding (Binance recon) |
| 2026-11-17T00:04 | xs_funding | kPEPE | buy | $3,479 | 0.0038915 | daily xs-funding (Binance recon) |
| 2026-11-17T00:04 | xs_funding | WLD | buy | $1,252 | 0.261203 | daily xs-funding (Binance recon) |
| 2026-11-17T00:04 | xs_funding | PENDLE | sell | $1,049 | 1.43444 | daily xs-funding (Binance recon) |
| 2026-11-17T00:04 | xs_funding | UNI | sell | $4,319 | 5.16466 | daily xs-funding (Binance recon) |
| 2026-11-17T00:04 | xs_funding | BCH | sell | $4,874 | 207.975 | daily xs-funding (Binance recon) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:NVDA | buy | $5,556 | 156.433 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:NBIS | buy | $4,168 | 106.914 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:AAPL | buy | $3,538 | 352.589 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SKHX | buy | $3,309 | 818.436 | weekend fade exit (Mon US close) |
