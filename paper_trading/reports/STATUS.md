# Paper trading status — 2026-11-20 14:04 UTC

BTC 61,806 · ETH 1,750.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $79,719 | -20.28% | -3.10% | -10.89% | -23.83% | -8.18 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,867 | -3.13% | -0.16% | -0.54% | -4.43% | -5.67 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.50 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.35 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,082 | +0.08% | +0.00% | -0.04% | -0.26% | 2.15 | 0.83 | -0.00 | 6 | $296 | $369 |
| trend_breakout_majors | $116,096 | +16.10% | +2.02% | +8.30% | -5.48% | 7.69 | 0.60 | -0.60 | 3 | $125 | $195 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.17 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.32 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,579 | +0.58% | -0.00% | -0.19% | -1.64% | 1.44 | 0.88 | +0.00 | 20 | $609 | $238 |
| xs_momentum | $98,111 | -1.89% | -1.09% | -0.21% | -3.01% | -3.78 | 0.90 | -0.04 | 35 | $223 | $15 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.60 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,115,129 on $1,100,000 → +1.38%**
Combined BTC exposure (7d realised, 4h returns): beta -0.088 (≈ -0.97 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 61835.4 | $-20,211 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 61806.5 | $-1,066 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE | -233.128 | 82.7772 | 62.9795 | $4,615 | -0.147 |
| HYPE/spot | 233.128 | 83.2675 | 62.9565 | $-4,735 | +0.147 |
| ZEC | -17.4301 | 1175 | 813.975 | $6,293 | -0.142 |
| ZEC/spot | 17.43 | 1175.27 | 813.97 | $-6,297 | +0.142 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.3315 | $-4,709 | +0.129 |
| PENDLE | -9671.02 | 1.81759 | 1.3299 | $4,716 | -0.129 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 61806.5 | $4,483 | -0.237 |
| ETH | -12.9689 | 2117.81 | 1750.45 | $4,764 | -0.196 |
| SOL | -265.603 | 89.0415 | 71.2815 | $4,717 | -0.163 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 61806.5 | $942 | -0.089 |
| TRX | 28470.1 | 0.309116 | 0.305345 | $-107 | +0.086 |
| ADA | 36975.2 | 0.155319 | 0.14637 | $-331 | +0.054 |
| LDO | -18837.8 | 0.298153 | 0.25137 | $881 | -0.047 |
| TAO | -25.0244 | 220.686 | 185.435 | $882 | -0.046 |
| AAVE | -44.8727 | 112.188 | 101.815 | $465 | -0.045 |
| SEI | 123581 | 0.0376927 | 0.036801 | $-110 | +0.045 |
| PENDLE | -3236.78 | 1.54767 | 1.3299 | $705 | -0.043 |
| OP | 58569.1 | 0.0891484 | 0.07273 | $-962 | +0.042 |
| JUP | 18056.4 | 0.241363 | 0.23202 | $-169 | +0.042 |
| UNI | -836.22 | 5.16466 | 4.72665 | $366 | -0.039 |
| APT | 9654.61 | 0.457585 | 0.4021 | $-536 | +0.039 |
| WLD | 15387.1 | 0.31305 | 0.24158 | $-1,100 | +0.037 |
| RENDER | -2753.67 | 1.56167 | 1.32895 | $641 | -0.036 |
| ENA | -33871.5 | 0.132226 | 0.10243 | $1,009 | -0.034 |
| INJ | 541.319 | 7.41463 | 6.23335 | $-639 | +0.034 |
| TIA | 10070 | 0.345986 | 0.33083 | $-153 | +0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003564 | $-293 | +0.032 |
| ZEC | -3.83105 | 862.252 | 813.975 | $185 | -0.031 |
| NEAR | -997.435 | 3.77006 | 2.9648 | $803 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 29868.9 | 0.314369 | 0.305345 | $-270 | +0.093 |
| BTC | -0.122056 | 65912.7 | 61806.5 | $501 | -0.077 |
| SOL | -73.6612 | 91.9832 | 71.2815 | $1,525 | -0.054 |
| ETH | -2.88908 | 1804.39 | 1750.45 | $156 | -0.052 |
| BNB | -8.83459 | 591.479 | 569.035 | $198 | -0.051 |
| RENDER | 3386.49 | 1.53278 | 1.32895 | $-690 | +0.046 |
| INJ | 607.666 | 7.33527 | 6.23335 | $-670 | +0.039 |
| DOGE | -65877 | 0.0643319 | 0.056559 | $512 | -0.038 |
| JUP | 15235.8 | 0.263176 | 0.23202 | $-475 | +0.036 |
| XRP | -3611.62 | 1.02442 | 0.947545 | $278 | -0.035 |
| ATOM | 2372.89 | 1.31579 | 1.2541 | $-146 | +0.030 |
| AVAX | 424.946 | 7.89216 | 6.826 | $-453 | +0.030 |
| BCH | -14.0941 | 212.452 | 199.71 | $180 | -0.029 |
| FIL | -4978.18 | 0.680775 | 0.55689 | $617 | -0.028 |
| TAO | 12.9296 | 197.549 | 185.435 | $-157 | +0.024 |
| ENA | -21996.7 | 0.108918 | 0.10243 | $143 | -0.023 |
| ARB | 18212.8 | 0.152074 | 0.11826 | $-616 | +0.022 |
| CRV | -11195.5 | 0.271234 | 0.18704 | $943 | -0.021 |
| AAVE | 18.9438 | 116.539 | 101.815 | $-279 | +0.020 |
| kPEPE | 522031 | 0.00484647 | 0.003564 | $-669 | +0.019 |
| APT | -4613.4 | 0.476746 | 0.4021 | $344 | -0.019 |
| NEAR | 530.46 | 3.26892 | 2.9648 | $-161 | +0.016 |
| WLD | -5872.05 | 0.34434 | 0.24158 | $603 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.131545 | $-83 | +0.012 |
| LDO | 4201.67 | 0.267639 | 0.25137 | $-68 | +0.011 |
| HBAR | -16510.2 | 0.0706575 | 0.052969 | $292 | -0.009 |
| SEI | 23468.7 | 0.0381037 | 0.036801 | $-31 | +0.009 |
| DOT | -1201.93 | 0.841601 | 0.69004 | $182 | -0.008 |
| TIA | 2451.15 | 0.345826 | 0.33083 | $-37 | +0.008 |
| ZEC | -0.884394 | 901.025 | 813.975 | $77 | -0.007 |
| OP | 7393.63 | 0.107142 | 0.07273 | $-254 | +0.005 |
| UNI | 96.3288 | 7.08573 | 4.72665 | $-227 | +0.005 |
| PENDLE | -340.102 | 1.58296 | 1.3299 | $86 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.29192 | $-66 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.68218 | $-14 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"CRV": -2.39, "INJ": -6.12, "RENDER": -1.99, "WLD": -2.54, "ZEC": -2.53}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"CRV": -2.39, "INJ": -6.12, "RENDER": -1.99, "WLD": -2.54, "ZEC": -2.53}}`
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
