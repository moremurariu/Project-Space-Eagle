# Paper trading status — 2026-11-23 15:04 UTC

BTC 63,168 · ETH 1,784.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $81,467 | -18.53% | +3.53% | -1.47% | -30.99% | -5.19 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,971 | -3.03% | +0.17% | -0.08% | -4.87% | -5.01 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.47 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.12 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,091 | +0.09% | -0.04% | +0.06% | -0.26% | 1.99 | 0.94 | -0.00 | 6 | $302 | $400 |
| trend_breakout_majors | $113,863 | +13.86% | -2.87% | -0.85% | -7.51% | 5.32 | 0.58 | -0.58 | 3 | $128 | $182 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.95 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.10 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.85 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,285 | +0.29% | +0.26% | -0.85% | -1.64% | 0.73 | 1.00 | -0.00 | 20 | $663 | $296 |
| xs_momentum | $98,158 | -1.84% | -0.09% | -0.71% | -3.13% | -2.93 | 0.98 | -0.02 | 35 | $243 | $52 |
| xyz_weekend_fade | $98,044 | -1.96% | -5.08% | -4.00% | -5.64% | -1.28 | 1.06 | +0.03 | 36 | $907 | $100 |

**Combined (ex-benchmark): $1,107,519 on $1,100,000 → +0.68%**
Combined BTC exposure (7d realised, 4h returns): beta -0.083 (≈ -0.91 books of BTC), corr -0.84

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 63189.3 | $-18,463 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 63168.5 | $-961 | +0.050 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.37141 | $-4,128 | +0.161 |
| PENDLE | -11765.6 | 1.72106 | 1.3693 | $4,139 | -0.161 |
| HYPE | -233.128 | 82.7772 | 67.8925 | $3,470 | -0.158 |
| HYPE/spot | 233.128 | 83.2675 | 67.8488 | $-3,595 | +0.158 |
| ZEC | -17.4301 | 1175 | 868.815 | $5,337 | -0.151 |
| ZEC/spot | 17.43 | 1175.27 | 867.865 | $-5,358 | +0.151 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 63168.5 | $3,468 | -0.221 |
| SOL | -265.603 | 89.0415 | 76.6545 | $3,290 | -0.179 |
| ETH | -11.3433 | 2117.81 | 1784.15 | $3,785 | -0.178 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 63168.5 | $746 | -0.091 |
| TRX | 28470.1 | 0.309116 | 0.310915 | $51 | +0.088 |
| LINK | -662.607 | 8.53769 | 9.33835 | $-531 | -0.062 |
| ADA | 39260.6 | 0.144249 | 0.15427 | $393 | +0.060 |
| BCH | 27.9335 | 199.151 | 210.075 | $305 | +0.059 |
| APT | 12637.8 | 0.436568 | 0.4026 | $-429 | +0.051 |
| TAO | -25.0244 | 220.686 | 198.97 | $543 | -0.050 |
| RENDER | -3551.19 | 1.49617 | 1.3833 | $401 | -0.049 |
| AAVE | -44.8727 | 112.188 | 103.195 | $404 | -0.046 |
| JUP | 18056.4 | 0.241363 | 0.24974 | $151 | +0.045 |
| SEI | 123581 | 0.0376927 | 0.036418 | $-158 | +0.045 |
| PENDLE | -3236.78 | 1.54767 | 1.3693 | $577 | -0.044 |
| NEAR | -1371.79 | 3.51678 | 3.16525 | $482 | -0.043 |
| OP | 58569.1 | 0.0891484 | 0.07406 | $-884 | +0.043 |
| UNI | -836.22 | 5.16466 | 5.1491 | $13 | -0.043 |
| WLD | 15387.1 | 0.31305 | 0.25351 | $-916 | +0.039 |
| ENA | -33871.5 | 0.132226 | 0.110485 | $736 | -0.037 |
| TIA | 10070 | 0.345986 | 0.34942 | $35 | +0.035 |
| ZEC | -3.83105 | 862.252 | 868.815 | $-25 | -0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003687 | $-183 | +0.033 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.310915 | $-72 | +0.105 |
| BTC | -0.122056 | 65912.7 | 63168.5 | $335 | -0.079 |
| BNB | -9.13414 | 580.021 | 573.57 | $59 | -0.053 |
| ETH | -2.88908 | 1804.39 | 1784.15 | $58 | -0.053 |
| RENDER | 3386.49 | 1.53278 | 1.3833 | $-506 | +0.048 |
| FIL | -7861.93 | 0.620713 | 0.57482 | $361 | -0.046 |
| ATOM | 3503.24 | 1.28664 | 1.2644 | $-78 | +0.045 |
| INJ | 607.666 | 7.33527 | 6.86905 | $-283 | +0.043 |
| JUP | 15235.8 | 0.263176 | 0.24974 | $-205 | +0.039 |
| CRV | -19201.4 | 0.232521 | 0.19339 | $751 | -0.038 |
| AAVE | 32.3234 | 108.591 | 103.195 | $-174 | +0.034 |
| APT | -7937.59 | 0.43262 | 0.4026 | $238 | -0.033 |
| AVAX | 424.946 | 7.89216 | 7.0494 | $-358 | +0.031 |
| BCH | -14.0941 | 212.452 | 210.075 | $34 | -0.030 |
| LTC | -67.4945 | 40.3115 | 43.137 | $-191 | -0.030 |
| NEAR | 890.083 | 3.11366 | 3.16525 | $46 | +0.029 |
| DOGE | -44257.5 | 0.0643319 | 0.059094 | $232 | -0.027 |
| TAO | 12.9296 | 197.549 | 198.97 | $18 | +0.026 |
| ENA | -21996.7 | 0.108918 | 0.110485 | $-34 | -0.025 |
| HBAR | -38109.1 | 0.0598269 | 0.055148 | $178 | -0.021 |
| TIA | 5612.94 | 0.336231 | 0.34942 | $74 | +0.020 |
| SOL | -21.0355 | 91.9832 | 76.6545 | $322 | -0.016 |
| WLD | -5872.05 | 0.34434 | 0.25351 | $533 | -0.015 |
| XRP | -1446.41 | 1.02442 | 0.99311 | $45 | -0.015 |
| XLM | 9051.42 | 0.140675 | 0.135685 | $-45 | +0.013 |
| ARB | 9111.65 | 0.152074 | 0.11937 | $-298 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.3693 | $72 | +0.010 |
| SEI | 23468.7 | 0.0381037 | 0.036418 | $-40 | +0.009 |
| DOT | -1201.93 | 0.841601 | 0.71062 | $157 | -0.009 |
| kPEPE | 173237 | 0.00484647 | 0.003687 | $-201 | +0.007 |
| OP | -6924.47 | 0.0705215 | 0.07406 | $-25 | -0.005 |
| UNI | 96.3288 | 7.08573 | 5.1491 | $-187 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.25343 | $-36 | -0.005 |
| ZEC | 0.47487 | 796.75 | 868.815 | $34 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.30584 | $-57 | +0.002 |

**xyz_weekend_fade**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| xyz:SOXL | -264.497 | 33.7478 | 42.8625 | $-2,411 | -0.116 |
| xyz:COST | 5.37885 | 1103.42 | 1078.55 | $-134 | +0.059 |
| xyz:NBIS | -56.3175 | 86.8826 | 101.545 | $-826 | -0.058 |
| xyz:SP500 | 0.655074 | 7002.81 | 7237.85 | $154 | +0.048 |
| xyz:NVDA | -27.6065 | 133.548 | 166.225 | $-902 | -0.047 |
| xyz:UNITREE | 42.2589 | 90.2361 | 108.415 | $768 | +0.047 |
| xyz:STRC | 43.9632 | 95.0508 | 97.7615 | $119 | +0.044 |
| xyz:ANT | 46.3349 | 89.5172 | 88.5255 | $-46 | +0.042 |
| xyz:AAPL | 11.2936 | 348.333 | 348.545 | $2 | +0.040 |
| xyz:META | 6.33532 | 571.237 | 589.275 | $114 | +0.038 |
| xyz:XYZ100 | 0.138214 | 25535.2 | 26739.5 | $166 | +0.038 |
| xyz:IBM | 15.4686 | 227.775 | 233.055 | $82 | +0.037 |
| xyz:MSTR | -40.5579 | 82.455 | 88.16 | $-231 | -0.036 |
| xyz:CBRS | -32.3279 | 86.9154 | 99.4625 | $-406 | -0.033 |
| xyz:SKHY | -23.7567 | 109.14 | 124.5 | $-365 | -0.030 |
| xyz:AMZN | 11.2909 | 252.591 | 255.345 | $31 | +0.029 |
| xyz:SPCX | 20.186 | 130.902 | 139.835 | $180 | +0.029 |
| xyz:MSFT | 5.37955 | 495.48 | 504.245 | $47 | +0.028 |
| xyz:CRCL | -51.3591 | 50.1159 | 52.458 | $-120 | -0.027 |
| xyz:BABA | 30.2312 | 83.0533 | 86.528 | $105 | +0.027 |
| xyz:IREN | -166.006 | 12.8106 | 15.2755 | $-409 | -0.026 |
| xyz:SKHX | -2.87179 | 779.493 | 877.82 | $-282 | -0.026 |
| xyz:ZHIPU | -45.2724 | 38.8205 | 46.581 | $-351 | -0.022 |
| xyz:PURRDAT | -209.653 | 7.87569 | 8.72445 | $-178 | -0.019 |
| xyz:SNDK | -18.9852 | 83.0642 | 91.97 | $-169 | -0.018 |
| xyz:HOOD | -24.3395 | 65.1215 | 68.614 | $-85 | -0.017 |
| xyz:GOOGL | 5.77104 | 273.323 | 281.455 | $47 | +0.017 |
| xyz:INTC | -19.5296 | 53.6475 | 58.591 | $-97 | -0.012 |
| xyz:AMD | -2.18996 | 442.79 | 447.995 | $-11 | -0.010 |
| xyz:TSLA | 2.4212 | 357.605 | 388.995 | $76 | +0.010 |
| xyz:MU | 1.06539 | 615.696 | 701.02 | $91 | +0.008 |
| xyz:MRVL | -4.33833 | 127.25 | 145.655 | $-80 | -0.006 |
| xyz:EWY | 4.12068 | 127.505 | 137.11 | $40 | +0.006 |
| xyz:DRAM | -13.9588 | 35.0429 | 39.7 | $-65 | -0.006 |
| xyz:CXMT | -83.1059 | 5.0613 | 5.66215 | $-50 | -0.005 |
| xyz:ORCL | -3.21642 | 104.62 | 114.69 | $-32 | -0.004 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 2.78, "BTC": 2.38, "ETC": 2.91, "JUP": 2.86, "TRX": 1.34}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BNB": 2.78, "BTC": 2.38, "ETC": 2.91, "JUP": 2.86, "TRX": 1.34}}`
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
