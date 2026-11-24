# Paper trading status — 2026-11-24 08:04 UTC

BTC 60,354 · ETH 1,696.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $77,854 | -22.15% | -1.81% | -6.62% | -30.99% | -6.24 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,753 | -3.25% | -0.09% | -0.35% | -4.87% | -5.29 | 0.05 | +0.05 | 1 | $53 | $-103 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.07 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,126 | +0.13% | +0.00% | +0.11% | -0.26% | 2.21 | 0.87 | +0.00 | 6 | $302 | $407 |
| trend_breakout_majors | $117,379 | +17.38% | +0.81% | +3.17% | -7.51% | 6.37 | 0.53 | -0.53 | 3 | $128 | $184 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.91 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.05 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.74 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,516 | +1.52% | +1.06% | +0.97% | -1.64% | 2.50 | 0.96 | +0.03 | 20 | $683 | $316 |
| xs_momentum | $98,734 | -1.27% | +0.52% | -0.29% | -3.13% | -1.90 | 0.85 | +0.00 | 35 | $247 | $53 |
| xyz_weekend_fade | $98,477 | -1.52% | -4.53% | -4.66% | -5.64% | -0.94 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,113,094 on $1,100,000 → +1.19%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.94 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60391 | $-22,076 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60354.5 | $-1,178 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.30018 | $-4,965 | +0.153 |
| PENDLE | -11765.6 | 1.72106 | 1.2991 | $4,965 | -0.153 |
| HYPE/spot | 233.128 | 83.2675 | 64.0022 | $-4,491 | +0.149 |
| HYPE | -233.128 | 82.7772 | 63.9885 | $4,380 | -0.149 |
| ZEC/spot | 17.43 | 1175.27 | 755.66 | $-7,314 | +0.132 |
| ZEC | -17.4301 | 1175 | 755.16 | $7,318 | -0.131 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 60354.5 | $4,588 | -0.205 |
| ETH | -11.3433 | 2117.81 | 1696.45 | $4,780 | -0.164 |
| SOL | -265.603 | 89.0415 | 71.3895 | $4,688 | -0.162 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.309345 | $7 | +0.087 |
| BTC | -0.144359 | 68333.1 | 60354.5 | $1,152 | -0.086 |
| ATOM | 4628.98 | 1.28373 | 1.3002 | $76 | +0.059 |
| LINK | -662.607 | 8.53769 | 9.0567 | $-344 | -0.059 |
| BCH | 27.9335 | 199.151 | 199.665 | $14 | +0.055 |
| SEI | 153686 | 0.0369478 | 0.034079 | $-441 | +0.052 |
| APT | 12637.8 | 0.436568 | 0.3836 | $-669 | +0.048 |
| TAO | -25.0244 | 220.686 | 188.92 | $795 | -0.047 |
| RENDER | -3551.19 | 1.49617 | 1.3074 | $670 | -0.046 |
| TIA | 13095.4 | 0.343495 | 0.34182 | $-22 | +0.044 |
| JUP | 18056.4 | 0.241363 | 0.239315 | $-37 | +0.043 |
| AAVE | -44.8727 | 112.188 | 95.4845 | $750 | -0.042 |
| PENDLE | -3236.78 | 1.54767 | 1.2991 | $805 | -0.041 |
| OP | 58569.1 | 0.0891484 | 0.07142 | $-1,038 | +0.041 |
| NEAR | -1371.79 | 3.51678 | 3.0016 | $707 | -0.041 |
| UNI | -836.22 | 5.16466 | 4.80865 | $298 | -0.040 |
| WLD | 15387.1 | 0.31305 | 0.24162 | $-1,099 | +0.037 |
| ENA | -33871.5 | 0.132226 | 0.100675 | $1,069 | -0.034 |
| kPEPE | 893923 | 0.0038915 | 0.00353 | $-323 | +0.031 |
| ZEC | -3.83105 | 862.252 | 755.16 | $410 | -0.028 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.309345 | $-124 | +0.104 |
| BTC | -0.122056 | 65912.7 | 60354.5 | $678 | -0.075 |
| BNB | -9.13414 | 580.021 | 551.22 | $263 | -0.051 |
| ATOM | 3503.24 | 1.28664 | 1.3002 | $47 | +0.046 |
| RENDER | 3386.49 | 1.53278 | 1.3074 | $-763 | +0.045 |
| FIL | -7861.93 | 0.620713 | 0.56312 | $453 | -0.045 |
| JUP | 15235.8 | 0.263176 | 0.239315 | $-364 | +0.037 |
| ETH | -2.08109 | 1804.39 | 1696.45 | $225 | -0.036 |
| CRV | -19201.4 | 0.232521 | 0.17952 | $1,018 | -0.035 |
| APT | -7937.59 | 0.43262 | 0.3836 | $389 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.7185 | $-499 | +0.029 |
| BCH | -14.0941 | 212.452 | 199.665 | $180 | -0.029 |
| INJ | 450.546 | 7.33527 | 6.24205 | $-493 | +0.028 |
| LTC | -67.4945 | 40.3115 | 41.5395 | $-83 | -0.028 |
| NEAR | 890.083 | 3.11366 | 3.0016 | $-100 | +0.027 |
| TAO | 12.9296 | 197.549 | 188.92 | $-112 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.100675 | $181 | -0.022 |
| AAVE | 21.5968 | 108.591 | 95.4845 | $-283 | +0.021 |
| HBAR | -38109.1 | 0.0598269 | 0.052042 | $297 | -0.020 |
| TIA | 5612.94 | 0.336231 | 0.34182 | $31 | +0.019 |
| WLD | -5872.05 | 0.34434 | 0.24162 | $603 | -0.014 |
| DOGE | -21725 | 0.0643319 | 0.056118 | $178 | -0.012 |
| XLM | 9051.42 | 0.140675 | 0.129325 | $-103 | +0.012 |
| PENDLE | 752.441 | 1.27394 | 1.2991 | $19 | +0.010 |
| DOT | -1201.93 | 0.841601 | 0.68079 | $193 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.034079 | $-94 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.00353 | $-228 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07142 | $-6 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.80865 | $-219 | +0.005 |
| SOL | -6.37858 | 91.9832 | 71.3895 | $131 | -0.005 |
| LDO | -1790.74 | 0.233401 | 0.23979 | $-11 | -0.004 |
| ARB | -3323.04 | 0.1124 | 0.11463 | $-7 | -0.004 |
| ZEC | 0.47487 | 796.75 | 755.16 | $-20 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.287005 | $-70 | +0.002 |
| XRP | -174.541 | 1.02442 | 0.942615 | $14 | -0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -1.96, "ALGO": -0.99, "BCH": -0.7, "PENDLE": -0.99, "XLM": -0.69}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"AAVE": -1.96, "ALGO": -0.99, "BCH": -0.7, "PENDLE": -0.99, "XLM": -0.69}}`
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
