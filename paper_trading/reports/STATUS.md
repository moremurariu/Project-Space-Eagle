# Paper trading status — 2026-11-23 23:04 UTC

BTC 61,404 · ETH 1,716.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $79,175 | -20.83% | +0.38% | -5.86% | -30.99% | -5.88 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,835 | -3.17% | +0.02% | -0.31% | -4.87% | -5.20 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.09 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,113 | +0.11% | -0.02% | +0.07% | -0.26% | 2.14 | 0.87 | +0.00 | 6 | $302 | $404 |
| trend_breakout_majors | $116,549 | +16.55% | -0.40% | +2.16% | -7.51% | 6.16 | 0.54 | -0.54 | 3 | $128 | $183 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.93 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.07 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.80 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,613 | +0.61% | +0.32% | +0.19% | -1.64% | 1.21 | 0.94 | +0.00 | 20 | $663 | $307 |
| xs_momentum | $98,530 | -1.47% | +0.24% | +0.00% | -3.13% | -2.27 | 0.93 | -0.01 | 35 | $243 | $53 |
| xyz_weekend_fade | $98,477 | -1.52% | -4.54% | -4.66% | -5.64% | -0.95 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,111,225 on $1,100,000 → +1.02%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.95 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 61413.9 | $-20,755 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 61404.5 | $-1,097 | +0.049 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.306 | $-4,897 | +0.153 |
| PENDLE | -11765.6 | 1.72106 | 1.3042 | $4,905 | -0.153 |
| HYPE | -233.128 | 82.7772 | 63.8825 | $4,405 | -0.149 |
| HYPE/spot | 233.128 | 83.2675 | 63.8405 | $-4,529 | +0.149 |
| ZEC/spot | 17.43 | 1175.27 | 775.988 | $-6,959 | +0.135 |
| ZEC | -17.4301 | 1175 | 775.775 | $6,959 | -0.135 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 61404.5 | $4,170 | -0.210 |
| ETH | -11.3433 | 2117.81 | 1716.55 | $4,552 | -0.167 |
| SOL | -265.603 | 89.0415 | 72.075 | $4,506 | -0.164 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 61404.5 | $1,000 | -0.088 |
| TRX | 28470.1 | 0.309116 | 0.309365 | $7 | +0.088 |
| LINK | -662.607 | 8.53769 | 8.98835 | $-299 | -0.059 |
| ADA | 39260.6 | 0.144249 | 0.147375 | $123 | +0.058 |
| BCH | 27.9335 | 199.151 | 199.85 | $20 | +0.055 |
| APT | 12637.8 | 0.436568 | 0.3776 | $-745 | +0.047 |
| TAO | -25.0244 | 220.686 | 189.85 | $772 | -0.047 |
| RENDER | -3551.19 | 1.49617 | 1.30315 | $685 | -0.046 |
| AAVE | -44.8727 | 112.188 | 96.635 | $698 | -0.043 |
| JUP | 18056.4 | 0.241363 | 0.23478 | $-119 | +0.042 |
| PENDLE | -3236.78 | 1.54767 | 1.3042 | $788 | -0.042 |
| SEI | 123581 | 0.0376927 | 0.033718 | $-491 | +0.041 |
| OP | 58569.1 | 0.0891484 | 0.06949 | $-1,151 | +0.040 |
| NEAR | -1371.79 | 3.51678 | 2.96115 | $762 | -0.040 |
| UNI | -836.22 | 5.16466 | 4.8123 | $295 | -0.040 |
| WLD | 15387.1 | 0.31305 | 0.23609 | $-1,184 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.100145 | $1,087 | -0.034 |
| TIA | 10070 | 0.345986 | 0.33294 | $-131 | +0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003426 | $-416 | +0.030 |
| ZEC | -3.83105 | 862.252 | 775.775 | $331 | -0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.309365 | $-124 | +0.104 |
| BTC | -0.122056 | 65912.7 | 61404.5 | $550 | -0.076 |
| BNB | -9.13414 | 580.021 | 557.255 | $208 | -0.052 |
| ETH | -2.88908 | 1804.39 | 1716.55 | $254 | -0.050 |
| ATOM | 3503.24 | 1.28664 | 1.2723 | $-50 | +0.045 |
| RENDER | 3386.49 | 1.53278 | 1.30315 | $-778 | +0.045 |
| FIL | -7861.93 | 0.620713 | 0.54812 | $571 | -0.044 |
| INJ | 607.666 | 7.33527 | 6.26635 | $-650 | +0.039 |
| JUP | 15235.8 | 0.263176 | 0.23478 | $-433 | +0.036 |
| CRV | -19201.4 | 0.232521 | 0.17864 | $1,035 | -0.035 |
| AAVE | 32.3234 | 108.591 | 96.635 | $-386 | +0.032 |
| APT | -7937.59 | 0.43262 | 0.3776 | $437 | -0.030 |
| AVAX | 424.946 | 7.89216 | 6.7629 | $-480 | +0.029 |
| BCH | -14.0941 | 212.452 | 199.85 | $178 | -0.029 |
| LTC | -67.4945 | 40.3115 | 41.255 | $-64 | -0.028 |
| NEAR | 890.083 | 3.11366 | 2.96115 | $-136 | +0.027 |
| DOGE | -44257.5 | 0.0643319 | 0.056274 | $357 | -0.025 |
| TAO | 12.9296 | 197.549 | 189.85 | $-100 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.100145 | $193 | -0.022 |
| HBAR | -38109.1 | 0.0598269 | 0.052392 | $283 | -0.020 |
| TIA | 5612.94 | 0.336231 | 0.33294 | $-18 | +0.019 |
| SOL | -21.0355 | 91.9832 | 72.075 | $419 | -0.015 |
| WLD | -5872.05 | 0.34434 | 0.23609 | $636 | -0.014 |
| XRP | -1446.41 | 1.02442 | 0.940935 | $121 | -0.014 |
| XLM | 9051.42 | 0.140675 | 0.12983 | $-98 | +0.012 |
| ARB | 9111.65 | 0.152074 | 0.11122 | $-372 | +0.010 |
| PENDLE | 752.441 | 1.27394 | 1.3042 | $23 | +0.010 |
| DOT | -1201.93 | 0.841601 | 0.67233 | $203 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.033718 | $-103 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003426 | $-246 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.06949 | $7 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.8123 | $-219 | +0.005 |
| LDO | -1790.74 | 0.233401 | 0.2373 | $-7 | -0.004 |
| ZEC | 0.47487 | 796.75 | 775.775 | $-10 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.287075 | $-70 | +0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -3.68, "ENA": -3.77, "INJ": -5.75, "SEI": -4.38, "ZEC": -5.33}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": -3.68, "ENA": -3.77, "INJ": -5.75, "SEI": -4.38, "ZEC": -5.33}}`
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
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SOXL | buy | $11,404 | 43.1142 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NBIS | buy | $5,534 | 98.2613 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NVDA | buy | $4,544 | 164.588 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:MSTR | buy | $3,438 | 84.756 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CBRS | buy | $3,199 | 98.9671 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHY | buy | $2,953 | 124.321 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CRCL | buy | $2,652 | 51.6426 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHX | buy | $2,516 | 876.234 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:IREN | buy | $2,450 | 14.7573 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:ZHIPU | buy | $2,071 | 45.7345 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:PURRDAT | buy | $1,737 | 8.28303 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SNDK | buy | $1,673 | 88.1224 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:HOOD | buy | $1,644 | 67.5282 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:INTC | buy | $1,164 | 59.6219 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:AMD | buy | $994 | 453.975 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:MRVL | buy | $627 | 144.51 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:DRAM | buy | $552 | 39.555 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CXMT | buy | $470 | 5.65143 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:ORCL | buy | $367 | 114.047 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:EWY | sell | $559 | 135.604 | weekend fade exit (Mon US close) |
