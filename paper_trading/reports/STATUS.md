# Paper trading status — 2026-11-15 22:04 UTC

BTC 65,062 · ETH 1,821.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,906 | -16.09% | +1.30% | -12.84% | -22.40% | -8.06 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,122 | -2.88% | +0.08% | -1.09% | -4.33% | -5.97 | 0.05 | +0.05 | 1 | $53 | $-96 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.57 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.83 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,033 | +0.03% | -0.16% | -0.11% | -0.24% | 2.09 | 0.91 | +0.00 | 6 | $296 | $310 |
| trend_breakout_majors | $113,988 | +13.99% | -1.50% | +12.30% | -5.48% | 8.30 | 0.55 | -0.55 | 3 | $115 | $167 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.62 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.80 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,924 | +0.92% | -0.11% | +1.58% | -1.56% | 2.74 | 0.95 | +0.02 | 20 | $561 | $146 |
| xs_momentum | $99,040 | -0.96% | -0.24% | -1.81% | -2.98% | nan | 0.90 | -0.03 | 39 | $188 | $-5 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,112,434 on $1,100,000 → +1.13%**
Combined BTC exposure (7d realised, 4h returns): beta -0.080 (≈ -0.88 books of BTC), corr -0.89

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65078 | $-16,024 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 65062.5 | $-815 | +0.052 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 233.128 | 83.2675 | 67.7695 | $-3,613 | +0.158 |
| ZEC/spot | 17.43 | 1175.27 | 905.98 | $-4,694 | +0.158 |
| HYPE | -233.128 | 82.7772 | 67.7355 | $3,507 | -0.158 |
| ZEC | -17.4301 | 1175 | 905.71 | $4,694 | -0.158 |
| PENDLE | -9671.02 | 1.81759 | 1.4456 | $3,598 | -0.140 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.4465 | $-3,597 | +0.140 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.356716 | 75630.7 | 65062.5 | $3,770 | -0.204 |
| SOL | -281.278 | 91.199 | 72.0415 | $5,389 | -0.178 |
| ETH | -10.8509 | 2187.58 | 1821.55 | $3,972 | -0.173 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 65062.5 | $472 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.30484 | $-122 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.965115 | $1,542 | -0.071 |
| XLM | 44454.5 | 0.151093 | 0.140105 | $-488 | +0.062 |
| ATOM | 4381.77 | 1.45283 | 1.304 | $-652 | +0.057 |
| BCH | 23.4344 | 238.546 | 208.915 | $-694 | +0.049 |
| SEI | 123581 | 0.0376927 | 0.039369 | $207 | +0.048 |
| TAO | -25.0244 | 220.686 | 193.2 | $688 | -0.048 |
| JUP | 18056.4 | 0.241363 | 0.265775 | $441 | +0.048 |
| LINK | -516.66 | 11.3839 | 9.26455 | $1,095 | -0.047 |
| LDO | -14834.4 | 0.308439 | 0.27744 | $460 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.3811 | $497 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13453 | $-459 | +0.036 |
| PENDLE | -2505.4 | 1.58072 | 1.4456 | $339 | -0.036 |
| TIA | 10070 | 0.345986 | 0.35813 | $122 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08157 | $-491 | +0.036 |
| ZEC | -3.83105 | 862.252 | 905.71 | $-166 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.18355 | $585 | -0.031 |
| WLD | 10593 | 0.336516 | 0.26703 | $-736 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.12015 | $583 | -0.027 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30484 | $-265 | +0.079 |
| SOL | -87.2593 | 91.9832 | 72.0415 | $1,740 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.965115 | $370 | -0.061 |
| AVAX | 607.041 | 7.89216 | 7.2922 | $-364 | +0.045 |
| INJ | 607.666 | 7.33527 | 7.00265 | $-202 | +0.043 |
| ARB | 29979.9 | 0.152074 | 0.13453 | $-526 | +0.041 |
| PENDLE | -2522.74 | 1.58296 | 1.4456 | $347 | -0.037 |
| BTC | -0.0540197 | 68049 | 65062.5 | $161 | -0.035 |
| RENDER | 2461.25 | 1.58063 | 1.3811 | $-491 | +0.034 |
| DOT | -4637.48 | 0.841601 | 0.73216 | $508 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.003867 | $-787 | +0.031 |
| UNI | 597.681 | 7.08573 | 5.16985 | $-1,145 | +0.031 |
| BCH | -14.0941 | 212.452 | 208.915 | $50 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.059064 | $352 | -0.028 |
| JUP | 10388.9 | 0.261398 | 0.265775 | $45 | +0.028 |
| ETH | -1.46269 | 1817.83 | 1821.55 | $-5 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26703 | $771 | -0.027 |
| ADA | -16388.2 | 0.190675 | 0.15407 | $600 | -0.025 |
| CRV | -11195.5 | 0.271234 | 0.2039 | $754 | -0.023 |
| AAVE | 18.1796 | 131.483 | 109.105 | $-407 | +0.020 |
| OP | 24123.2 | 0.107142 | 0.08157 | $-617 | +0.020 |
| ATOM | 1186.94 | 1.29824 | 1.304 | $7 | +0.016 |
| FIL | -2342.25 | 0.799282 | 0.60972 | $444 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.039369 | $264 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.140105 | $45 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27744 | $41 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.12015 | $-41 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.059138 | $190 | -0.010 |
| TIA | 2451.15 | 0.345826 | 0.35813 | $30 | +0.009 |
| LINK | 77.5637 | 11.0659 | 9.26455 | $-140 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4552 | $237 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 905.71 | $-192 | +0.006 |
| BNB | 0.917396 | 705.088 | 593.705 | $-102 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.18355 | $84 | -0.005 |
| TAO | 2.54822 | 211.979 | 193.2 | $-48 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.309855 | $-55 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.079658 | $-26 | +0.002 |
| LTC | -3.23093 | 54.9478 | 45.3525 | $31 | -0.001 |
| SUI | 66.7499 | 0.886044 | 0.72973 | $-10 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -0.67, "DOT": -0.14, "FIL": -0.08, "TIA": -0.14, "kPEPE": -0.77}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -0.67, "DOT": -0.14, "FIL": -0.08, "TIA": -0.14, "kPEPE": -0.77}}`
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
| 2026-11-15T10:06 | dn_funding_carry | HYPE/spot | buy | $2,130 | 68.4242 | dn-carry hedge match |
| 2026-11-15T02:05 | claude_discretionary | BTC | buy | $626 | 62964.6 | discretionary: Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% s |
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
