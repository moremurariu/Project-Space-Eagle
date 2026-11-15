# Paper trading status — 2026-11-15 23:04 UTC

BTC 65,464 · ETH 1,839.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $84,417 | -15.58% | +3.23% | -12.46% | -22.40% | -7.74 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,153 | -2.85% | +0.17% | -1.10% | -4.33% | -5.90 | 0.05 | +0.05 | 1 | $53 | $-96 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.57 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.82 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,039 | +0.04% | -0.16% | -0.16% | -0.24% | 2.16 | 0.92 | +0.00 | 6 | $296 | $310 |
| trend_breakout_majors | $113,428 | +13.43% | -2.67% | +11.78% | -5.48% | 7.97 | 0.56 | -0.56 | 3 | $115 | $167 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.62 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.79 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,768 | +0.77% | -0.41% | +1.43% | -1.56% | 2.34 | 0.96 | +0.02 | 20 | $561 | $146 |
| xs_momentum | $98,796 | -1.20% | -0.77% | -2.10% | -2.98% | nan | 0.91 | -0.03 | 39 | $188 | $-5 |
| xyz_weekend_fade | $101,093 | +1.09% | -0.13% | +1.41% | -1.21% | nan | 1.00 | -0.08 | 36 | $668 | $3 |

**Combined (ex-benchmark): $1,111,384 on $1,100,000 → +1.03%**
Combined BTC exposure (7d realised, 4h returns): beta -0.081 (≈ -0.89 books of BTC), corr -0.89

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65474 | $-15,513 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 65464.5 | $-784 | +0.052 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC/spot | 17.43 | 1175.27 | 917.548 | $-4,492 | +0.160 |
| ZEC | -17.4301 | 1175 | 917.205 | $4,493 | -0.160 |
| HYPE/spot | 233.128 | 83.2675 | 68.482 | $-3,447 | +0.160 |
| HYPE | -233.128 | 82.7772 | 68.4405 | $3,342 | -0.159 |
| PENDLE | -9671.02 | 1.81759 | 1.4638 | $3,421 | -0.142 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.465 | $-3,419 | +0.141 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.356716 | 75630.7 | 65464.5 | $3,626 | -0.206 |
| SOL | -281.278 | 91.199 | 72.8505 | $5,161 | -0.181 |
| ETH | -10.8509 | 2187.58 | 1838.95 | $3,783 | -0.176 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 65464.5 | $414 | -0.094 |
| TRX | 28470.1 | 0.309116 | 0.305155 | $-113 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.976895 | $1,454 | -0.072 |
| XLM | 44454.5 | 0.151093 | 0.141845 | $-411 | +0.063 |
| ATOM | 4381.77 | 1.45283 | 1.3132 | $-612 | +0.057 |
| BCH | 23.4344 | 238.546 | 210.925 | $-647 | +0.049 |
| SEI | 123581 | 0.0376927 | 0.039735 | $252 | +0.049 |
| TAO | -25.0244 | 220.686 | 195.73 | $625 | -0.049 |
| JUP | 18056.4 | 0.241363 | 0.26755 | $473 | +0.048 |
| LINK | -516.66 | 11.3839 | 9.32695 | $1,063 | -0.048 |
| LDO | -14834.4 | 0.308439 | 0.27943 | $430 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.4 | $445 | -0.038 |
| ARB | 27302.4 | 0.151348 | 0.13537 | $-436 | +0.037 |
| PENDLE | -2505.4 | 1.58072 | 1.4638 | $293 | -0.036 |
| TIA | 10070 | 0.345986 | 0.3595 | $136 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08184 | $-479 | +0.036 |
| ZEC | -3.83105 | 862.252 | 917.205 | $-211 | -0.035 |
| NEAR | -997.435 | 3.77006 | 3.23665 | $532 | -0.032 |
| WLD | 10593 | 0.336516 | 0.26994 | $-705 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.12116 | $560 | -0.027 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.305155 | $-257 | +0.080 |
| SOL | -87.2593 | 91.9832 | 72.8505 | $1,670 | -0.064 |
| XRP | -6239.14 | 1.02442 | 0.976895 | $297 | -0.062 |
| AVAX | 607.041 | 7.89216 | 7.3036 | $-357 | +0.045 |
| INJ | 607.666 | 7.33527 | 7.007 | $-199 | +0.043 |
| ARB | 29979.9 | 0.152074 | 0.13537 | $-501 | +0.041 |
| PENDLE | -2522.74 | 1.58296 | 1.4638 | $301 | -0.037 |
| BTC | -0.0540197 | 68049 | 65464.5 | $140 | -0.036 |
| RENDER | 2461.25 | 1.58063 | 1.4 | $-445 | +0.035 |
| DOT | -4637.48 | 0.841601 | 0.73701 | $485 | -0.035 |
| kPEPE | 803792 | 0.00484647 | 0.003905 | $-757 | +0.032 |
| UNI | 597.681 | 7.08573 | 5.1883 | $-1,134 | +0.031 |
| BCH | -14.0941 | 212.452 | 210.925 | $22 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.059741 | $321 | -0.028 |
| JUP | 10388.9 | 0.261398 | 0.26755 | $64 | +0.028 |
| WLD | -9976.36 | 0.34434 | 0.26994 | $742 | -0.027 |
| ETH | -1.46269 | 1817.83 | 1838.95 | $-31 | -0.027 |
| ADA | -16388.2 | 0.190675 | 0.155565 | $575 | -0.026 |
| CRV | -11195.5 | 0.271234 | 0.20605 | $730 | -0.023 |
| AAVE | 18.1796 | 131.483 | 109.71 | $-396 | +0.020 |
| OP | 24123.2 | 0.107142 | 0.08184 | $-610 | +0.020 |
| ATOM | 1186.94 | 1.29824 | 1.3132 | $18 | +0.016 |
| FIL | -2342.25 | 0.799282 | 0.61332 | $436 | -0.015 |
| SEI | -33311.6 | 0.0472943 | 0.039735 | $252 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.141845 | $30 | -0.013 |
| LDO | 4201.67 | 0.267639 | 0.27943 | $50 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.12116 | $-49 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.059388 | $186 | -0.010 |
| TIA | 2451.15 | 0.345826 | 0.3595 | $34 | +0.009 |
| LINK | 77.5637 | 11.0659 | 9.32695 | $-135 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4582 | $233 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 917.205 | $-185 | +0.006 |
| BNB | 0.917396 | 705.088 | 597.76 | $-98 | +0.006 |
| NEAR | -155.553 | 3.72366 | 3.23665 | $76 | -0.005 |
| TAO | 2.54822 | 211.979 | 195.73 | $-41 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.312475 | $-53 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.07996 | $-26 | +0.002 |
| LTC | -3.23093 | 54.9478 | 45.605 | $30 | -0.001 |
| SUI | 66.7499 | 0.886044 | 0.73894 | $-10 | +0.000 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:MSTR | 107.175 | 101.7 | 101.685 | $-2 | +0.108 |
| xyz:PURRDAT | 1374.3 | 7.94377 | 7.92995 | $-19 | +0.108 |
| xyz:CRCL | 141.952 | 58.8308 | 58.819 | $-2 | +0.083 |
| xyz:COIN | 66.2433 | 121.002 | 120.965 | $-2 | +0.079 |
| xyz:NVDA | -35.5169 | 166.557 | 166.565 | $-0 | -0.059 |
| xyz:NBIS | -38.985 | 117.666 | 117.75 | $-3 | -0.045 |
| xyz:SKHX | -4.04354 | 905.53 | 905.575 | $-0 | -0.036 |
| xyz:AAPL | -10.0333 | 359.898 | 359.935 | $-0 | -0.036 |
| xyz:SKHY | -25.0067 | 132.85 | 132.86 | $-0 | -0.033 |
| xyz:TSLA | -7.31388 | 392.66 | 392.68 | $-0 | -0.028 |
| xyz:SPCX | -20.038 | 140 | 140.015 | $-0 | -0.028 |
| xyz:HOOD | 36.023 | 76.0676 | 76.027 | $-1 | +0.027 |
| xyz:IBM | -10.5453 | 248.34 | 248.39 | $-1 | -0.026 |
| xyz:XYZ100 | -0.092663 | 28069.1 | 28070.5 | $-0 | -0.026 |
| xyz:MSFT | -4.84152 | 526.191 | 526.225 | $-0 | -0.025 |
| xyz:GOOGL | -7.9314 | 296.85 | 296.88 | $-0 | -0.023 |
| xyz:SMSN | -15.2881 | 153.075 | 153.09 | $-0 | -0.023 |
| xyz:AMZN | -7.79693 | 267.121 | 267.15 | $-0 | -0.021 |
| xyz:SP500 | -0.272659 | 7575.27 | 7575.65 | $-0 | -0.020 |
| xyz:EWY | -14.158 | 142.995 | 143.01 | $-0 | -0.020 |
| xyz:ZHIPU | 40.034 | 50.5065 | 50.49 | $-1 | +0.020 |
| xyz:CBRS | 14.3825 | 106.881 | 106.84 | $-1 | +0.015 |
| xyz:DRAM | -35.1899 | 42.3639 | 42.366 | $-0 | -0.015 |
| xyz:ANT | -14.5089 | 102.223 | 102.265 | $-1 | -0.015 |
| xyz:META | -2.20162 | 640.883 | 640.915 | $-0 | -0.014 |
| xyz:UNITREE | -13.6311 | 101.43 | 101.44 | $-0 | -0.014 |
| xyz:MRVL | -6.61861 | 156.143 | 156.19 | $-0 | -0.010 |
| xyz:LITE | 1.26894 | 589.45 | 589.305 | $-0 | +0.007 |
| xyz:SNDK | 0.85415 | 873.385 | 873.295 | $-0 | +0.007 |
| xyz:AMD | -1.18441 | 530.998 | 531.025 | $-0 | -0.006 |
| xyz:ORCL | -4.86148 | 124.674 | 124.68 | $-0 | -0.006 |
| xyz:INTC | 7.74056 | 58.9717 | 58.96 | $-0 | +0.005 |
| xyz:CXMT | -68.0257 | 5.9968 | 5.99775 | $-0 | -0.004 |
| xyz:MRNA | -1.55358 | 221.486 | 221.545 | $-0 | -0.003 |
| xyz:MU | -0.369063 | 772.665 | 772.785 | $-0 | -0.003 |
| xyz:SOXL | 4.26559 | 57.705 | 57.6935 | $-0 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -0.07, "FIL": 0.13, "TRX": 0.1, "UNI": 0.04, "kPEPE": -1.02}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ARB": -0.07, "FIL": 0.13, "TRX": 0.1, "UNI": 0.04, "kPEPE": -1.02}}`
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
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:PURRDAT | buy | $10,917 | 7.94377 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MSTR | buy | $10,900 | 101.7 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:CRCL | buy | $8,351 | 58.8308 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:COIN | buy | $8,016 | 121.002 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:HOOD | buy | $2,740 | 76.0676 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:ZHIPU | buy | $2,022 | 50.5065 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:CBRS | buy | $1,537 | 106.881 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:LITE | buy | $748 | 589.45 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:SNDK | buy | $746 | 873.385 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:INTC | buy | $456 | 58.9717 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:SOXL | buy | $246 | 57.705 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MU | sell | $285 | 772.665 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MRNA | sell | $344 | 221.486 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:CXMT | sell | $408 | 5.9968 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:ORCL | sell | $606 | 124.674 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:AMD | sell | $629 | 530.998 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MRVL | sell | $1,033 | 156.143 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:UNITREE | sell | $1,383 | 101.43 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:META | sell | $1,411 | 640.883 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:ANT | sell | $1,483 | 102.223 | weekend fade entry |
