# Paper trading status — 2026-11-22 11:04 UTC

BTC 60,900 · ETH 1,710.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $78,524 | -21.48% | +5.03% | -6.99% | -30.99% | -6.61 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,797 | -3.20% | +0.23% | -0.37% | -4.87% | -5.46 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.48 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.20 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,117 | +0.12% | +0.02% | +0.10% | -0.26% | 2.25 | 0.87 | +0.00 | 6 | $302 | $390 |
| trend_breakout_majors | $117,187 | +17.19% | -2.46% | +2.99% | -5.48% | 6.85 | 0.56 | -0.56 | 3 | $127 | $177 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.03 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.18 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -5.04 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,292 | +0.29% | -0.49% | -0.48% | -1.64% | 0.79 | 0.96 | +0.01 | 20 | $653 | $274 |
| xs_momentum | $98,386 | -1.61% | -0.53% | -0.90% | -3.01% | -2.75 | 0.94 | -0.01 | 36 | $238 | $48 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.34 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,116,173 on $1,100,000 → +1.47%**
Combined BTC exposure (7d realised, 4h returns): beta -0.068 (≈ -0.75 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60910 | $-21,406 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60900.5 | $-1,136 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.293 | $-5,049 | +0.152 |
| PENDLE | -11765.6 | 1.72106 | 1.2908 | $5,062 | -0.152 |
| HYPE/spot | 233.128 | 83.2675 | 62.415 | $-4,861 | +0.145 |
| HYPE | -233.128 | 82.7772 | 62.4135 | $4,747 | -0.145 |
| ZEC/spot | 17.43 | 1175.27 | 795.24 | $-6,624 | +0.138 |
| ZEC | -17.4301 | 1175 | 794.885 | $6,625 | -0.138 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 60900.5 | $4,887 | -0.231 |
| ETH | -11.3433 | 2117.81 | 1710.45 | $4,621 | -0.166 |
| SOL | -265.603 | 89.0415 | 70.7235 | $4,865 | -0.160 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 60900.5 | $1,073 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.305045 | $-116 | +0.087 |
| ETC | 1191.56 | 5.48251 | 5.5794 | $115 | +0.066 |
| LINK | -662.607 | 8.53769 | 8.81975 | $-187 | -0.058 |
| ADA | 39260.6 | 0.144249 | 0.14585 | $63 | +0.057 |
| APT | 12637.8 | 0.436568 | 0.381 | $-702 | +0.048 |
| RENDER | -3551.19 | 1.49617 | 1.3429 | $544 | -0.048 |
| TAO | -25.0244 | 220.686 | 190.555 | $754 | -0.048 |
| AAVE | -44.8727 | 112.188 | 97.783 | $646 | -0.044 |
| SEI | 123581 | 0.0376927 | 0.034945 | $-340 | +0.043 |
| OP | 58569.1 | 0.0891484 | 0.07225 | $-990 | +0.042 |
| PENDLE | -3236.78 | 1.54767 | 1.2908 | $831 | -0.042 |
| NEAR | -1371.79 | 3.51678 | 3.00965 | $696 | -0.041 |
| JUP | 18056.4 | 0.241363 | 0.22258 | $-339 | +0.040 |
| UNI | -836.22 | 5.16466 | 4.7294 | $364 | -0.039 |
| WLD | 15387.1 | 0.31305 | 0.23454 | $-1,208 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.102575 | $1,004 | -0.035 |
| TIA | 10070 | 0.345986 | 0.33977 | $-63 | +0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003414 | $-427 | +0.030 |
| ZEC | -3.83105 | 862.252 | 794.885 | $258 | -0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.305045 | $-268 | +0.103 |
| BTC | -0.122056 | 65912.7 | 60900.5 | $612 | -0.076 |
| BNB | -11.3465 | 580.021 | 552.435 | $313 | -0.064 |
| ETH | -2.88908 | 1804.39 | 1710.45 | $271 | -0.050 |
| RENDER | 3386.49 | 1.53278 | 1.3429 | $-643 | +0.046 |
| FIL | -7861.93 | 0.620713 | 0.5333 | $687 | -0.043 |
| ATOM | 3503.24 | 1.28664 | 1.1811 | $-370 | +0.042 |
| INJ | 607.666 | 7.33527 | 6.22835 | $-673 | +0.038 |
| CRV | -19201.4 | 0.232521 | 0.18548 | $903 | -0.036 |
| JUP | 15235.8 | 0.263176 | 0.22258 | $-619 | +0.034 |
| AAVE | 32.3234 | 108.591 | 97.783 | $-349 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.381 | $410 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.678 | $-516 | +0.029 |
| BCH | -14.0941 | 212.452 | 199.1 | $188 | -0.029 |
| NEAR | 890.083 | 3.11366 | 3.00965 | $-93 | +0.027 |
| SOL | -36.1325 | 91.9832 | 70.7235 | $768 | -0.026 |
| TAO | 12.9296 | 197.549 | 190.555 | $-90 | +0.025 |
| DOGE | -44257.5 | 0.0643319 | 0.055616 | $386 | -0.025 |
| ENA | -21996.7 | 0.108918 | 0.102575 | $140 | -0.023 |
| TIA | 5612.94 | 0.336231 | 0.33977 | $20 | +0.019 |
| kPEPE | 522031 | 0.00484647 | 0.003414 | $-748 | +0.018 |
| WLD | -5872.05 | 0.34434 | 0.23454 | $645 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.932655 | $133 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.12977 | $-99 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11499 | $-338 | +0.011 |
| LTC | -25.7819 | 40.5854 | 39.666 | $24 | -0.010 |
| PENDLE | 752.441 | 1.27394 | 1.2908 | $13 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.051569 | $315 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.034945 | $-74 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.66436 | $213 | -0.008 |
| ZEC | -0.884394 | 901.025 | 794.885 | $94 | -0.007 |
| OP | -6924.47 | 0.0705215 | 0.07225 | $-12 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.7294 | $-227 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.23882 | $-10 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.286025 | $-70 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.6601 | $-15 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": 1.74, "LTC": 1.69, "RENDER": 1.36, "TRX": 0.46, "ZEC": 1.19}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": 1.74, "LTC": 1.69, "RENDER": 1.36, "TRX": 0.46, "ZEC": 1.19}}`
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
