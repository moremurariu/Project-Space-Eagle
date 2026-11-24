# Paper trading status — 2026-11-24 04:04 UTC

BTC 60,286 · ETH 1,695.0 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $77,760 | -22.24% | -1.60% | -7.19% | -30.99% | -6.29 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,748 | -3.25% | -0.08% | -0.38% | -4.87% | -5.32 | 0.05 | +0.05 | 1 | $53 | $-102 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.46 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.08 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,134 | +0.13% | +0.01% | +0.12% | -0.26% | 2.28 | 0.87 | +0.00 | 6 | $302 | $406 |
| trend_breakout_majors | $117,420 | +17.42% | +0.69% | +3.26% | -7.51% | 6.41 | 0.53 | -0.53 | 3 | $128 | $182 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.92 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.06 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.76 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $101,079 | +1.08% | +0.85% | +0.67% | -1.64% | 1.88 | 0.96 | +0.03 | 20 | $683 | $313 |
| xs_momentum | $98,743 | -1.26% | +0.41% | -0.08% | -3.13% | -1.89 | 0.85 | +0.00 | 35 | $247 | $54 |
| xyz_weekend_fade | $98,477 | -1.52% | -4.39% | -4.66% | -5.64% | -0.94 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,112,709 on $1,100,000 → +1.16%**
Combined BTC exposure (7d realised, 4h returns): beta -0.086 (≈ -0.95 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60317.9 | $-22,170 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60285.5 | $-1,183 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| PENDLE/spot | 11756.4 | 1.72251 | 1.30989 | $-4,851 | +0.154 |
| PENDLE | -11765.6 | 1.72106 | 1.3077 | $4,863 | -0.154 |
| HYPE/spot | 233.128 | 83.2675 | 63.2705 | $-4,662 | +0.147 |
| HYPE | -233.128 | 82.7772 | 63.2695 | $4,548 | -0.147 |
| ZEC/spot | 17.43 | 1175.27 | 758.487 | $-7,264 | +0.132 |
| ZEC | -17.4301 | 1175 | 758.015 | $7,268 | -0.132 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.398145 | 71878.7 | 60285.5 | $4,616 | -0.204 |
| ETH | -11.3433 | 2117.81 | 1695.05 | $4,795 | -0.164 |
| SOL | -265.603 | 89.0415 | 71.3925 | $4,688 | -0.161 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 28470.1 | 0.309116 | 0.309275 | $5 | +0.087 |
| BTC | -0.144359 | 68333.1 | 60285.5 | $1,162 | -0.086 |
| LINK | -662.607 | 8.53769 | 9.03185 | $-327 | -0.059 |
| ATOM | 4628.98 | 1.28373 | 1.2877 | $18 | +0.059 |
| BCH | 27.9335 | 199.151 | 201.065 | $53 | +0.056 |
| SEI | 153686 | 0.0369478 | 0.033737 | $-493 | +0.051 |
| APT | 12637.8 | 0.436568 | 0.3798 | $-717 | +0.047 |
| TAO | -25.0244 | 220.686 | 190.005 | $768 | -0.047 |
| RENDER | -3551.19 | 1.49617 | 1.28495 | $750 | -0.045 |
| TIA | 13095.4 | 0.343495 | 0.33556 | $-104 | +0.043 |
| AAVE | -44.8727 | 112.188 | 97.517 | $658 | -0.043 |
| JUP | 18056.4 | 0.241363 | 0.237565 | $-69 | +0.042 |
| PENDLE | -3236.78 | 1.54767 | 1.3077 | $777 | -0.042 |
| OP | 58569.1 | 0.0891484 | 0.07029 | $-1,105 | +0.041 |
| NEAR | -1371.79 | 3.51678 | 2.9794 | $737 | -0.040 |
| UNI | -836.22 | 5.16466 | 4.83015 | $280 | -0.040 |
| WLD | 15387.1 | 0.31305 | 0.23991 | $-1,125 | +0.037 |
| ENA | -33871.5 | 0.132226 | 0.10101 | $1,057 | -0.034 |
| kPEPE | 893923 | 0.0038915 | 0.003466 | $-380 | +0.031 |
| ZEC | -3.83105 | 862.252 | 758.015 | $399 | -0.029 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.309275 | $-127 | +0.104 |
| BTC | -0.122056 | 65912.7 | 60285.5 | $687 | -0.075 |
| BNB | -9.13414 | 580.021 | 552.075 | $255 | -0.051 |
| ATOM | 3503.24 | 1.28664 | 1.2877 | $4 | +0.046 |
| RENDER | 3386.49 | 1.53278 | 1.28495 | $-839 | +0.044 |
| FIL | -7861.93 | 0.620713 | 0.55283 | $534 | -0.044 |
| JUP | 15235.8 | 0.263176 | 0.237565 | $-390 | +0.037 |
| ETH | -2.08109 | 1804.39 | 1695.05 | $228 | -0.036 |
| CRV | -19201.4 | 0.232521 | 0.17828 | $1,041 | -0.035 |
| APT | -7937.59 | 0.43262 | 0.3798 | $419 | -0.031 |
| AVAX | 424.946 | 7.89216 | 6.7577 | $-482 | +0.029 |
| BCH | -14.0941 | 212.452 | 201.065 | $160 | -0.029 |
| INJ | 450.546 | 7.33527 | 6.20195 | $-511 | +0.028 |
| LTC | -67.4945 | 40.3115 | 41.308 | $-67 | -0.028 |
| NEAR | 890.083 | 3.11366 | 2.9794 | $-120 | +0.027 |
| TAO | 12.9296 | 197.549 | 190.005 | $-98 | +0.025 |
| ENA | -21996.7 | 0.108918 | 0.10101 | $174 | -0.023 |
| AAVE | 21.5968 | 108.591 | 97.517 | $-239 | +0.021 |
| HBAR | -38109.1 | 0.0598269 | 0.052048 | $296 | -0.020 |
| TIA | 5612.94 | 0.336231 | 0.33556 | $-4 | +0.019 |
| WLD | -5872.05 | 0.34434 | 0.23991 | $613 | -0.014 |
| DOGE | -21725 | 0.0643319 | 0.056022 | $181 | -0.012 |
| XLM | 9051.42 | 0.140675 | 0.129905 | $-97 | +0.012 |
| PENDLE | 752.441 | 1.27394 | 1.3077 | $25 | +0.010 |
| DOT | -1201.93 | 0.841601 | 0.67401 | $201 | -0.008 |
| SEI | 23468.7 | 0.0381037 | 0.033737 | $-102 | +0.008 |
| kPEPE | 173237 | 0.00484647 | 0.003466 | $-239 | +0.006 |
| OP | -6924.47 | 0.0705215 | 0.07029 | $2 | -0.005 |
| UNI | 96.3288 | 7.08573 | 4.83015 | $-217 | +0.005 |
| SOL | -6.37858 | 91.9832 | 71.3925 | $131 | -0.005 |
| LDO | -1790.74 | 0.233401 | 0.2375 | $-7 | -0.004 |
| ARB | -3323.04 | 0.1124 | 0.11408 | $-6 | -0.004 |
| ZEC | 0.47487 | 796.75 | 758.015 | $-18 | +0.004 |
| ONDO | 657.524 | 0.392809 | 0.287245 | $-69 | +0.002 |
| XRP | -174.541 | 1.02442 | 0.948005 | $13 | -0.002 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BTC": -1.52, "ETC": -1.36, "INJ": -1.35, "RENDER": -1.53, "ZEC": -2.22}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"BTC": -1.52, "ETC": -1.36, "INJ": -1.35, "RENDER": -1.53, "ZEC": -2.22}}`
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
