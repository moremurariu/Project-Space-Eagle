# Paper trading status — 2026-11-22 02:04 UTC

BTC 60,106 · ETH 1,686.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $77,529 | -22.47% | +1.91% | -4.45% | -30.99% | -7.17 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,736 | -3.26% | +0.09% | -0.23% | -4.87% | -5.63 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.48 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.23 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,090 | +0.09% | +0.01% | -0.09% | -0.26% | 2.07 | 0.86 | -0.00 | 6 | $302 | $386 |
| trend_breakout_majors | $117,940 | +17.94% | -1.45% | +1.04% | -5.48% | 7.24 | 0.55 | -0.55 | 3 | $127 | $178 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.06 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.20 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -5.11 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,358 | +0.36% | -0.80% | -0.77% | -1.64% | 0.91 | 0.94 | +0.01 | 20 | $653 | $266 |
| xs_momentum | $98,461 | -1.54% | -0.77% | -0.98% | -3.01% | -2.66 | 0.92 | -0.01 | 36 | $238 | $40 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.39 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,116,979 on $1,100,000 → +1.54%**
Combined BTC exposure (7d realised, 4h returns): beta -0.072 (≈ -0.80 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60139 | $-22,401 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60106.5 | $-1,197 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -11765.6 | 1.72106 | 1.26915 | $5,317 | -0.149 |
| PENDLE/spot | 11756.4 | 1.72251 | 1.27 | $-5,320 | +0.149 |
| HYPE/spot | 233.128 | 83.2675 | 61.2355 | $-5,136 | +0.143 |
| HYPE | -233.128 | 82.7772 | 61.2325 | $5,023 | -0.143 |
| ZEC | -17.4301 | 1175 | 780.415 | $6,878 | -0.136 |
| ZEC/spot | 17.43 | 1175.27 | 780.38 | $-6,883 | +0.136 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 60106.5 | $5,240 | -0.227 |
| ETH | -11.3433 | 2117.81 | 1686.25 | $4,895 | -0.162 |
| SOL | -265.603 | 89.0415 | 70.2595 | $4,989 | -0.158 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.305145 | $-113 | +0.087 |
| BTC | -0.144359 | 68333.1 | 60106.5 | $1,188 | -0.086 |
| ETC | 1191.56 | 5.48251 | 5.4832 | $1 | +0.065 |
| LINK | -662.607 | 8.53769 | 8.5824 | $-30 | -0.057 |
| ADA | 39260.6 | 0.144249 | 0.144045 | $-8 | +0.056 |
| RENDER | -3551.19 | 1.49617 | 1.32025 | $625 | -0.047 |
| APT | 12637.8 | 0.436568 | 0.3707 | $-832 | +0.047 |
| TAO | -25.0244 | 220.686 | 186.67 | $851 | -0.047 |
| AAVE | -44.8727 | 112.188 | 97.105 | $677 | -0.043 |
| SEI | 123581 | 0.0376927 | 0.034192 | $-433 | +0.042 |
| OP | 58569.1 | 0.0891484 | 0.0704 | $-1,098 | +0.041 |
| PENDLE | -3236.78 | 1.54767 | 1.26915 | $901 | -0.041 |
| NEAR | -1371.79 | 3.51678 | 2.91325 | $828 | -0.040 |
| JUP | 18056.4 | 0.241363 | 0.21999 | $-386 | +0.040 |
| UNI | -836.22 | 5.16466 | 4.73015 | $363 | -0.039 |
| WLD | 15387.1 | 0.31305 | 0.22901 | $-1,293 | +0.035 |
| ENA | -33871.5 | 0.132226 | 0.098458 | $1,144 | -0.033 |
| TIA | 10070 | 0.345986 | 0.32334 | $-228 | +0.032 |
| kPEPE | 893923 | 0.0038915 | 0.003424 | $-418 | +0.030 |
| ZEC | -3.83105 | 862.252 | 780.415 | $314 | -0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.305145 | $-264 | +0.103 |
| BTC | -0.122056 | 65912.7 | 60106.5 | $709 | -0.075 |
| BNB | -11.3465 | 580.021 | 549.46 | $347 | -0.063 |
| ETH | -2.88908 | 1804.39 | 1686.25 | $341 | -0.049 |
| RENDER | 3386.49 | 1.53278 | 1.32025 | $-720 | +0.045 |
| ATOM | 3503.24 | 1.28664 | 1.1636 | $-431 | +0.041 |
| FIL | -7861.93 | 0.620713 | 0.51709 | $815 | -0.041 |
| INJ | 607.666 | 7.33527 | 5.95355 | $-840 | +0.037 |
| CRV | -19201.4 | 0.232521 | 0.17711 | $1,064 | -0.035 |
| JUP | 15235.8 | 0.263176 | 0.21999 | $-658 | +0.034 |
| AAVE | 32.3234 | 108.591 | 97.105 | $-371 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.3707 | $491 | -0.030 |
| BCH | -14.0941 | 212.452 | 199.62 | $181 | -0.029 |
| AVAX | 424.946 | 7.89216 | 6.5732 | $-560 | +0.028 |
| NEAR | 890.083 | 3.11366 | 2.91325 | $-178 | +0.026 |
| SOL | -36.1325 | 91.9832 | 70.2595 | $785 | -0.026 |
| DOGE | -44257.5 | 0.0643319 | 0.055115 | $408 | -0.025 |
| TAO | 12.9296 | 197.549 | 186.67 | $-141 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.098458 | $230 | -0.022 |
| TIA | 5612.94 | 0.336231 | 0.32334 | $-72 | +0.018 |
| kPEPE | 522031 | 0.00484647 | 0.003424 | $-743 | +0.018 |
| XRP | -1446.41 | 1.02442 | 0.932075 | $134 | -0.014 |
| WLD | -5872.05 | 0.34434 | 0.22901 | $677 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.12833 | $-112 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11341 | $-352 | +0.010 |
| LTC | -25.7819 | 40.5854 | 39.79 | $21 | -0.010 |
| PENDLE | 752.441 | 1.27394 | 1.26915 | $-4 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.051273 | $320 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.034192 | $-92 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.65302 | $227 | -0.008 |
| ZEC | -0.884394 | 901.025 | 780.415 | $107 | -0.007 |
| OP | -6924.47 | 0.0705215 | 0.0704 | $1 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.73015 | $-227 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.23228 | $2 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.28204 | $-73 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.64698 | $-16 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ADA": -0.76, "AVAX": -0.83, "HBAR": -0.72, "INJ": -1.21, "ZEC": -1.48}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ADA": -0.76, "AVAX": -0.83, "HBAR": -0.72, "INJ": -1.21, "ZEC": -1.48}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.5, "PENDLE": 10.3, "ZEC": 10.2}, "last_day": "2026-11-22", "top_apr": {"BTC": 6.2, "CRV": 7.7, "ENA": 7.7, "ETC": 4.2, "HYPE": 8.5, "LDO": 9.3, "NEAR": 4.7, "PENDLE": 10.3, "RENDER": 4.3, "ZEC": 10.2}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-21T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-21T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-22"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-21T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-22T00:04 | xs_momentum | XRP | buy | $2,015 | 0.930512 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | PENDLE | buy | $1,392 | 1.27394 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | AAVE | buy | $1,302 | 97.3365 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | SOL | buy | $1,121 | 70.3166 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | NEAR | buy | $1,037 | 2.88464 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | OP | sell | $1,010 | 0.0705215 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | ARB | sell | $1,032 | 0.113376 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | APT | sell | $1,235 | 0.371381 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | LDO | sell | $1,399 | 0.233401 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | CRV | sell | $1,428 | 0.178385 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_momentum | FIL | sell | $1,491 | 0.517028 | signal 2026-11-21 23h |
| 2026-11-22T00:04 | xs_funding | ADA | buy | $5,663 | 0.144249 | daily xs-funding (Binance recon) |
| 2026-11-22T00:04 | xs_funding | BCH | sell | $5,819 | 199.798 | daily xs-funding (Binance recon) |
| 2026-11-22T00:04 | trend_breakout_majors | ETH | buy | $2,740 | 1685.33 | signal 2026-11-21 23h |
| 2026-11-21T00:04 | dn_funding_carry | PENDLE/spot | buy | $2,677 | 1.27817 | daily dn-carry |
| 2026-11-21T00:04 | dn_funding_carry | PENDLE | sell | $2,671 | 1.27536 | daily dn-carry |
| 2026-11-21T00:04 | xs_momentum | SOL | buy | $1,486 | 68.8689 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | ATOM | buy | $1,385 | 1.22545 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | DOGE | buy | $1,175 | 0.0543447 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | TIA | buy | $1,040 | 0.328793 | signal 2026-11-20 23h |
