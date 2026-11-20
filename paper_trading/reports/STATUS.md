# Paper trading status — 2026-11-20 16:04 UTC

BTC 58,954 · ETH 1,632.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $76,044 | -23.96% | -7.66% | -14.31% | -26.98% | -9.29 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,647 | -3.35% | -0.39% | -0.73% | -4.62% | -6.05 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.50 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.34 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,075 | +0.08% | +0.01% | -0.16% | -0.26% | 2.09 | 0.80 | -0.00 | 6 | $296 | $370 |
| trend_breakout_majors | $119,748 | +19.75% | +5.46% | +10.77% | -5.48% | 8.81 | 0.55 | -0.55 | 3 | $125 | $195 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.16 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.32 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,192 | +1.19% | +0.67% | +0.50% | -1.64% | 2.53 | 0.84 | +0.01 | 20 | $609 | $240 |
| xs_momentum | $98,876 | -1.12% | -0.02% | +0.39% | -3.01% | -2.06 | 0.85 | -0.04 | 35 | $223 | $16 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.59 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,119,934 on $1,100,000 → +1.81%**
Combined BTC exposure (7d realised, 4h returns): beta -0.088 (≈ -0.97 books of BTC), corr -0.93

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 58989 | $-23,886 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 58953.5 | $-1,286 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE | -233.128 | 82.7772 | 61.4735 | $4,966 | -0.143 |
| HYPE/spot | 233.128 | 83.2675 | 61.4065 | $-5,096 | +0.143 |
| ZEC/spot | 17.43 | 1175.27 | 774.899 | $-6,978 | +0.135 |
| ZEC | -17.4301 | 1175 | 774.545 | $6,980 | -0.135 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.239 | $-5,602 | +0.120 |
| PENDLE | -9671.02 | 1.81759 | 1.23785 | $5,607 | -0.120 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 58953.5 | $5,753 | -0.219 |
| ETH | -12.9689 | 2117.81 | 1632.05 | $6,300 | -0.177 |
| SOL | -265.603 | 89.0415 | 68.091 | $5,565 | -0.151 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.302505 | $-188 | +0.085 |
| BTC | -0.144359 | 68333.1 | 58953.5 | $1,354 | -0.084 |
| ADA | 36975.2 | 0.155319 | 0.13916 | $-597 | +0.051 |
| LDO | -18837.8 | 0.298153 | 0.23685 | $1,155 | -0.044 |
| TAO | -25.0244 | 220.686 | 173.94 | $1,170 | -0.043 |
| AAVE | -44.8727 | 112.188 | 96.59 | $700 | -0.043 |
| SEI | 123581 | 0.0376927 | 0.034669 | $-374 | +0.042 |
| OP | 58569.1 | 0.0891484 | 0.0688 | $-1,192 | +0.040 |
| PENDLE | -3236.78 | 1.54767 | 1.23785 | $1,003 | -0.040 |
| JUP | 18056.4 | 0.241363 | 0.22124 | $-363 | +0.039 |
| UNI | -836.22 | 5.16466 | 4.45635 | $592 | -0.037 |
| APT | 9654.61 | 0.457585 | 0.3752 | $-795 | +0.036 |
| WLD | 15387.1 | 0.31305 | 0.23122 | $-1,259 | +0.035 |
| RENDER | -2753.67 | 1.56167 | 1.2467 | $867 | -0.034 |
| INJ | 541.319 | 7.41463 | 6.12425 | $-699 | +0.033 |
| ENA | -33871.5 | 0.132226 | 0.097681 | $1,170 | -0.033 |
| TIA | 10070 | 0.345986 | 0.31807 | $-281 | +0.032 |
| kPEPE | 893923 | 0.0038915 | 0.00339 | $-448 | +0.030 |
| ZEC | -3.83105 | 862.252 | 774.545 | $336 | -0.029 |
| NEAR | -997.435 | 3.77006 | 2.8253 | $942 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 29868.9 | 0.314369 | 0.302505 | $-354 | +0.091 |
| BTC | -0.122056 | 65912.7 | 58953.5 | $849 | -0.073 |
| SOL | -73.6612 | 91.9832 | 68.091 | $1,760 | -0.051 |
| BNB | -8.83459 | 591.479 | 543.905 | $420 | -0.049 |
| ETH | -2.88908 | 1804.39 | 1632.05 | $498 | -0.048 |
| RENDER | 3386.49 | 1.53278 | 1.2467 | $-969 | +0.043 |
| INJ | 607.666 | 7.33527 | 6.12425 | $-736 | +0.038 |
| DOGE | -65877 | 0.0643319 | 0.053441 | $717 | -0.036 |
| JUP | 15235.8 | 0.263176 | 0.22124 | $-639 | +0.034 |
| XRP | -3611.62 | 1.02442 | 0.906075 | $427 | -0.033 |
| ATOM | 2372.89 | 1.31579 | 1.2074 | $-257 | +0.029 |
| AVAX | 424.946 | 7.89216 | 6.4555 | $-611 | +0.028 |
| BCH | -14.0941 | 212.452 | 188.75 | $334 | -0.027 |
| FIL | -4978.18 | 0.680775 | 0.51878 | $806 | -0.026 |
| TAO | 12.9296 | 197.549 | 173.94 | $-305 | +0.023 |
| ENA | -21996.7 | 0.108918 | 0.097681 | $247 | -0.022 |
| ARB | 18212.8 | 0.152074 | 0.11356 | $-701 | +0.021 |
| CRV | -11195.5 | 0.271234 | 0.17742 | $1,050 | -0.020 |
| AAVE | 18.9438 | 116.539 | 96.59 | $-378 | +0.019 |
| kPEPE | 522031 | 0.00484647 | 0.00339 | $-760 | +0.018 |
| APT | -4613.4 | 0.476746 | 0.3752 | $468 | -0.018 |
| NEAR | 530.46 | 3.26892 | 2.8253 | $-235 | +0.015 |
| WLD | -5872.05 | 0.34434 | 0.23122 | $664 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.12624 | $-131 | +0.012 |
| LDO | 4201.67 | 0.267639 | 0.23685 | $-129 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.050774 | $328 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.034669 | $-81 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.65817 | $220 | -0.008 |
| TIA | 2451.15 | 0.345826 | 0.31807 | $-68 | +0.008 |
| ZEC | -0.884394 | 901.025 | 774.545 | $112 | -0.007 |
| OP | 7393.63 | 0.107142 | 0.0688 | $-283 | +0.005 |
| UNI | 96.3288 | 7.08573 | 4.45635 | $-253 | +0.004 |
| PENDLE | -340.102 | 1.58296 | 1.23785 | $117 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.27309 | $-79 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.6484 | $-16 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -6.25, "CRV": -6.35, "FIL": -7.03, "PENDLE": -7.52, "UNI": -6.61}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -6.25, "CRV": -6.35, "FIL": -7.03, "PENDLE": -7.52, "UNI": -6.61}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 7.8, "PENDLE": 10.8, "ZEC": 9.5}, "last_day": "2026-11-20", "top_apr": {"BNB": 6.9, "BTC": 7.4, "CRV": 7.7, "DOT": 5.9, "HBAR": 7.0, "HYPE": 7.8, "LDO": 10.9, "NEAR": 6.7, "PENDLE": 10.8, "ZEC": 9.5}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-19T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-19T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-20"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-19T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-20T00:04 | xs_momentum | DOT | buy | $1,353 | 0.715747 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | XRP | buy | $1,158 | 0.962098 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | AAVE | buy | $1,100 | 104.561 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | SEI | buy | $1,051 | 0.0381037 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | TAO | buy | $1,012 | 192.892 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | BTC | sell | $1,068 | 62071.4 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | UNI | sell | $1,210 | 4.94349 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | AVAX | sell | $1,276 | 7.00851 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | ETH | sell | $1,290 | 1760.36 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | APT | sell | $1,332 | 0.41265 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | ENA | sell | $1,428 | 0.104965 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | ARB | sell | $1,436 | 0.122072 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | FIL | sell | $1,517 | 0.575471 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_funding | LDO | sell | $1,041 | 0.260038 | daily xs-funding (Binance recon) |
| 2026-11-20T00:04 | xs_funding | ENA | sell | $1,200 | 0.104961 | daily xs-funding (Binance recon) |
| 2026-11-20T00:04 | trend_breakout_majors | SOL | sell | $2,332 | 73.2068 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | trend_breakout_majors | BTC | sell | $2,758 | 62071.4 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | trend_breakout_majors | ETH | sell | $3,729 | 1760.36 | signal 2026-11-19 23h |
| 2026-11-19T00:04 | xs_momentum | PENDLE | buy | $1,324 | 1.4493 | signal 2026-11-18 23h |
| 2026-11-19T00:04 | xs_momentum | TRX | buy | $1,254 | 0.309765 | signal 2026-11-18 23h |
