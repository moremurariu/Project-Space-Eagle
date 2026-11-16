# Paper trading status — 2026-11-16 23:04 UTC

BTC 65,216 · ETH 1,823.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $84,107 | -15.89% | +0.24% | -13.02% | -22.40% | -7.48 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,133 | -2.87% | +0.01% | -1.18% | -4.33% | -5.75 | 0.05 | +0.05 | 1 | $53 | $-97 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.55 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.70 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,039 | +0.04% | +0.01% | -0.09% | -0.26% | 2.04 | 0.92 | +0.00 | 6 | $296 | $323 |
| trend_breakout_majors | $114,081 | +14.08% | +0.08% | +12.28% | -5.48% | 7.95 | 0.53 | -0.53 | 3 | $116 | $172 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.51 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.68 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,427 | +0.43% | -0.49% | +1.05% | -1.56% | 1.41 | 0.92 | -0.01 | 20 | $581 | $163 |
| xs_momentum | $98,526 | -1.47% | -0.52% | -1.86% | -2.98% | -3.52 | 0.90 | -0.03 | 37 | $195 | $-3 |
| xyz_weekend_fade | $103,286 | +3.29% | +2.04% | +2.04% | -1.21% | 6.26 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,113,600 on $1,100,000 → +1.24%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.96 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65234.2 | $-15,823 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 65216.5 | $-804 | +0.052 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 233.128 | 83.2675 | 70.263 | $-3,032 | +0.164 |
| HYPE | -233.128 | 82.7772 | 70.2425 | $2,922 | -0.164 |
| ZEC | -17.4301 | 1175 | 907.895 | $4,656 | -0.158 |
| ZEC/spot | 17.43 | 1175.27 | 907.76 | $-4,663 | +0.158 |
| PENDLE | -9671.02 | 1.81759 | 1.43485 | $3,701 | -0.139 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.436 | $-3,699 | +0.139 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.309603 | 75630.7 | 65216.5 | $3,224 | -0.177 |
| SOL | -281.278 | 91.199 | 71.4445 | $5,557 | -0.176 |
| ETH | -10.8509 | 2187.58 | 1823.25 | $3,953 | -0.173 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 65216.5 | $450 | -0.094 |
| TRX | 28470.1 | 0.309116 | 0.305025 | $-116 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.955025 | $1,617 | -0.071 |
| ADA | 36975.2 | 0.155319 | 0.152685 | $-97 | +0.056 |
| BCH | 23.4344 | 238.546 | 208.91 | $-695 | +0.049 |
| SEI | 123581 | 0.0376927 | 0.038909 | $150 | +0.048 |
| TAO | -25.0244 | 220.686 | 190.9 | $745 | -0.048 |
| LINK | -516.66 | 11.3839 | 9.12145 | $1,169 | -0.047 |
| APT | 9654.61 | 0.457585 | 0.4522 | $-52 | +0.043 |
| JUP | 18056.4 | 0.241363 | 0.241535 | $3 | +0.043 |
| LDO | -14834.4 | 0.308439 | 0.27598 | $482 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.37185 | $523 | -0.038 |
| PENDLE | -2505.4 | 1.58072 | 1.43485 | $365 | -0.036 |
| ARB | 27302.4 | 0.151348 | 0.13018 | $-578 | +0.035 |
| TIA | 10070 | 0.345986 | 0.35219 | $62 | +0.035 |
| OP | 44197.2 | 0.0926854 | 0.07934 | $-590 | +0.035 |
| ZEC | -3.83105 | 862.252 | 907.895 | $-175 | -0.035 |
| NEAR | -997.435 | 3.77006 | 3.1628 | $606 | -0.031 |
| WLD | 10593 | 0.336516 | 0.26197 | $-790 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.114705 | $705 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.305025 | $-260 | +0.080 |
| SOL | -87.2593 | 91.9832 | 71.4445 | $1,792 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.955025 | $433 | -0.060 |
| RENDER | 3386.49 | 1.53278 | 1.37185 | $-545 | +0.047 |
| BTC | -0.0697602 | 67434.2 | 65216.5 | $155 | -0.046 |
| AVAX | 607.041 | 7.89216 | 7.1104 | $-475 | +0.044 |
| INJ | 607.666 | 7.33527 | 6.82945 | $-307 | +0.042 |
| ARB | 29979.9 | 0.152074 | 0.13018 | $-656 | +0.040 |
| JUP | 15235.8 | 0.263176 | 0.241535 | $-330 | +0.037 |
| PENDLE | -2522.74 | 1.58296 | 1.43485 | $374 | -0.037 |
| DOT | -4637.48 | 0.841601 | 0.72751 | $529 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.00389 | $-769 | +0.032 |
| BCH | -14.0941 | 212.452 | 208.91 | $50 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.05896 | $357 | -0.028 |
| ETH | -1.46269 | 1817.83 | 1823.25 | $-8 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26197 | $822 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.20599 | $730 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.07934 | $-671 | +0.019 |
| UNI | 341.169 | 7.08573 | 5.1358 | $-665 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.3284 | $36 | +0.016 |
| ADA | -9776.31 | 0.190675 | 0.152685 | $371 | -0.015 |
| TAO | 7.68451 | 200.727 | 190.9 | $-76 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.60268 | $460 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.038909 | $279 | -0.013 |
| BNB | -2.14595 | 596.735 | 592.22 | $10 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.13724 | $70 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27598 | $35 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.114705 | $5 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057261 | $221 | -0.010 |
| AAVE | 8.4283 | 131.483 | 107.91 | $-199 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35219 | $16 | +0.009 |
| APT | -1385.4 | 0.626089 | 0.4522 | $241 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 907.895 | $-190 | +0.006 |
| NEAR | 164.539 | 3.21111 | 3.1628 | $-8 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.30531 | $-58 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.079514 | $-27 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.71791 | $-11 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.35, "ARB": -1.88, "JUP": -2.84, "OP": -1.12, "TIA": -1.53}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.35, "ARB": -1.88, "JUP": -2.84, "OP": -1.12, "TIA": -1.53}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 6.5, "PENDLE": 11.0, "ZEC": 9.7}, "last_day": "2026-11-16", "top_apr": {"BNB": 7.0, "BTC": 8.3, "CRV": 9.4, "ETC": 6.4, "HYPE": 6.5, "LDO": 11.0, "NEAR": 9.0, "PENDLE": 11.0, "TAO": 6.6, "ZEC": 9.7}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-16"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-15T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:NVDA | buy | $5,556 | 156.433 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:NBIS | buy | $4,168 | 106.914 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:AAPL | buy | $3,538 | 352.589 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SKHX | buy | $3,309 | 818.436 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SKHY | buy | $3,058 | 122.29 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:TSLA | buy | $2,777 | 379.687 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SPCX | buy | $2,747 | 137.077 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:IBM | buy | $2,552 | 241.97 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:XYZ100 | buy | $2,523 | 27226.9 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:MSFT | buy | $2,510 | 518.336 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:GOOGL | buy | $2,254 | 284.166 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SMSN | buy | $2,213 | 144.77 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:SP500 | buy | $2,023 | 7420.82 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:AMZN | buy | $2,011 | 257.975 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:EWY | buy | $1,936 | 136.711 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:ANT | buy | $1,410 | 97.1781 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:UNITREE | buy | $1,388 | 101.823 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:DRAM | buy | $1,384 | 39.323 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:META | buy | $1,379 | 626.391 | weekend fade exit (Mon US close) |
| 2026-11-16T21:04 | xyz_weekend_fade | xyz:MRVL | buy | $963 | 145.505 | weekend fade exit (Mon US close) |
