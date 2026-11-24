# Paper trading status — 2026-11-24 19:04 UTC

BTC 59,778 · ETH 1,675.7 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $77,111 | -22.89% | -3.04% | -6.23% | -30.99% | -6.34 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,709 | -3.29% | -0.15% | -0.32% | -4.87% | -5.31 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.45 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.04 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,138 | +0.14% | +0.01% | +0.10% | -0.26% | 2.27 | 0.83 | +0.00 | 6 | $302 | $411 |
| trend_breakout_majors | $118,427 | +18.43% | +2.36% | +3.30% | -7.51% | 6.61 | 0.52 | -0.52 | 3 | $128 | $181 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.88 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.02 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.67 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,261 | +1.26% | +0.66% | +0.81% | -1.64% | 2.09 | 0.93 | +0.03 | 20 | $683 | $327 |
| xs_momentum | $98,318 | -1.68% | -0.29% | -0.50% | -3.13% | -2.53 | 0.83 | -0.00 | 35 | $247 | $55 |
| xyz_weekend_fade | $98,477 | -1.52% | -0.22% | -4.66% | -5.64% | -0.93 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,113,438 on $1,100,000 → +1.22%**
Combined BTC exposure (7d realised, 4h returns): beta -0.085 (≈ -0.94 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 59815.3 | $-22,819 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 59777.5 | $-1,222 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.2485 | $-5,573 | +0.147 |
| PENDLE | -11765.6 | 1.72106 | 1.2468 | $5,580 | -0.146 |
| HYPE/spot | 233.128 | 83.2675 | 60.4665 | $-5,316 | +0.141 |
| HYPE | -233.128 | 82.7772 | 60.4115 | $5,214 | -0.141 |
| ZEC | -17.4301 | 1175 | 720.375 | $7,924 | -0.125 |
| ZEC/spot | 17.43 | 1175.27 | 720.36 | $-7,929 | +0.125 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 59777.5 | $4,818 | -0.201 |
| ETH | -11.3433 | 2117.81 | 1675.65 | $5,016 | -0.160 |
| SOL | -265.603 | 89.0415 | 69.1875 | $5,273 | -0.155 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.308095 | $-29 | +0.087 |
| BTC | -0.144359 | 68333.1 | 59777.5 | $1,235 | -0.085 |
| LINK | -662.607 | 8.53769 | 8.8577 | $-212 | -0.058 |
| ATOM | 4628.98 | 1.28373 | 1.2511 | $-151 | +0.057 |
| BCH | 27.9335 | 199.151 | 194.75 | $-123 | +0.054 |
| SEI | 153686 | 0.0369478 | 0.033322 | $-557 | +0.051 |
| APT | 12637.8 | 0.436568 | 0.3703 | $-837 | +0.046 |
| RENDER | -3551.19 | 1.49617 | 1.2862 | $746 | -0.045 |
| TAO | -25.0244 | 220.686 | 181.055 | $992 | -0.045 |
| TIA | 13095.4 | 0.343495 | 0.32646 | $-223 | +0.042 |
| AAVE | -44.8727 | 112.188 | 93.475 | $840 | -0.041 |
| OP | 58569.1 | 0.0891484 | 0.06892 | $-1,185 | +0.040 |
| PENDLE | -3236.78 | 1.54767 | 1.2468 | $974 | -0.040 |
| JUP | 18056.4 | 0.241363 | 0.223305 | $-326 | +0.040 |
| UNI | -836.22 | 5.16466 | 4.71655 | $375 | -0.039 |
| NEAR | -1371.79 | 3.51678 | 2.84795 | $917 | -0.039 |
| WLD | 15387.1 | 0.31305 | 0.234 | $-1,216 | +0.036 |
| ENA | -33871.5 | 0.132226 | 0.097603 | $1,173 | -0.033 |
| kPEPE | 893923 | 0.0038915 | 0.003419 | $-422 | +0.030 |
| ZEC | -3.83105 | 862.252 | 720.375 | $544 | -0.027 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.308095 | $-166 | +0.104 |
| BTC | -0.122056 | 65912.7 | 59777.5 | $749 | -0.074 |
| BNB | -9.13414 | 580.021 | 548.84 | $285 | -0.051 |
| ATOM | 3503.24 | 1.28664 | 1.2511 | $-125 | +0.045 |
| RENDER | 3386.49 | 1.53278 | 1.2862 | $-835 | +0.044 |
| FIL | -7861.93 | 0.620713 | 0.53947 | $639 | -0.043 |
| ETH | -2.08109 | 1804.39 | 1675.65 | $268 | -0.035 |
| JUP | 15235.8 | 0.263176 | 0.223305 | $-607 | +0.035 |
| CRV | -19201.4 | 0.232521 | 0.17622 | $1,081 | -0.034 |
| APT | -7937.59 | 0.43262 | 0.3703 | $495 | -0.030 |
| AVAX | 424.946 | 7.89216 | 6.5423 | $-574 | +0.028 |
| LTC | -67.4945 | 40.3115 | 40.8125 | $-34 | -0.028 |
| BCH | -14.0941 | 212.452 | 194.75 | $249 | -0.028 |
| INJ | 450.546 | 7.33527 | 5.78675 | $-698 | +0.027 |
| NEAR | 890.083 | 3.11366 | 2.84795 | $-237 | +0.026 |
| TAO | 12.9296 | 197.549 | 181.055 | $-213 | +0.024 |
| ENA | -21996.7 | 0.108918 | 0.097603 | $249 | -0.022 |
| AAVE | 21.5968 | 108.591 | 93.475 | $-326 | +0.021 |
| HBAR | -38109.1 | 0.0598269 | 0.050015 | $374 | -0.019 |
| TIA | 5612.94 | 0.336231 | 0.32646 | $-55 | +0.019 |
| WLD | -5872.05 | 0.34434 | 0.234 | $648 | -0.014 |
| DOGE | -21725 | 0.0643319 | 0.055127 | $200 | -0.012 |
| XLM | 9051.42 | 0.140675 | 0.124 | $-151 | +0.011 |
| PENDLE | 752.441 | 1.27394 | 1.2468 | $-20 | +0.010 |
| DOT | -1201.93 | 0.841601 | 0.65357 | $226 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.033322 | $-112 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003419 | $-247 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.06892 | $11 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.71655 | $-228 | +0.005 |
| SOL | -6.37858 | 91.9832 | 69.1875 | $145 | -0.004 |
| LDO | -1790.74 | 0.233401 | 0.23141 | $4 | -0.004 |
| ARB | -3323.04 | 0.1124 | 0.11108 | $4 | -0.004 |
| ZEC | 0.47487 | 796.75 | 720.375 | $-36 | +0.003 |
| ONDO | 657.524 | 0.392809 | 0.27858 | $-75 | +0.002 |
| XRP | -174.541 | 1.02442 | 0.918735 | $18 | -0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": -1.86, "HBAR": -1.86, "INJ": -2.91, "PENDLE": -1.96, "TIA": -1.86}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BCH": -1.86, "HBAR": -1.86, "INJ": -2.91, "PENDLE": -1.96, "TIA": -1.86}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.4, "PENDLE": 9.6, "ZEC": 10.4}, "last_day": "2026-11-24", "top_apr": {"BTC": 5.2, "DOGE": 4.1, "ENA": 9.0, "ETC": 7.3, "HYPE": 8.4, "LDO": 8.3, "LINK": 6.3, "PENDLE": 9.6, "RENDER": 8.4, "ZEC": 10.4}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-24"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-23T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-22 23:04: 36 trades, 38 names, weekend xs-dispersion 2.05%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-24T00:04 | xs_momentum | ETH | buy | $1,378 | 1705.04 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | DOGE | buy | $1,267 | 0.0562481 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | XRP | buy | $1,193 | 0.938082 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | SOL | buy | $1,053 | 71.8201 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | INJ | sell | $988 | 6.29079 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | AAVE | sell | $1,038 | 96.7782 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_momentum | ARB | sell | $1,398 | 0.1124 | signal 2026-11-23 23h |
| 2026-11-24T00:04 | xs_funding | ATOM | buy | $5,942 | 1.28373 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | SEI | buy | $1,020 | 0.0338899 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | TIA | buy | $1,014 | 0.335205 | daily xs-funding (Binance recon) |
| 2026-11-24T00:04 | xs_funding | ADA | sell | $5,816 | 0.148126 | daily xs-funding (Binance recon) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SOXL | buy | $11,404 | 43.1142 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NBIS | buy | $5,534 | 98.2613 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:NVDA | buy | $4,544 | 164.588 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:MSTR | buy | $3,438 | 84.756 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CBRS | buy | $3,199 | 98.9671 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHY | buy | $2,953 | 124.321 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:CRCL | buy | $2,652 | 51.6426 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:SKHX | buy | $2,516 | 876.234 | weekend fade exit (Mon US close) |
| 2026-11-23T21:04 | xyz_weekend_fade | xyz:IREN | buy | $2,450 | 14.7573 | weekend fade exit (Mon US close) |
