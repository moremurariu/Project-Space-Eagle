# Paper trading status — 2026-11-20 03:04 UTC

BTC 62,070 · ETH 1,744.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $80,071 | -19.93% | -1.15% | -12.41% | -23.58% | -8.16 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,887 | -3.11% | -0.06% | -0.62% | -4.41% | -5.70 | 0.05 | +0.05 | 1 | $53 | $-101 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.51 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.39 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,095 | +0.10% | +0.03% | -0.09% | -0.26% | 2.30 | 0.87 | +0.00 | 6 | $296 | $363 |
| trend_breakout_majors | $115,681 | +15.68% | +0.77% | +9.24% | -5.48% | 7.63 | 0.60 | -0.60 | 3 | $125 | $192 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.21 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.36 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,579 | +0.58% | +0.21% | -0.60% | -1.64% | 1.47 | 0.90 | +0.00 | 20 | $609 | $222 |
| xs_momentum | $98,448 | -1.55% | -0.70% | +0.00% | -2.98% | -3.11 | 0.91 | -0.04 | 35 | $223 | $10 |
| xyz_weekend_fade | $103,286 | +3.29% | +0.00% | +2.04% | -1.21% | 5.68 | 0.00 | +0.00 | 0 | $785 | $29 |

**Combined (ex-benchmark): $1,115,084 on $1,100,000 → +1.37%**
Combined BTC exposure (7d realised, 4h returns): beta -0.089 (≈ -0.98 books of BTC), corr -0.91

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 62108.3 | $-19,859 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 62069.5 | $-1,046 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE/spot | 233.128 | 83.2675 | 65.6955 | $-4,097 | +0.153 |
| HYPE | -233.128 | 82.7772 | 65.639 | $3,995 | -0.153 |
| ZEC/spot | 17.43 | 1175.27 | 868.8 | $-5,342 | +0.151 |
| ZEC | -17.4301 | 1175 | 868.505 | $5,342 | -0.151 |
| PENDLE | -9671.02 | 1.81759 | 1.35235 | $4,499 | -0.131 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.3535 | $-4,496 | +0.131 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.445135 | 71878.7 | 62069.5 | $4,366 | -0.239 |
| ETH | -12.9689 | 2117.81 | 1744.05 | $4,847 | -0.196 |
| SOL | -265.603 | 89.0415 | 72.7045 | $4,339 | -0.167 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 62069.5 | $904 | -0.089 |
| TRX | 28470.1 | 0.309116 | 0.309125 | $0 | +0.088 |
| ADA | 36975.2 | 0.155319 | 0.14915 | $-228 | +0.055 |
| LDO | -18837.8 | 0.298153 | 0.25555 | $803 | -0.048 |
| TAO | -25.0244 | 220.686 | 188.79 | $798 | -0.047 |
| SEI | 123581 | 0.0376927 | 0.037473 | $-27 | +0.046 |
| AAVE | -44.8727 | 112.188 | 103.185 | $404 | -0.046 |
| PENDLE | -3236.78 | 1.54767 | 1.35235 | $632 | -0.044 |
| OP | 58569.1 | 0.0891484 | 0.0731 | $-940 | +0.043 |
| JUP | 18056.4 | 0.241363 | 0.236885 | $-81 | +0.043 |
| UNI | -836.22 | 5.16466 | 4.84235 | $270 | -0.040 |
| APT | 9654.61 | 0.457585 | 0.4027 | $-530 | +0.039 |
| WLD | 15387.1 | 0.31305 | 0.24465 | $-1,052 | +0.037 |
| RENDER | -2753.67 | 1.56167 | 1.3531 | $574 | -0.037 |
| INJ | 541.319 | 7.41463 | 6.73435 | $-368 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.1022 | $1,017 | -0.034 |
| TIA | 10070 | 0.345986 | 0.33654 | $-95 | +0.034 |
| ZEC | -3.83105 | 862.252 | 868.505 | $-24 | -0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003582 | $-277 | +0.032 |
| NEAR | -997.435 | 3.77006 | 3.05005 | $718 | -0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 29868.9 | 0.314369 | 0.309125 | $-157 | +0.094 |
| BTC | -0.122056 | 65912.7 | 62069.5 | $469 | -0.077 |
| SOL | -73.6612 | 91.9832 | 72.7045 | $1,420 | -0.054 |
| BNB | -8.83459 | 591.479 | 573.475 | $159 | -0.051 |
| ETH | -2.88908 | 1804.39 | 1744.05 | $174 | -0.051 |
| RENDER | 3386.49 | 1.53278 | 1.3531 | $-608 | +0.047 |
| INJ | 607.666 | 7.33527 | 6.73435 | $-365 | +0.042 |
| DOGE | -65877 | 0.0643319 | 0.057414 | $456 | -0.038 |
| JUP | 15235.8 | 0.263176 | 0.236885 | $-401 | +0.037 |
| XRP | -3611.62 | 1.02442 | 0.954915 | $251 | -0.035 |
| ATOM | 2372.89 | 1.31579 | 1.2495 | $-157 | +0.030 |
| AVAX | 424.946 | 7.89216 | 6.8975 | $-423 | +0.030 |
| BCH | -14.0941 | 212.452 | 205.44 | $99 | -0.029 |
| FIL | -4978.18 | 0.680775 | 0.55697 | $616 | -0.028 |
| TAO | 12.9296 | 197.549 | 188.79 | $-113 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.1022 | $148 | -0.023 |
| ARB | 18212.8 | 0.152074 | 0.11816 | $-618 | +0.022 |
| CRV | -11195.5 | 0.271234 | 0.19011 | $908 | -0.022 |
| AAVE | 18.9438 | 116.539 | 103.185 | $-253 | +0.020 |
| kPEPE | 522031 | 0.00484647 | 0.003582 | $-660 | +0.019 |
| APT | -4613.4 | 0.476746 | 0.4027 | $342 | -0.019 |
| NEAR | 530.46 | 3.26892 | 3.05005 | $-116 | +0.016 |
| WLD | -5872.05 | 0.34434 | 0.24465 | $585 | -0.015 |
| XLM | 9051.42 | 0.140675 | 0.132725 | $-72 | +0.012 |
| LDO | 4201.67 | 0.267639 | 0.25555 | $-51 | +0.011 |
| SEI | 23468.7 | 0.0381037 | 0.037473 | $-15 | +0.009 |
| HBAR | -16510.2 | 0.0706575 | 0.053213 | $288 | -0.009 |
| DOT | -1201.93 | 0.841601 | 0.70175 | $168 | -0.009 |
| TIA | 2451.15 | 0.345826 | 0.33654 | $-23 | +0.008 |
| ZEC | -0.884394 | 901.025 | 868.505 | $29 | -0.008 |
| OP | 7393.63 | 0.107142 | 0.0731 | $-252 | +0.005 |
| UNI | 96.3288 | 7.08573 | 4.84235 | $-216 | +0.005 |
| PENDLE | -340.102 | 1.58296 | 1.35235 | $78 | -0.005 |
| ONDO | 657.524 | 0.392809 | 0.291255 | $-67 | +0.002 |
| SUI | 66.7499 | 0.886044 | 0.70008 | $-12 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-18 18:10 (due 11-19): hold token BTC 0.05. BTC 63.1k (range 62.8-65.5k for 3 days after the 11-15 squeeze), F&G 10-13 for a week, funding back to +4-6% APR on majors (the shorts have been squeezed out). Macro turned more hawkish: Fed's Waller floated a 'shock' 50bp hike, gold -3%, oil -4%/24h. $60k is a visible bid level. No edge from fear readings (r64), systematic books are net short (beta -0.09); keep the small long as a squeeze hedge. Rules unchanged: reclaim 72k -> 0.15; 78k -> 0.20; daily close < 55k -> 0. Review by 11-23."}`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"INJ": -4.29, "JUP": -4.43, "NEAR": -3.94, "WLD": -2.99, "kPEPE": -2.96}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"INJ": -4.29, "JUP": -4.43, "NEAR": -3.94, "WLD": -2.99, "kPEPE": -2.96}}`
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
