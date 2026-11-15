# Paper trading status — 2026-11-15 01:08 UTC

BTC 62,649 · ETH 1,718.3 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $80,816 | -19.18% | -8.24% | -17.32% | -22.40% | -11.60 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,941 | -3.06% | -0.39% | -1.66% | -4.33% | -6.59 | 0.04 | +0.04 | 1 | $53 | $-95 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.59 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.94 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,153 | +0.15% | -0.01% | +0.04% | -0.19% | 3.51 | 0.83 | -0.02 | 6 | $293 | $302 |
| trend_breakout_majors | $116,922 | +16.92% | +7.07% | +17.29% | -5.48% | 10.96 | 0.52 | -0.52 | 3 | $115 | $162 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.73 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.91 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | -0.16% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,304 | +1.30% | +1.11% | +1.60% | -1.56% | 4.03 | 0.90 | +0.02 | 20 | $561 | $126 |
| xs_momentum | $99,523 | -0.48% | +0.70% | -0.87% | -2.98% | nan | 0.85 | -0.02 | 39 | $188 | $-8 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,116,169 on $1,100,000 → +1.47%**
Combined BTC exposure (7d realised, 4h returns): beta -0.067 (≈ -0.73 books of BTC), corr -0.82

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 62685 | $-19,114 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.067052 | 77535.3 | 62649 | $-998 | +0.043 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 848.88 | $-5,689 | +0.148 |
| ZEC | -17.4301 | 1175 | 848.63 | $5,689 | -0.148 |
| HYPE | -233.128 | 82.7772 | 63.3485 | $4,529 | -0.147 |
| PENDLE | -9671.02 | 1.81759 | 1.3438 | $4,582 | -0.130 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.344 | $-4,588 | +0.130 |
| HYPE/spot | 201.997 | 85.5551 | 63.2595 | $-4,504 | +0.128 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.356716 | 75630.7 | 62649 | $4,631 | -0.191 |
| SOL | -281.278 | 91.199 | 68.6345 | $6,347 | -0.165 |
| ETH | -10.8509 | 2187.58 | 1718.35 | $5,092 | -0.159 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 62649 | $821 | -0.089 |
| TRX | 28470.1 | 0.309116 | 0.30587 | $-92 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.910915 | $1,945 | -0.067 |
| XLM | 44454.5 | 0.151093 | 0.13352 | $-781 | +0.059 |
| ATOM | 4381.77 | 1.45283 | 1.2423 | $-922 | +0.054 |
| BCH | 23.4344 | 238.546 | 197.01 | $-973 | +0.046 |
| SEI | 123581 | 0.0376927 | 0.03712 | $-71 | +0.045 |
| TAO | -25.0244 | 220.686 | 180.805 | $998 | -0.045 |
| LINK | -516.66 | 11.3839 | 8.7185 | $1,377 | -0.044 |
| JUP | 18056.4 | 0.241363 | 0.248285 | $125 | +0.044 |
| LDO | -14834.4 | 0.308439 | 0.26094 | $705 | -0.038 |
| RENDER | -2753.67 | 1.56167 | 1.3274 | $645 | -0.036 |
| OP | 44197.2 | 0.0926854 | 0.07835 | $-634 | +0.034 |
| ARB | 27302.4 | 0.151348 | 0.12535 | $-710 | +0.034 |
| TIA | 10070 | 0.345986 | 0.33775 | $-83 | +0.034 |
| PENDLE | -2505.4 | 1.58072 | 1.3438 | $594 | -0.033 |
| ZEC | -3.83105 | 862.252 | 848.63 | $52 | -0.032 |
| NEAR | -997.435 | 3.77006 | 2.98595 | $782 | -0.029 |
| WLD | 10593 | 0.336516 | 0.25025 | $-914 | +0.026 |
| ENA | -22442.6 | 0.146112 | 0.11238 | $757 | -0.025 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30587 | $-238 | +0.079 |
| SOL | -87.2593 | 91.9832 | 68.6345 | $2,037 | -0.060 |
| XRP | -6239.14 | 1.02442 | 0.910915 | $708 | -0.057 |
| AVAX | 607.041 | 7.89216 | 6.8954 | $-605 | +0.042 |
| INJ | 607.666 | 7.33527 | 6.729 | $-368 | +0.041 |
| ARB | 29979.9 | 0.152074 | 0.12535 | $-801 | +0.038 |
| PENDLE | -2522.74 | 1.58296 | 1.3438 | $603 | -0.034 |
| BTC | -0.0540197 | 68049 | 62649 | $292 | -0.034 |
| RENDER | 2461.25 | 1.58063 | 1.3274 | $-623 | +0.033 |
| DOT | -4637.48 | 0.841601 | 0.68726 | $716 | -0.032 |
| kPEPE | 803792 | 0.00484647 | 0.003779 | $-858 | +0.031 |
| UNI | 597.681 | 7.08573 | 4.72835 | $-1,409 | +0.028 |
| BCH | -14.0941 | 212.452 | 197.01 | $218 | -0.028 |
| DOGE | -46908.3 | 0.0665749 | 0.056028 | $495 | -0.026 |
| JUP | 10388.9 | 0.261398 | 0.248285 | $-136 | +0.026 |
| ETH | -1.46269 | 1817.83 | 1718.35 | $146 | -0.025 |
| WLD | -9976.36 | 0.34434 | 0.25025 | $939 | -0.025 |
| ADA | -16388.2 | 0.190675 | 0.144375 | $759 | -0.024 |
| CRV | -11195.5 | 0.271234 | 0.19954 | $803 | -0.022 |
| OP | 24123.2 | 0.107142 | 0.07835 | $-695 | +0.019 |
| AAVE | 18.1796 | 131.483 | 97.971 | $-609 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.2423 | $-66 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.59516 | $478 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.03712 | $339 | -0.012 |
| XLM | -8726.83 | 0.145302 | 0.13352 | $103 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.26094 | $-28 | +0.011 |
| ENA | -8394.07 | 0.115324 | 0.11238 | $25 | -0.009 |
| HBAR | -16510.2 | 0.0706575 | 0.055679 | $247 | -0.009 |
| TIA | 2451.15 | 0.345826 | 0.33775 | $-20 | +0.008 |
| LINK | 77.5637 | 11.0659 | 8.7185 | $-182 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4391 | $259 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 848.63 | $-227 | +0.005 |
| BNB | 0.917396 | 705.088 | 570.14 | $-124 | +0.005 |
| NEAR | -155.553 | 3.72366 | 2.98595 | $115 | -0.005 |
| TAO | 2.54822 | 211.979 | 180.805 | $-79 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.29126 | $-67 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.076469 | $-33 | +0.002 |
| LTC | -3.23093 | 54.9478 | 43.217 | $38 | -0.001 |
| SUI | 66.7499 | 0.886044 | 0.68556 | $-13 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-12 12:10 (due 11-13): cut BTC 0.15 -> 0.05 ahead of the 72k rule. BTC 72.9k, 1.2% above the rule. The liquidity picture keeps worsening: stablecoin supply -$14.2B over 30 days (largest drop since June 2022), BlackRock BTC ETF outflows $2B over 9 sessions, MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of a large BTC proxy), Fed hike odds high, gold -2.5%/24h. ETH 7d RR25 -6.6 (put skew), F&G 25-32. No sign of capitulation/reversal yet. Keep a token long for a fear-driven bounce; systematic books are already net short. Rules: BTC reclaims 7`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AVAX": -2.56, "BTC": -2.07, "ETH": -2.41, "INJ": -3.42, "PENDLE": -2.24}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AVAX": -2.56, "BTC": -2.07, "ETH": -2.41, "INJ": -3.42, "PENDLE": -2.24}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 7.5, "PENDLE": 11.0, "ZEC": 9.4}, "last_day": "2026-11-15", "top_apr": {"BTC": 8.5, "CRV": 10.6, "ETC": 7.2, "HYPE": 7.5, "LDO": 11.0, "OP": 9.4, "PENDLE": 11.0, "RENDER": 9.3, "TAO": 7.7, "ZEC": 9.4}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-15"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-15T00:04 | dn_funding_carry | ENA | buy | $12,876 | 0.115365 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | UNI | buy | $12,052 | 4.87649 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ZEC/spot | buy | $2,659 | 862.731 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | HYPE | sell | $2,006 | 64.4673 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ZEC | sell | $2,654 | 862.133 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | UNI/spot | sell | $12,045 | 4.87956 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ENA/spot | sell | $12,869 | 0.115377 | daily dn-carry |
| 2026-11-15T00:04 | xs_momentum | AVAX | buy | $1,634 | 7.09625 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | TRX | buy | $1,514 | 0.306628 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | TIA | buy | $1,213 | 0.345826 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | LDO | buy | $1,125 | 0.267639 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ARB | buy | $1,117 | 0.127725 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | SEI | buy | $1,043 | 0.0376879 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | XLM | buy | $1,042 | 0.135457 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | INJ | buy | $1,028 | 6.80679 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ATOM | buy | $997 | 1.26138 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | DOT | sell | $1,005 | 0.708701 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ENA | sell | $1,045 | 0.115324 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ZEC | sell | $1,142 | 862.252 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | BTC | sell | $1,182 | 64101.3 | signal 2026-11-14 23h |
