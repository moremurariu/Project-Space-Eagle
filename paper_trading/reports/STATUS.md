# Paper trading status — 2026-11-19 05:04 UTC

BTC 63,480 · ETH 1,796.8 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $81,855 | -18.14% | -1.43% | -13.81% | -22.75% | -7.66 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,996 | -3.00% | -0.07% | -0.81% | -4.36% | -5.63 | 0.05 | +0.05 | 1 | $53 | $-100 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.06% | -0.69% | 0.52 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.47 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,063 | +0.06% | +0.02% | -0.11% | -0.26% | 2.10 | 0.92 | -0.00 | 6 | $296 | $354 |
| trend_breakout_majors | $114,064 | +14.06% | +0.40% | +9.82% | -5.48% | 7.20 | 0.55 | -0.55 | 3 | $121 | $186 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.29 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.44 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,097 | +0.10% | +0.14% | -0.98% | -1.64% | 0.56 | 0.93 | +0.02 | 20 | $608 | $207 |
| xs_momentum | $99,167 | -0.83% | +0.26% | +0.58% | -2.98% | -1.68 | 0.90 | +0.02 | 35 | $214 | $7 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.83 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,113,783 on $1,100,000 → +1.25%**
Combined BTC exposure (7d realised, 4h returns): beta -0.085 (≈ -0.93 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 63490 | $-18,075 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 63480.5 | $-937 | +0.050 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC | -17.4301 | 1175 | 933.84 | $4,204 | -0.163 |
| ZEC/spot | 17.43 | 1175.27 | 933.797 | $-4,209 | +0.163 |
| HYPE/spot | 233.128 | 83.2675 | 67.568 | $-3,660 | +0.157 |
| HYPE | -233.128 | 82.7772 | 67.5245 | $3,556 | -0.157 |
| PENDLE | -9671.02 | 1.81759 | 1.45705 | $3,487 | -0.141 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.45688 | $-3,497 | +0.141 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.400699 | 72966.3 | 63480.5 | $3,801 | -0.223 |
| ETH | -10.8509 | 2187.58 | 1796.75 | $4,241 | -0.171 |
| SOL | -233.753 | 91.199 | 74.5275 | $3,897 | -0.153 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 63480.5 | $701 | -0.092 |
| TRX | 28470.1 | 0.309116 | 0.311535 | $69 | +0.089 |
| ADA | 36975.2 | 0.155319 | 0.152645 | $-99 | +0.056 |
| AAVE | -44.8727 | 112.188 | 108.795 | $152 | -0.049 |
| TAO | -25.0244 | 220.686 | 194.685 | $651 | -0.049 |
| SEI | 123581 | 0.0376927 | 0.038975 | $158 | +0.048 |
| JUP | 18056.4 | 0.241363 | 0.26625 | $449 | +0.048 |
| PENDLE | -3236.78 | 1.54767 | 1.45705 | $293 | -0.047 |
| OP | 58569.1 | 0.0891484 | 0.07633 | $-751 | +0.045 |
| UNI | -836.22 | 5.16466 | 5.18355 | $-16 | -0.043 |
| APT | 9654.61 | 0.457585 | 0.43 | $-266 | +0.041 |
| WLD | 15387.1 | 0.31305 | 0.26209 | $-784 | +0.040 |
| LDO | -14834.4 | 0.308439 | 0.27122 | $552 | -0.040 |
| RENDER | -2753.67 | 1.56167 | 1.4278 | $369 | -0.039 |
| INJ | 541.319 | 7.41463 | 6.9868 | $-232 | +0.038 |
| ZEC | -3.83105 | 862.252 | 933.84 | $-274 | -0.036 |
| TIA | 10070 | 0.345986 | 0.3496 | $36 | +0.035 |
| NEAR | -997.435 | 3.77006 | 3.41785 | $351 | -0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003782 | $-98 | +0.034 |
| ENA | -22442.6 | 0.146112 | 0.111895 | $768 | -0.025 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 29868.9 | 0.314369 | 0.311535 | $-85 | +0.094 |
| BTC | -0.104843 | 66543.3 | 63480.5 | $321 | -0.067 |
| SOL | -73.6612 | 91.9832 | 74.5275 | $1,286 | -0.055 |
| BNB | -8.83459 | 591.479 | 586.545 | $44 | -0.052 |
| RENDER | 3386.49 | 1.53278 | 1.4278 | $-356 | +0.049 |
| XRP | -4815.21 | 1.02442 | 0.974515 | $240 | -0.047 |
| AVAX | 607.041 | 7.89216 | 7.0654 | $-502 | +0.043 |
| INJ | 607.666 | 7.33527 | 6.9868 | $-212 | +0.043 |
| JUP | 15235.8 | 0.263176 | 0.26625 | $47 | +0.041 |
| DOGE | -65877 | 0.0643319 | 0.059062 | $347 | -0.039 |
| ETH | -2.15636 | 1819.35 | 1796.75 | $49 | -0.039 |
| ARB | 29979.9 | 0.152074 | 0.12746 | $-738 | +0.039 |
| ATOM | 2372.89 | 1.31579 | 1.3065 | $-22 | +0.031 |
| BCH | -14.0941 | 212.452 | 206.355 | $86 | -0.029 |
| DOT | -3092.91 | 0.841601 | 0.71979 | $377 | -0.022 |
| CRV | -11195.5 | 0.271234 | 0.19834 | $816 | -0.022 |
| kPEPE | 522031 | 0.00484647 | 0.003782 | $-556 | +0.020 |
| NEAR | 530.46 | 3.26892 | 3.41785 | $79 | +0.018 |
| UNI | 341.169 | 7.08573 | 5.18355 | $-649 | +0.018 |
| WLD | -5872.05 | 0.34434 | 0.26209 | $483 | -0.016 |
| TAO | 7.68451 | 200.727 | 194.685 | $-46 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.57006 | $537 | -0.013 |
| XLM | 9051.42 | 0.140675 | 0.136635 | $-37 | +0.012 |
| LDO | 4201.67 | 0.267639 | 0.27122 | $15 | +0.011 |
| ENA | -8394.07 | 0.115324 | 0.111895 | $29 | -0.009 |
| HBAR | -16510.2 | 0.0706575 | 0.055594 | $249 | -0.009 |
| AAVE | 8.4283 | 131.483 | 108.795 | $-191 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.3496 | $9 | +0.009 |
| ZEC | -0.884394 | 901.025 | 933.84 | $-29 | -0.008 |
| APT | -1385.4 | 0.626089 | 0.43 | $272 | -0.006 |
| OP | 7393.63 | 0.107142 | 0.07633 | $-228 | +0.006 |
| PENDLE | -340.102 | 1.58296 | 1.45705 | $43 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.300825 | $-60 | +0.002 |
| SEI | -4111.87 | 0.0472943 | 0.038975 | $34 | -0.002 |
| SUI | 66.7499 | 0.886044 | 0.72331 | $-11 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.78, "ATOM": -0.3, "BCH": -1.05, "CRV": -0.99, "SEI": -0.56}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.78, "ATOM": -0.3, "BCH": -1.05, "CRV": -0.99, "SEI": -0.56}}`
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
