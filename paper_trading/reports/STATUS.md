# Paper trading status — 2026-11-18 07:04 UTC

BTC 64,660 · ETH 1,811.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,385 | -16.62% | +0.01% | -12.48% | -22.40% | -7.30 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,088 | -2.91% | -0.00% | -0.77% | -4.33% | -5.60 | 0.05 | +0.05 | 1 | $53 | $-99 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.07% | -0.69% | 0.53 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.56 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,060 | +0.06% | +0.04% | -0.08% | -0.26% | 2.13 | 0.93 | +0.00 | 6 | $296 | $342 |
| trend_breakout_majors | $113,439 | +13.44% | -0.29% | +9.70% | -5.48% | 7.17 | 0.53 | -0.53 | 3 | $120 | $178 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.37 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.53 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,062 | +0.06% | -0.48% | +0.31% | -1.64% | 0.52 | 0.94 | +0.02 | 20 | $608 | $191 |
| xs_momentum | $98,892 | -1.11% | -0.13% | -0.97% | -2.98% | -2.41 | 0.93 | -0.00 | 36 | $208 | $4 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.99 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,112,936 on $1,100,000 → +1.18%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.94 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64674.6 | $-16,545 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64660.5 | $-846 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 955.03 | $-3,839 | +0.166 |
| ZEC | -17.4301 | 1175 | 954.925 | $3,836 | -0.166 |
| HYPE/spot | 233.128 | 83.2675 | 68.9475 | $-3,338 | +0.161 |
| HYPE | -233.128 | 82.7772 | 68.9375 | $3,226 | -0.161 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4299 | $-3,758 | +0.138 |
| PENDLE | -9671.02 | 1.81759 | 1.4286 | $3,762 | -0.138 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.357592 | 74182.5 | 64660.5 | $3,405 | -0.204 |
| ETH | -10.8509 | 2187.58 | 1811.45 | $4,081 | -0.173 |
| SOL | -233.753 | 91.199 | 74.7965 | $3,834 | -0.154 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64660.5 | $530 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.309185 | $2 | +0.088 |
| ADA | 36975.2 | 0.155319 | 0.153895 | $-53 | +0.057 |
| AAVE | -44.8727 | 112.188 | 110.925 | $57 | -0.050 |
| TAO | -25.0244 | 220.686 | 195.935 | $619 | -0.049 |
| SEI | 123581 | 0.0376927 | 0.039566 | $232 | +0.049 |
| JUP | 18056.4 | 0.241363 | 0.258895 | $317 | +0.047 |
| PENDLE | -3236.78 | 1.54767 | 1.4286 | $385 | -0.046 |
| OP | 58569.1 | 0.0891484 | 0.07751 | $-682 | +0.045 |
| UNI | -836.22 | 5.16466 | 5.2177 | $-44 | -0.044 |
| APT | 9654.61 | 0.457585 | 0.4435 | $-136 | +0.043 |
| LDO | -14834.4 | 0.308439 | 0.27474 | $500 | -0.041 |
| WLD | 15387.1 | 0.31305 | 0.2645 | $-747 | +0.041 |
| INJ | 541.319 | 7.41463 | 7.3409 | $-40 | +0.040 |
| RENDER | -2753.67 | 1.56167 | 1.4333 | $353 | -0.039 |
| ZEC | -3.83105 | 862.252 | 954.925 | $-355 | -0.037 |
| TIA | 10070 | 0.345986 | 0.35338 | $74 | +0.036 |
| NEAR | -997.435 | 3.77006 | 3.38435 | $385 | -0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003761 | $-117 | +0.034 |
| ENA | -22442.6 | 0.146112 | 0.113595 | $730 | -0.025 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.309185 | $-152 | +0.081 |
| BTC | -0.104843 | 66543.3 | 64660.5 | $197 | -0.069 |
| SOL | -73.6612 | 91.9832 | 74.7965 | $1,266 | -0.056 |
| RENDER | 3386.49 | 1.53278 | 1.4333 | $-337 | +0.049 |
| XRP | -4815.21 | 1.02442 | 0.987575 | $177 | -0.048 |
| INJ | 607.666 | 7.33527 | 7.3409 | $3 | +0.045 |
| AVAX | 607.041 | 7.89216 | 7.1699 | $-438 | +0.044 |
| BNB | -6.85128 | 594.407 | 592.41 | $14 | -0.041 |
| JUP | 15235.8 | 0.263176 | 0.258895 | $-65 | +0.040 |
| ARB | 29979.9 | 0.152074 | 0.131 | $-632 | +0.040 |
| DOGE | -65877 | 0.0643319 | 0.05942 | $324 | -0.040 |
| ETH | -2.15636 | 1819.35 | 1811.45 | $17 | -0.039 |
| ATOM | 2372.89 | 1.31579 | 1.3086 | $-17 | +0.031 |
| kPEPE | 803792 | 0.00484647 | 0.003761 | $-872 | +0.031 |
| BCH | -14.0941 | 212.452 | 208.175 | $60 | -0.030 |
| WLD | -9976.36 | 0.34434 | 0.2645 | $797 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.205 | $742 | -0.023 |
| DOT | -3092.91 | 0.841601 | 0.73082 | $343 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.07751 | $-715 | +0.019 |
| PENDLE | -1253.55 | 1.58296 | 1.4286 | $193 | -0.018 |
| UNI | 341.169 | 7.08573 | 5.2177 | $-637 | +0.018 |
| TAO | 7.68451 | 200.727 | 195.935 | $-37 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.60165 | $463 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.141145 | $4 | +0.013 |
| LDO | 4201.67 | 0.267639 | 0.27474 | $30 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.113595 | $15 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057304 | $220 | -0.010 |
| AAVE | 8.4283 | 131.483 | 110.925 | $-173 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35338 | $19 | +0.009 |
| ZEC | -0.884394 | 901.025 | 954.925 | $-48 | -0.009 |
| APT | -1385.4 | 0.626089 | 0.4435 | $253 | -0.006 |
| NEAR | 164.539 | 3.21111 | 3.38435 | $29 | +0.006 |
| ADA | -3148.06 | 0.190675 | 0.153895 | $116 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.30815 | $-56 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.039566 | $32 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.73165 | $-10 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.51, "CRV": -0.63, "ENA": -0.79, "RENDER": -0.35, "kPEPE": -1.06}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.51, "CRV": -0.63, "ENA": -0.79, "RENDER": -0.35, "kPEPE": -1.06}}`
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
