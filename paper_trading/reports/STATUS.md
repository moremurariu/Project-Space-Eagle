# Paper trading status — 2026-11-23 18:04 UTC

BTC 61,658 · ETH 1,744.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $79,520 | -20.48% | +0.98% | -4.94% | -30.99% | -5.80 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,854 | -3.15% | +0.05% | -0.26% | -4.87% | -5.19 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.11 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,108 | +0.11% | -0.01% | +0.07% | -0.26% | 2.11 | 0.90 | +0.00 | 6 | $302 | $401 |
| trend_breakout_majors | $115,687 | +15.69% | -1.21% | +1.17% | -7.51% | 5.91 | 0.55 | -0.55 | 3 | $128 | $182 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.94 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.09 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.83 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,804 | +0.80% | +0.72% | -0.07% | -1.64% | 1.51 | 0.96 | +0.00 | 20 | $663 | $299 |
| xs_momentum | $98,667 | -1.33% | +0.34% | -0.03% | -3.13% | -2.05 | 0.95 | -0.01 | 35 | $243 | $52 |
| xyz_weekend_fade | $98,689 | -1.31% | -4.45% | -4.33% | -5.64% | -0.81 | 1.05 | +0.03 | 36 | $907 | $116 |

**Combined (ex-benchmark): $1,110,918 on $1,100,000 → +0.99%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.95 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 61681 | $-20,410 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 61657.5 | $-1,078 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.3384 | $-4,516 | +0.157 |
| PENDLE | -11765.6 | 1.72106 | 1.33655 | $4,524 | -0.157 |
| HYPE | -233.128 | 82.7772 | 65.533 | $4,020 | -0.153 |
| HYPE/spot | 233.128 | 83.2675 | 65.484 | $-4,146 | +0.152 |
| ZEC/spot | 17.43 | 1175.27 | 818.23 | $-6,223 | +0.142 |
| ZEC | -17.4301 | 1175 | 818.11 | $6,221 | -0.142 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 61657.5 | $4,070 | -0.212 |
| ETH | -11.3433 | 2117.81 | 1744.55 | $4,234 | -0.171 |
| SOL | -265.603 | 89.0415 | 73.7425 | $4,063 | -0.169 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 61657.5 | $964 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.311555 | $69 | +0.088 |
| LINK | -662.607 | 8.53769 | 9.09295 | $-368 | -0.060 |
| ADA | 39260.6 | 0.144249 | 0.150795 | $257 | +0.059 |
| BCH | 27.9335 | 199.151 | 202.545 | $95 | +0.056 |
| APT | 12637.8 | 0.436568 | 0.3912 | $-573 | +0.049 |
| TAO | -25.0244 | 220.686 | 194.465 | $656 | -0.048 |
| RENDER | -3551.19 | 1.49617 | 1.3384 | $560 | -0.047 |
| JUP | 18056.4 | 0.241363 | 0.247815 | $117 | +0.044 |
| AAVE | -44.8727 | 112.188 | 99.0525 | $589 | -0.044 |
| SEI | 123581 | 0.0376927 | 0.035482 | $-273 | +0.043 |
| PENDLE | -3236.78 | 1.54767 | 1.33655 | $683 | -0.043 |
| OP | 58569.1 | 0.0891484 | 0.07186 | $-1,013 | +0.042 |
| NEAR | -1371.79 | 3.51678 | 3.0593 | $628 | -0.042 |
| UNI | -836.22 | 5.16466 | 4.938 | $190 | -0.041 |
| WLD | 15387.1 | 0.31305 | 0.24268 | $-1,083 | +0.037 |
| ENA | -33871.5 | 0.132226 | 0.104455 | $941 | -0.035 |
| TIA | 10070 | 0.345986 | 0.34173 | $-43 | +0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003516 | $-336 | +0.031 |
| ZEC | -3.83105 | 862.252 | 818.11 | $169 | -0.031 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.311555 | $-51 | +0.105 |
| BTC | -0.122056 | 65912.7 | 61657.5 | $519 | -0.076 |
| BNB | -9.13414 | 580.021 | 561.935 | $165 | -0.052 |
| ETH | -2.88908 | 1804.39 | 1744.55 | $173 | -0.051 |
| RENDER | 3386.49 | 1.53278 | 1.3384 | $-658 | +0.046 |
| ATOM | 3503.24 | 1.28664 | 1.2762 | $-37 | +0.045 |
| FIL | -7861.93 | 0.620713 | 0.56807 | $414 | -0.045 |
| INJ | 607.666 | 7.33527 | 6.6412 | $-422 | +0.041 |
| JUP | 15235.8 | 0.263176 | 0.247815 | $-234 | +0.038 |
| CRV | -19201.4 | 0.232521 | 0.18506 | $911 | -0.036 |
| AAVE | 32.3234 | 108.591 | 99.0525 | $-308 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.3912 | $329 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.896 | $-423 | +0.030 |
| BCH | -14.0941 | 212.452 | 202.545 | $140 | -0.029 |
| LTC | -67.4945 | 40.3115 | 42.0425 | $-117 | -0.029 |
| NEAR | 890.083 | 3.11366 | 3.0593 | $-48 | +0.028 |
| DOGE | -44257.5 | 0.0643319 | 0.057381 | $308 | -0.026 |
| TAO | 12.9296 | 197.549 | 194.465 | $-40 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.104455 | $98 | -0.023 |
| HBAR | -38109.1 | 0.0598269 | 0.054102 | $218 | -0.021 |
| TIA | 5612.94 | 0.336231 | 0.34173 | $31 | +0.019 |
| SOL | -21.0355 | 91.9832 | 73.7425 | $384 | -0.016 |
| WLD | -5872.05 | 0.34434 | 0.24268 | $597 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.957015 | $97 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.132005 | $-78 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11453 | $-342 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.33655 | $47 | +0.010 |
| SEI | 23468.7 | 0.0381037 | 0.035482 | $-62 | +0.008 |
| DOT | -1201.93 | 0.841601 | 0.69059 | $182 | -0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003516 | $-230 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07186 | $-9 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.938 | $-207 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.24593 | $-22 | -0.004 |
| ZEC | 0.47487 | 796.75 | 818.11 | $10 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.294995 | $-64 | +0.002 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:SOXL | -264.497 | 33.7478 | 43.038 | $-2,457 | -0.115 |
| xyz:COST | 5.37885 | 1103.42 | 1079.8 | $-127 | +0.059 |
| xyz:NBIS | -56.3175 | 86.8826 | 98.704 | $-666 | -0.056 |
| xyz:SP500 | 0.655074 | 7002.81 | 7247.95 | $161 | +0.048 |
| xyz:UNITREE | 42.2589 | 90.2361 | 109.04 | $795 | +0.047 |
| xyz:NVDA | -27.6065 | 133.548 | 164.165 | $-845 | -0.046 |
| xyz:STRC | 43.9632 | 95.0508 | 97.647 | $114 | +0.043 |
| xyz:ANT | 46.3349 | 89.5172 | 89.21 | $-14 | +0.042 |
| xyz:AAPL | 11.2936 | 348.333 | 348.815 | $5 | +0.040 |
| xyz:META | 6.33532 | 571.237 | 596.155 | $158 | +0.038 |
| xyz:XYZ100 | 0.138214 | 25535.2 | 26770.5 | $171 | +0.037 |
| xyz:IBM | 15.4686 | 227.775 | 234.015 | $97 | +0.037 |
| xyz:MSTR | -40.5579 | 82.455 | 85.602 | $-128 | -0.035 |
| xyz:CBRS | -32.3279 | 86.9154 | 97.52 | $-343 | -0.032 |
| xyz:SKHY | -23.7567 | 109.14 | 124.73 | $-370 | -0.030 |
| xyz:AMZN | 11.2909 | 252.591 | 258.745 | $69 | +0.030 |
| xyz:SPCX | 20.186 | 130.902 | 136.215 | $107 | +0.028 |
| xyz:MSFT | 5.37955 | 495.48 | 497.365 | $10 | +0.027 |
| xyz:CRCL | -51.3591 | 50.1159 | 51.873 | $-90 | -0.027 |
| xyz:BABA | 30.2312 | 83.0533 | 86.654 | $109 | +0.027 |
| xyz:SKHX | -2.87179 | 779.493 | 879.485 | $-287 | -0.026 |
| xyz:IREN | -166.006 | 12.8106 | 14.8485 | $-338 | -0.025 |
| xyz:ZHIPU | -45.2724 | 38.8205 | 46.305 | $-339 | -0.021 |
| xyz:PURRDAT | -209.653 | 7.87569 | 8.3779 | $-105 | -0.018 |
| xyz:SNDK | -18.9852 | 83.0642 | 90.457 | $-140 | -0.017 |
| xyz:HOOD | -24.3395 | 65.1215 | 67.732 | $-64 | -0.017 |
| xyz:GOOGL | 5.77104 | 273.323 | 283.82 | $61 | +0.017 |
| xyz:INTC | -19.5296 | 53.6475 | 58.261 | $-90 | -0.012 |
| xyz:AMD | -2.18996 | 442.79 | 458.275 | $-34 | -0.010 |
| xyz:TSLA | 2.4212 | 357.605 | 388.405 | $75 | +0.010 |
| xyz:MU | 1.06539 | 615.696 | 706.81 | $97 | +0.008 |
| xyz:MRVL | -4.33833 | 127.25 | 144.495 | $-75 | -0.006 |
| xyz:EWY | 4.12068 | 127.505 | 136.255 | $36 | +0.006 |
| xyz:DRAM | -13.9588 | 35.0429 | 39.748 | $-66 | -0.006 |
| xyz:CXMT | -83.1059 | 5.0613 | 5.71395 | $-54 | -0.005 |
| xyz:ORCL | -3.21642 | 104.62 | 113.855 | $-30 | -0.004 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -3.74, "ENA": -5.1, "ETH": -3.7, "ONDO": -3.44, "SOL": -3.8}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -3.74, "ENA": -5.1, "ETH": -3.7, "ONDO": -3.44, "SOL": -3.8}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 10.2, "PENDLE": 9.9, "ZEC": 10.0}, "last_day": "2026-11-23", "top_apr": {"BTC": 4.9, "CRV": 5.8, "ENA": 9.0, "ETC": 5.3, "HYPE": 10.2, "LDO": 8.7, "LINK": 5.2, "PENDLE": 9.9, "RENDER": 7.4, "ZEC": 10.0}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-23"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-22T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-22 23:04: 36 trades, 38 names, weekend xs-dispersion 2.05%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-23T00:04 | xs_momentum | BNB | buy | $1,238 | 559.664 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | ZEC | buy | $1,083 | 796.75 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | SOL | buy | $1,066 | 70.632 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | SUI | sell | $44 | 0.655817 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | HBAR | sell | $1,113 | 0.0515479 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | kPEPE | sell | $1,179 | 0.00337961 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_momentum | LTC | sell | $1,674 | 40.1422 | signal 2026-11-22 23h |
| 2026-11-23T00:04 | xs_funding | BCH | buy | $5,563 | 199.151 | daily xs-funding (Binance recon) |
| 2026-11-23T00:04 | xs_funding | ETC | sell | $6,775 | 5.68563 | daily xs-funding (Binance recon) |
| 2026-11-23T00:04 | trend_breakout_majors | BTC | buy | $2,875 | 61184.6 | signal 2026-11-22 23h |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:COST | buy | $5,935 | 1103.42 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:SP500 | buy | $4,587 | 7002.81 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:STRC | buy | $4,179 | 95.0508 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:ANT | buy | $4,148 | 89.5172 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:AAPL | buy | $3,934 | 348.333 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:UNITREE | buy | $3,813 | 90.2361 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:META | buy | $3,619 | 571.237 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:XYZ100 | buy | $3,529 | 25535.2 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:IBM | buy | $3,523 | 227.775 | weekend fade entry |
| 2026-11-22T23:04 | xyz_weekend_fade | xyz:AMZN | buy | $2,852 | 252.591 | weekend fade entry |
