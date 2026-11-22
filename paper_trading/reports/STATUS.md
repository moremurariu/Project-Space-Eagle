# Paper trading status — 2026-11-22 22:04 UTC

BTC 60,862 · ETH 1,715.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $78,501 | -21.50% | +0.74% | -6.53% | -30.99% | -6.53 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,794 | -3.21% | +0.04% | -0.34% | -4.87% | -5.41 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.47 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.16 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,133 | +0.13% | +0.03% | +0.11% | -0.26% | 2.35 | 0.87 | +0.00 | 6 | $302 | $395 |
| trend_breakout_majors | $117,252 | +17.25% | -0.42% | +2.88% | -5.48% | 6.80 | 0.56 | -0.56 | 3 | $127 | $177 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.00 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.14 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.96 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,357 | +0.36% | +0.09% | -0.57% | -1.64% | 0.88 | 0.95 | +0.01 | 20 | $653 | $280 |
| xs_momentum | $98,408 | -1.59% | -0.00% | -0.67% | -3.01% | -2.65 | 0.93 | -0.01 | 36 | $238 | $52 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.28 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,116,339 on $1,100,000 → +1.49%**
Combined BTC exposure (7d realised, 4h returns): beta -0.067 (≈ -0.74 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60892 | $-21,429 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60861.5 | $-1,139 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.29295 | $-5,050 | +0.152 |
| PENDLE | -11765.6 | 1.72106 | 1.29015 | $5,070 | -0.152 |
| HYPE/spot | 233.128 | 83.2675 | 62.53 | $-4,834 | +0.146 |
| HYPE | -233.128 | 82.7772 | 62.5205 | $4,722 | -0.146 |
| ZEC/spot | 17.43 | 1175.27 | 795.612 | $-6,617 | +0.138 |
| ZEC | -17.4301 | 1175 | 795.095 | $6,622 | -0.138 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 60861.5 | $4,904 | -0.231 |
| ETH | -11.3433 | 2117.81 | 1715.25 | $4,566 | -0.166 |
| SOL | -265.603 | 89.0415 | 70.3385 | $4,968 | -0.159 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 60861.5 | $1,079 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.305615 | $-100 | +0.087 |
| ETC | 1191.56 | 5.48251 | 5.6861 | $243 | +0.068 |
| LINK | -662.607 | 8.53769 | 8.7783 | $-159 | -0.058 |
| ADA | 39260.6 | 0.144249 | 0.144185 | $-3 | +0.056 |
| APT | 12637.8 | 0.436568 | 0.3793 | $-724 | +0.048 |
| RENDER | -3551.19 | 1.49617 | 1.34195 | $548 | -0.047 |
| TAO | -25.0244 | 220.686 | 189.96 | $769 | -0.047 |
| AAVE | -44.8727 | 112.188 | 98.1615 | $629 | -0.044 |
| SEI | 123581 | 0.0376927 | 0.034403 | $-407 | +0.042 |
| PENDLE | -3236.78 | 1.54767 | 1.29015 | $834 | -0.042 |
| OP | 58569.1 | 0.0891484 | 0.07108 | $-1,058 | +0.041 |
| NEAR | -1371.79 | 3.51678 | 2.99235 | $719 | -0.041 |
| JUP | 18056.4 | 0.241363 | 0.22448 | $-305 | +0.040 |
| UNI | -836.22 | 5.16466 | 4.70685 | $383 | -0.039 |
| WLD | 15387.1 | 0.31305 | 0.23862 | $-1,145 | +0.037 |
| ENA | -33871.5 | 0.132226 | 0.101265 | $1,049 | -0.034 |
| TIA | 10070 | 0.345986 | 0.33284 | $-132 | +0.033 |
| ZEC | -3.83105 | 862.252 | 795.095 | $257 | -0.030 |
| kPEPE | 893923 | 0.0038915 | 0.003401 | $-438 | +0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.305615 | $-249 | +0.103 |
| BTC | -0.122056 | 65912.7 | 60861.5 | $617 | -0.075 |
| BNB | -11.3465 | 580.021 | 556.595 | $266 | -0.064 |
| ETH | -2.88908 | 1804.39 | 1715.25 | $258 | -0.050 |
| RENDER | 3386.49 | 1.53278 | 1.34195 | $-646 | +0.046 |
| FIL | -7861.93 | 0.620713 | 0.53442 | $678 | -0.043 |
| ATOM | 3503.24 | 1.28664 | 1.1718 | $-402 | +0.042 |
| INJ | 607.666 | 7.33527 | 6.19615 | $-692 | +0.038 |
| CRV | -19201.4 | 0.232521 | 0.18065 | $996 | -0.035 |
| JUP | 15235.8 | 0.263176 | 0.22448 | $-590 | +0.035 |
| AAVE | 32.3234 | 108.591 | 98.1615 | $-337 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.3793 | $423 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.6701 | $-519 | +0.029 |
| BCH | -14.0941 | 212.452 | 197.845 | $206 | -0.028 |
| NEAR | 890.083 | 3.11366 | 2.99235 | $-108 | +0.027 |
| SOL | -36.1325 | 91.9832 | 70.3385 | $782 | -0.026 |
| TAO | 12.9296 | 197.549 | 189.96 | $-98 | +0.025 |
| DOGE | -44257.5 | 0.0643319 | 0.055101 | $409 | -0.025 |
| ENA | -21996.7 | 0.108918 | 0.101265 | $168 | -0.023 |
| TIA | 5612.94 | 0.336231 | 0.33284 | $-19 | +0.019 |
| kPEPE | 522031 | 0.00484647 | 0.003401 | $-755 | +0.018 |
| WLD | -5872.05 | 0.34434 | 0.23862 | $621 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.923335 | $146 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.127325 | $-121 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11489 | $-339 | +0.011 |
| LTC | -25.7819 | 40.5854 | 39.9865 | $15 | -0.010 |
| PENDLE | 752.441 | 1.27394 | 1.29015 | $12 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.051462 | $317 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.034403 | $-87 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.65965 | $219 | -0.008 |
| ZEC | -0.884394 | 901.025 | 795.095 | $94 | -0.007 |
| OP | -6924.47 | 0.0705215 | 0.07108 | $-4 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.70685 | $-229 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.23588 | $-4 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.283285 | $-72 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.65503 | $-15 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.35, "CRV": -0.49, "ENA": -0.59, "NEAR": -0.47, "PENDLE": -0.23}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.35, "CRV": -0.49, "ENA": -0.59, "NEAR": -0.47, "PENDLE": -0.23}}`
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
