# Paper trading status — 2026-11-21 02:04 UTC

BTC 59,004 · ETH 1,625.3 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $76,098 | -23.90% | -4.95% | -13.59% | -27.81% | -9.06 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,651 | -3.35% | -0.25% | -0.69% | -4.67% | -5.97 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.49 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.30 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,087 | +0.09% | +0.01% | -0.08% | -0.26% | 2.16 | 0.84 | +0.00 | 6 | $302 | $374 |
| trend_breakout_majors | $119,633 | +19.63% | +3.60% | +9.56% | -5.48% | 8.61 | 0.55 | -0.55 | 3 | $125 | $190 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.13 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.28 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,135 | +1.13% | +0.28% | +0.94% | -1.64% | 2.38 | 0.92 | +0.02 | 20 | $646 | $253 |
| xs_momentum | $99,152 | -0.85% | +0.31% | +0.32% | -3.01% | -1.47 | 0.89 | +0.01 | 36 | $229 | $26 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.53 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,120,052 on $1,100,000 → +1.82%**
Combined BTC exposure (7d realised, 4h returns): beta -0.084 (≈ -0.92 books of BTC), corr -0.93

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 59031 | $-23,832 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 59003.5 | $-1,282 | +0.047 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE | -11765.6 | 1.72106 | 1.2809 | $5,179 | -0.151 |
| PENDLE/spot | 11756.4 | 1.72251 | 1.2815 | $-5,185 | +0.151 |
| HYPE/spot | 233.128 | 83.2675 | 60.444 | $-5,321 | +0.141 |
| HYPE | -233.128 | 82.7772 | 60.4115 | $5,214 | -0.141 |
| ZEC/spot | 17.43 | 1175.27 | 746.74 | $-7,469 | +0.130 |
| ZEC | -17.4301 | 1175 | 746.5 | $7,469 | -0.130 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 59003.5 | $5,731 | -0.220 |
| ETH | -12.9689 | 2117.81 | 1625.35 | $6,387 | -0.176 |
| SOL | -265.603 | 89.0415 | 68.7515 | $5,389 | -0.153 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.301755 | $-210 | +0.085 |
| BTC | -0.144359 | 68333.1 | 59003.5 | $1,347 | -0.084 |
| ETC | 1191.56 | 5.48251 | 5.4797 | $-3 | +0.065 |
| LINK | -662.607 | 8.53769 | 8.55985 | $-15 | -0.056 |
| BCH | 29.1245 | 194.392 | 194.115 | $-8 | +0.056 |
| APT | 12637.8 | 0.436568 | 0.3628 | $-932 | +0.045 |
| RENDER | -3551.19 | 1.49617 | 1.27215 | $796 | -0.045 |
| TAO | -25.0244 | 220.686 | 180.105 | $1,016 | -0.045 |
| AAVE | -44.8727 | 112.188 | 98.084 | $633 | -0.044 |
| SEI | 123581 | 0.0376927 | 0.034985 | $-335 | +0.043 |
| PENDLE | -3236.78 | 1.54767 | 1.2809 | $863 | -0.041 |
| OP | 58569.1 | 0.0891484 | 0.06978 | $-1,134 | +0.040 |
| JUP | 18056.4 | 0.241363 | 0.22632 | $-272 | +0.040 |
| NEAR | -1371.79 | 3.51678 | 2.7906 | $996 | -0.038 |
| UNI | -836.22 | 5.16466 | 4.47925 | $573 | -0.037 |
| WLD | 15387.1 | 0.31305 | 0.22711 | $-1,322 | +0.035 |
| ENA | -33871.5 | 0.132226 | 0.098856 | $1,130 | -0.033 |
| TIA | 10070 | 0.345986 | 0.32813 | $-180 | +0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003453 | $-392 | +0.031 |
| ZEC | -3.83105 | 862.252 | 746.5 | $443 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.301755 | $-377 | +0.101 |
| BTC | -0.122056 | 65912.7 | 59003.5 | $843 | -0.073 |
| BNB | -11.3465 | 580.021 | 538.845 | $467 | -0.062 |
| ETH | -2.88908 | 1804.39 | 1625.35 | $517 | -0.047 |
| RENDER | 3386.49 | 1.53278 | 1.27215 | $-883 | +0.043 |
| ATOM | 3503.24 | 1.28664 | 1.2176 | $-242 | +0.043 |
| INJ | 607.666 | 7.33527 | 6.3228 | $-615 | +0.039 |
| SOL | -52.0808 | 91.9832 | 68.7515 | $1,210 | -0.036 |
| JUP | 15235.8 | 0.263176 | 0.22632 | $-562 | +0.035 |
| XRP | -3611.62 | 1.02442 | 0.909975 | $413 | -0.033 |
| AVAX | 424.946 | 7.89216 | 6.4964 | $-593 | +0.028 |
| BCH | -14.0941 | 212.452 | 194.115 | $258 | -0.028 |
| FIL | -4978.18 | 0.680775 | 0.53231 | $739 | -0.027 |
| DOGE | -44257.5 | 0.0643319 | 0.054561 | $432 | -0.024 |
| TAO | 12.9296 | 197.549 | 180.105 | $-226 | +0.023 |
| ENA | -21996.7 | 0.108918 | 0.098856 | $221 | -0.022 |
| ARB | 18212.8 | 0.152074 | 0.11292 | $-713 | +0.021 |
| CRV | -11195.5 | 0.271234 | 0.17869 | $1,036 | -0.020 |
| AAVE | 18.9438 | 116.539 | 98.084 | $-350 | +0.019 |
| TIA | 5612.94 | 0.336231 | 0.32813 | $-45 | +0.019 |
| kPEPE | 522031 | 0.00484647 | 0.003453 | $-727 | +0.018 |
| APT | -4613.4 | 0.476746 | 0.3628 | $526 | -0.017 |
| NEAR | 530.46 | 3.26892 | 2.7906 | $-254 | +0.015 |
| WLD | -5872.05 | 0.34434 | 0.22711 | $688 | -0.013 |
| XLM | 9051.42 | 0.140675 | 0.127115 | $-123 | +0.012 |
| LTC | -25.7819 | 40.5854 | 40.6425 | $-1 | -0.011 |
| LDO | 4201.67 | 0.267639 | 0.23668 | $-130 | +0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.051198 | $321 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.034985 | $-73 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.66705 | $210 | -0.008 |
| ZEC | -0.884394 | 901.025 | 746.5 | $137 | -0.007 |
| OP | 7393.63 | 0.107142 | 0.06978 | $-276 | +0.005 |
| PENDLE | -340.102 | 1.58296 | 1.2809 | $103 | -0.004 |
| UNI | 96.3288 | 7.08573 | 4.47925 | $-251 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.278295 | $-75 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.6463 | $-16 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.73, "AVAX": -0.97, "NEAR": -1.3, "ONDO": -0.92, "WLD": -1.34}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"APT": -1.73, "AVAX": -0.97, "NEAR": -1.3, "ONDO": -0.92, "WLD": -1.34}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 7.8, "PENDLE": 10.6, "ZEC": 9.8}, "last_day": "2026-11-21", "top_apr": {"AAVE": 4.8, "BTC": 6.7, "CRV": 7.7, "ENA": 5.2, "HYPE": 7.8, "INJ": 7.4, "LDO": 10.2, "NEAR": 4.7, "PENDLE": 10.6, "ZEC": 9.8}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-20T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-20T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-21"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-20T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-15 23:04: 36 trades, 38 names, weekend xs-dispersion 2.45%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-21T00:04 | dn_funding_carry | PENDLE/spot | buy | $2,677 | 1.27817 | daily dn-carry |
| 2026-11-21T00:04 | dn_funding_carry | PENDLE | sell | $2,671 | 1.27536 | daily dn-carry |
| 2026-11-21T00:04 | xs_momentum | SOL | buy | $1,486 | 68.8689 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | ATOM | buy | $1,385 | 1.22545 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | DOGE | buy | $1,175 | 0.0543447 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | TIA | buy | $1,040 | 0.328793 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | TRX | buy | $1,030 | 0.30183 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | LTC | sell | $1,046 | 40.5854 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_momentum | BNB | sell | $1,356 | 539.721 | signal 2026-11-20 23h |
| 2026-11-21T00:04 | xs_funding | ETC | buy | $6,533 | 5.48251 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | BCH | buy | $5,662 | 194.392 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | LDO | buy | $4,524 | 0.240151 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | APT | buy | $1,099 | 0.36855 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | RENDER | sell | $1,013 | 1.27002 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | NEAR | sell | $1,064 | 2.84195 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | INJ | sell | $3,421 | 6.31993 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | ADA | sell | $5,286 | 0.142965 | daily xs-funding (Binance recon) |
| 2026-11-21T00:04 | xs_funding | LINK | sell | $5,657 | 8.53769 | daily xs-funding (Binance recon) |
| 2026-11-20T00:04 | xs_momentum | DOT | buy | $1,353 | 0.715747 | signal 2026-11-19 23h |
| 2026-11-20T00:04 | xs_momentum | XRP | buy | $1,158 | 0.962098 | signal 2026-11-19 23h |
