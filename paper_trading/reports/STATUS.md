# Paper trading status — 2026-11-16 20:04 UTC

BTC 65,028 · ETH 1,822.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $83,875 | -16.13% | +0.33% | -12.96% | -22.40% | -7.66 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,119 | -2.88% | +0.02% | -1.11% | -4.33% | -5.80 | 0.05 | +0.05 | 1 | $53 | $-97 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.55 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.72 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,011 | +0.01% | -0.00% | -0.10% | -0.26% | 1.79 | 0.92 | -0.00 | 6 | $296 | $321 |
| trend_breakout_majors | $114,115 | +14.12% | -0.11% | +12.33% | -5.48% | 8.02 | 0.53 | -0.53 | 3 | $116 | $171 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.52 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.69 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,671 | +0.67% | -0.42% | +1.26% | -1.56% | 1.99 | 0.93 | -0.00 | 20 | $581 | $161 |
| xs_momentum | $98,759 | -1.24% | -0.33% | -1.46% | -2.98% | -2.95 | 0.91 | -0.03 | 37 | $195 | $-3 |
| xyz_weekend_fade | $103,559 | +3.56% | +2.31% | +2.72% | -1.21% | 6.83 | 0.95 | -0.05 | 36 | $668 | $30 |

**Combined (ex-benchmark): $1,114,342 on $1,100,000 → +1.30%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.95 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 65054 | $-16,055 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 65028.5 | $-818 | +0.052 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE | -233.128 | 82.7772 | 68.8365 | $3,250 | -0.160 |
| HYPE/spot | 233.128 | 83.2675 | 68.8025 | $-3,372 | +0.160 |
| ZEC | -17.4301 | 1175 | 905.055 | $4,705 | -0.158 |
| ZEC/spot | 17.43 | 1175.27 | 904.298 | $-4,723 | +0.158 |
| PENDLE | -9671.02 | 1.81759 | 1.4459 | $3,595 | -0.140 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.44685 | $-3,594 | +0.140 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.309603 | 75630.7 | 65028.5 | $3,282 | -0.176 |
| SOL | -281.278 | 91.199 | 71.5555 | $5,525 | -0.176 |
| ETH | -10.8509 | 2187.58 | 1822.45 | $3,962 | -0.173 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 65028.5 | $477 | -0.093 |
| TRX | 28470.1 | 0.309116 | 0.30577 | $-95 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.960055 | $1,579 | -0.071 |
| ADA | 36975.2 | 0.155319 | 0.153855 | $-54 | +0.057 |
| BCH | 23.4344 | 238.546 | 208.5 | $-704 | +0.049 |
| SEI | 123581 | 0.0376927 | 0.03924 | $191 | +0.048 |
| TAO | -25.0244 | 220.686 | 191.53 | $730 | -0.048 |
| LINK | -516.66 | 11.3839 | 9.2016 | $1,128 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.247555 | $112 | +0.044 |
| APT | 9654.61 | 0.457585 | 0.4605 | $28 | +0.044 |
| LDO | -14834.4 | 0.308439 | 0.28004 | $421 | -0.041 |
| RENDER | -2753.67 | 1.56167 | 1.378 | $506 | -0.038 |
| PENDLE | -2505.4 | 1.58072 | 1.4459 | $338 | -0.036 |
| ARB | 27302.4 | 0.151348 | 0.13261 | $-512 | +0.036 |
| TIA | 10070 | 0.345986 | 0.35875 | $129 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08047 | $-540 | +0.035 |
| ZEC | -3.83105 | 862.252 | 905.055 | $-164 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.19005 | $579 | -0.032 |
| WLD | 10593 | 0.336516 | 0.26423 | $-766 | +0.028 |
| ENA | -22442.6 | 0.146112 | 0.11718 | $649 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30577 | $-241 | +0.080 |
| SOL | -87.2593 | 91.9832 | 71.5555 | $1,783 | -0.063 |
| XRP | -6239.14 | 1.02442 | 0.960055 | $402 | -0.061 |
| RENDER | 3386.49 | 1.53278 | 1.378 | $-524 | +0.047 |
| BTC | -0.0697602 | 67434.2 | 65028.5 | $168 | -0.046 |
| AVAX | 607.041 | 7.89216 | 7.1875 | $-428 | +0.044 |
| INJ | 607.666 | 7.33527 | 6.93065 | $-246 | +0.043 |
| ARB | 29979.9 | 0.152074 | 0.13261 | $-584 | +0.040 |
| JUP | 15235.8 | 0.263176 | 0.247555 | $-238 | +0.038 |
| PENDLE | -2522.74 | 1.58296 | 1.4459 | $346 | -0.037 |
| DOT | -4637.48 | 0.841601 | 0.7301 | $517 | -0.034 |
| kPEPE | 803792 | 0.00484647 | 0.003929 | $-737 | +0.032 |
| BCH | -14.0941 | 212.452 | 208.5 | $56 | -0.030 |
| DOGE | -46908.3 | 0.0665749 | 0.05958 | $328 | -0.028 |
| ETH | -1.46269 | 1817.83 | 1822.45 | $-7 | -0.027 |
| WLD | -9976.36 | 0.34434 | 0.26423 | $799 | -0.027 |
| CRV | -11195.5 | 0.271234 | 0.20587 | $732 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.08047 | $-643 | +0.020 |
| UNI | 341.169 | 7.08573 | 5.14745 | $-661 | +0.018 |
| ATOM | 1186.94 | 1.29824 | 1.3382 | $47 | +0.016 |
| ADA | -9776.31 | 0.190675 | 0.153855 | $360 | -0.015 |
| TAO | 7.68451 | 200.727 | 191.53 | $-71 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.61237 | $438 | -0.015 |
| SEI | -33311.6 | 0.0472943 | 0.03924 | $268 | -0.013 |
| BNB | -2.14595 | 596.735 | 592.05 | $10 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.137495 | $68 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.28004 | $52 | +0.012 |
| ENA | -8394.07 | 0.115324 | 0.11718 | $-16 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057831 | $212 | -0.010 |
| AAVE | 8.4283 | 131.483 | 107.965 | $-198 | +0.009 |
| TIA | 2451.15 | 0.345826 | 0.35875 | $32 | +0.009 |
| APT | -1385.4 | 0.626089 | 0.4605 | $229 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 905.055 | $-192 | +0.006 |
| NEAR | 164.539 | 3.21111 | 3.19005 | $-3 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.309165 | $-55 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.080021 | $-26 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.724 | $-11 | +0.000 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:PURRDAT | 1374.3 | 7.94377 | 8.34445 | $551 | +0.111 |
| xyz:MSTR | 107.175 | 101.7 | 103.82 | $227 | +0.107 |
| xyz:CRCL | 141.952 | 58.8308 | 57.836 | $-141 | +0.079 |
| xyz:COIN | 66.2433 | 121.002 | 120.33 | $-45 | +0.077 |
| xyz:NVDA | -35.5169 | 166.557 | 156.06 | $373 | -0.054 |
| xyz:NBIS | -38.985 | 117.666 | 106.86 | $421 | -0.040 |
| xyz:AAPL | -10.0333 | 359.898 | 351.975 | $79 | -0.034 |
| xyz:SKHX | -4.04354 | 905.53 | 830.16 | $305 | -0.032 |
| xyz:SKHY | -25.0067 | 132.85 | 124.05 | $220 | -0.030 |
| xyz:SPCX | -20.038 | 140 | 138.855 | $23 | -0.027 |
| xyz:TSLA | -7.31388 | 392.66 | 376.99 | $115 | -0.027 |
| xyz:HOOD | 36.023 | 76.0676 | 72.044 | $-145 | +0.025 |
| xyz:IBM | -10.5453 | 248.34 | 242.155 | $65 | -0.025 |
| xyz:XYZ100 | -0.092663 | 28069.1 | 27252.5 | $76 | -0.024 |
| xyz:MSFT | -4.84152 | 526.191 | 518.695 | $36 | -0.024 |
| xyz:GOOGL | -7.9314 | 296.85 | 284.855 | $95 | -0.022 |
| xyz:SMSN | -15.2881 | 153.075 | 146.175 | $105 | -0.022 |
| xyz:SP500 | -0.272659 | 7575.27 | 7420.95 | $42 | -0.020 |
| xyz:AMZN | -7.79693 | 267.121 | 258.075 | $71 | -0.019 |
| xyz:EWY | -14.158 | 142.995 | 137.805 | $73 | -0.019 |
| xyz:ZHIPU | 40.034 | 50.5065 | 44.147 | $-255 | +0.017 |
| xyz:CBRS | 14.3825 | 106.881 | 101.88 | $-72 | +0.014 |
| xyz:ANT | -14.5089 | 102.223 | 98.096 | $60 | -0.014 |
| xyz:DRAM | -35.1899 | 42.3639 | 39.7685 | $91 | -0.014 |
| xyz:UNITREE | -13.6311 | 101.43 | 101.98 | $-7 | -0.013 |
| xyz:META | -2.20162 | 640.883 | 627 | $31 | -0.013 |
| xyz:MRVL | -6.61861 | 156.143 | 147.045 | $60 | -0.009 |
| xyz:SNDK | 0.85415 | 873.385 | 817.1 | $-48 | +0.007 |
| xyz:LITE | 1.26894 | 589.45 | 548.11 | $-52 | +0.007 |
| xyz:AMD | -1.18441 | 530.998 | 506.655 | $29 | -0.006 |
| xyz:ORCL | -4.86148 | 124.674 | 118.82 | $28 | -0.006 |
| xyz:INTC | 7.74056 | 58.9717 | 55.685 | $-25 | +0.004 |
| xyz:CXMT | -68.0257 | 5.9968 | 5.65265 | $23 | -0.004 |
| xyz:MRNA | -1.55358 | 221.486 | 216.335 | $8 | -0.003 |
| xyz:MU | -0.369063 | 772.665 | 726.27 | $17 | -0.003 |
| xyz:SOXL | 4.26559 | 57.705 | 50.2015 | $-32 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.1, "ENA": 0.17, "ETH": 0.21, "HBAR": 0.24, "JUP": -0.16}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -0.1, "ENA": 0.17, "ETH": 0.21, "HBAR": 0.24, "JUP": -0.16}}`
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
| 2026-11-16T00:04 | xs_momentum | RENDER | buy | $1,300 | 1.40551 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | JUP | buy | $1,294 | 0.266987 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | NEAR | buy | $1,028 | 3.21111 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | ADA | buy | $1,027 | 0.155296 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | TAO | buy | $1,002 | 195.145 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | LTC | buy | $147 | 45.4998 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | LINK | sell | $721 | 9.29306 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | BTC | sell | $1,028 | 65324.2 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | AAVE | sell | $1,071 | 109.844 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | UNI | sell | $1,326 | 5.16872 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_momentum | BNB | sell | $1,828 | 596.735 | signal 2026-11-15 23h |
| 2026-11-16T00:04 | xs_funding | ADA | buy | $5,743 | 0.155319 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | APT | buy | $4,418 | 0.457585 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | ATOM | sell | $5,739 | 1.3098 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | xs_funding | XLM | sell | $6,275 | 0.141149 | daily xs-funding (Binance recon) |
| 2026-11-16T00:04 | trend_breakout_majors | BTC | buy | $3,078 | 65330.8 | signal 2026-11-15 23h |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:PURRDAT | buy | $10,917 | 7.94377 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:MSTR | buy | $10,900 | 101.7 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:CRCL | buy | $8,351 | 58.8308 | weekend fade entry |
| 2026-11-15T23:04 | xyz_weekend_fade | xyz:COIN | buy | $8,016 | 121.002 | weekend fade entry |
