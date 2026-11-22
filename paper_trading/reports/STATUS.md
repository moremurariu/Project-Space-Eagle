# Paper trading status — 2026-11-22 05:04 UTC

BTC 59,144 · ETH 1,660.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $76,281 | -23.72% | +5.08% | -8.65% | -30.99% | -7.58 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,662 | -3.34% | +0.24% | -0.45% | -4.87% | -5.74 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.48 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.22 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,108 | +0.11% | -0.07% | +0.00% | -0.26% | 2.20 | 0.84 | +0.00 | 6 | $302 | $387 |
| trend_breakout_majors | $118,925 | +18.92% | -2.97% | +3.95% | -5.48% | 7.55 | 0.53 | -0.53 | 3 | $127 | $177 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.05 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.20 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -5.08 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,507 | +0.51% | -1.03% | -0.22% | -1.64% | 1.16 | 0.93 | +0.01 | 20 | $653 | $269 |
| xs_momentum | $98,639 | -1.36% | -0.78% | -0.37% | -3.01% | -2.32 | 0.91 | -0.01 | 36 | $238 | $42 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.37 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,118,234 on $1,100,000 → +1.66%**
Combined BTC exposure (7d realised, 4h returns): beta -0.071 (≈ -0.79 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 59173 | $-23,649 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 59143.5 | $-1,271 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.2495 | $-5,561 | +0.147 |
| PENDLE | -11765.6 | 1.72106 | 1.24785 | $5,568 | -0.147 |
| HYPE/spot | 233.128 | 83.2675 | 59.81 | $-5,469 | +0.139 |
| HYPE | -233.128 | 82.7772 | 59.7975 | $5,357 | -0.139 |
| ZEC/spot | 17.43 | 1175.27 | 769.26 | $-7,077 | +0.134 |
| ZEC | -17.4301 | 1175 | 769.075 | $7,075 | -0.134 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 59143.5 | $5,669 | -0.221 |
| ETH | -11.3433 | 2117.81 | 1660.55 | $5,187 | -0.158 |
| SOL | -265.603 | 89.0415 | 69.2565 | $5,255 | -0.155 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.304925 | $-119 | +0.086 |
| BTC | -0.144359 | 68333.1 | 59143.5 | $1,327 | -0.085 |
| ETC | 1191.56 | 5.48251 | 5.4475 | $-42 | +0.065 |
| LINK | -662.607 | 8.53769 | 8.5497 | $-8 | -0.056 |
| ADA | 39260.6 | 0.144249 | 0.141785 | $-97 | +0.055 |
| RENDER | -3551.19 | 1.49617 | 1.30725 | $671 | -0.046 |
| APT | 12637.8 | 0.436568 | 0.3669 | $-880 | +0.046 |
| TAO | -25.0244 | 220.686 | 183.46 | $932 | -0.046 |
| AAVE | -44.8727 | 112.188 | 96.0445 | $724 | -0.043 |
| SEI | 123581 | 0.0376927 | 0.033757 | $-486 | +0.042 |
| OP | 58569.1 | 0.0891484 | 0.06991 | $-1,127 | +0.041 |
| PENDLE | -3236.78 | 1.54767 | 1.24785 | $970 | -0.040 |
| NEAR | -1371.79 | 3.51678 | 2.86505 | $894 | -0.039 |
| JUP | 18056.4 | 0.241363 | 0.21584 | $-461 | +0.039 |
| UNI | -836.22 | 5.16466 | 4.59165 | $479 | -0.038 |
| WLD | 15387.1 | 0.31305 | 0.22515 | $-1,353 | +0.034 |
| ENA | -33871.5 | 0.132226 | 0.097595 | $1,173 | -0.033 |
| TIA | 10070 | 0.345986 | 0.32038 | $-258 | +0.032 |
| kPEPE | 893923 | 0.0038915 | 0.003334 | $-498 | +0.030 |
| ZEC | -3.83105 | 862.252 | 769.075 | $357 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.304925 | $-272 | +0.103 |
| BTC | -0.122056 | 65912.7 | 59143.5 | $826 | -0.073 |
| BNB | -11.3465 | 580.021 | 540.05 | $454 | -0.062 |
| ETH | -2.88908 | 1804.39 | 1660.55 | $416 | -0.049 |
| RENDER | 3386.49 | 1.53278 | 1.30725 | $-764 | +0.045 |
| ATOM | 3503.24 | 1.28664 | 1.1547 | $-462 | +0.041 |
| FIL | -7861.93 | 0.620713 | 0.51134 | $860 | -0.041 |
| INJ | 607.666 | 7.33527 | 5.90885 | $-867 | +0.036 |
| CRV | -19201.4 | 0.232521 | 0.1766 | $1,074 | -0.034 |
| JUP | 15235.8 | 0.263176 | 0.21584 | $-721 | +0.033 |
| AAVE | 32.3234 | 108.591 | 96.0445 | $-406 | +0.031 |
| APT | -7937.59 | 0.43262 | 0.3669 | $522 | -0.030 |
| AVAX | 424.946 | 7.89216 | 6.5227 | $-582 | +0.028 |
| BCH | -14.0941 | 212.452 | 195.955 | $233 | -0.028 |
| NEAR | 890.083 | 3.11366 | 2.86505 | $-221 | +0.026 |
| SOL | -36.1325 | 91.9832 | 69.2565 | $821 | -0.025 |
| DOGE | -44257.5 | 0.0643319 | 0.054209 | $448 | -0.024 |
| TAO | 12.9296 | 197.549 | 183.46 | $-182 | +0.024 |
| ENA | -21996.7 | 0.108918 | 0.097595 | $249 | -0.022 |
| TIA | 5612.94 | 0.336231 | 0.32038 | $-89 | +0.018 |
| kPEPE | 522031 | 0.00484647 | 0.003334 | $-790 | +0.018 |
| XRP | -1446.41 | 1.02442 | 0.918695 | $153 | -0.013 |
| WLD | -5872.05 | 0.34434 | 0.22515 | $700 | -0.013 |
| XLM | 9051.42 | 0.140675 | 0.127125 | $-123 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11187 | $-366 | +0.010 |
| LTC | -25.7819 | 40.5854 | 39.2555 | $34 | -0.010 |
| PENDLE | 752.441 | 1.27394 | 1.24785 | $-20 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.050799 | $328 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.033757 | $-102 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.64614 | $235 | -0.008 |
| ZEC | -0.884394 | 901.025 | 769.075 | $117 | -0.007 |
| OP | -6924.47 | 0.0705215 | 0.06991 | $4 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.59165 | $-240 | +0.004 |
| LDO | -1790.74 | 0.233401 | 0.22975 | $7 | -0.004 |
| ONDO | 657.524 | 0.392809 | 0.277365 | $-76 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.63781 | $-17 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"DOGE": -1.67, "ONDO": -1.63, "PENDLE": -1.8, "UNI": -1.98, "kPEPE": -2.33}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"DOGE": -1.67, "ONDO": -1.63, "PENDLE": -1.8, "UNI": -1.98, "kPEPE": -2.33}}`
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
