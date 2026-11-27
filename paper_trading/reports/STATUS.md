# Paper trading status — 2026-11-27 00:04 UTC

BTC 60,650 · ETH 1,693.2 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $78,219 | -21.78% | +1.04% | -2.28% | -30.99% | -5.56 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $96,776 | -3.22% | +0.05% | -0.11% | -4.87% | -4.96 | 0.05 | +0.05 | 1 | $53 | $-103 |
| crash_rebound | $100,123 | +0.12% | +0.00% | +0.00% | -0.69% | 0.43 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 2.91 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,118 | +0.12% | +0.00% | +0.03% | -0.26% | 2.00 | 0.86 | -0.00 | 6 | $306 | $432 |
| trend_breakout_majors | $117,256 | +17.26% | -0.56% | +1.60% | -7.51% | 5.84 | 0.60 | -0.60 | 3 | $132 | $194 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -2.76 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -2.89 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | -4.39 | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $102,327 | +2.33% | +1.47% | +1.38% | -1.64% | 3.29 | 0.98 | -0.07 | 20 | $717 | $366 |
| xs_momentum | $97,944 | -2.06% | -0.05% | -0.92% | -3.47% | -2.93 | 0.83 | +0.02 | 39 | $273 | $67 |
| xyz_weekend_fade | $98,477 | -1.52% | +0.00% | -4.66% | -5.64% | -0.89 | 0.00 | +0.00 | 0 | $1,025 | $134 |

**Combined (ex-benchmark): $1,113,006 on $1,100,000 → +1.18%**
Combined BTC exposure (7d realised, 4h returns): beta -0.087 (≈ -0.96 books of BTC), corr -0.85

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 60674 | $-21,711 | +1.002 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 60649.5 | $-1,155 | +0.048 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| HYPE | -233.128 | 82.7772 | 62.5855 | $4,707 | -0.146 |
| HYPE/spot | 233.128 | 83.2675 | 62.5364 | $-4,833 | +0.146 |
| PENDLE/spot | 11756.4 | 1.72251 | 1.2117 | $-6,005 | +0.142 |
| PENDLE | -11765.6 | 1.72106 | 1.21055 | $6,006 | -0.142 |
| ZEC | -20.3886 | 1111.39 | 688.8 | $8,616 | -0.140 |
| ZEC/spot | 20.3817 | 1111.83 | 688.428 | $-8,630 | +0.140 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.448882 | 70508.2 | 60649.5 | $4,425 | -0.232 |
| ETH | -12.8885 | 2066.89 | 1693.15 | $4,817 | -0.186 |
| SOL | -303.429 | 86.6295 | 71.1885 | $4,685 | -0.184 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 60649.5 | $1,109 | -0.086 |
| ETH | -4.08866 | 1693.07 | 1693.15 | $-0 | -0.068 |
| LINK | -662.607 | 8.53769 | 9.30155 | $-506 | -0.060 |
| BCH | 27.9335 | 199.151 | 210.615 | $320 | +0.057 |
| ADA | 39526.3 | 0.145586 | 0.14875 | $125 | +0.057 |
| DOT | 7976.14 | 0.67508 | 0.67496 | $-1 | +0.053 |
| AAVE | -56.3044 | 108.666 | 94.8455 | $778 | -0.052 |
| PENDLE | -4098.66 | 1.48275 | 1.21055 | $1,116 | -0.048 |
| SEI | 127166 | 0.0369478 | 0.038683 | $221 | +0.048 |
| APT | 12637.8 | 0.436568 | 0.3851 | $-650 | +0.048 |
| LDO | -20605.5 | 0.228435 | 0.23523 | $-140 | -0.047 |
| RENDER | -3551.19 | 1.49617 | 1.33225 | $582 | -0.046 |
| TIA | 13095.4 | 0.343495 | 0.33693 | $-86 | +0.043 |
| JUP | 18056.4 | 0.241363 | 0.23813 | $-58 | +0.042 |
| NEAR | -1371.79 | 3.51678 | 3.06045 | $626 | -0.041 |
| OP | 58569.1 | 0.0891484 | 0.06968 | $-1,140 | +0.040 |
| UNI | -836.22 | 5.16466 | 4.6697 | $414 | -0.038 |
| WLD | 15387.1 | 0.31305 | 0.24194 | $-1,094 | +0.036 |
| ZEC | -5.40144 | 811.813 | 688.8 | $664 | -0.036 |
| kPEPE | 893923 | 0.0038915 | 0.00349 | $-359 | +0.030 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 33280.4 | 0.313083 | 0.303905 | $-305 | +0.103 |
| BTC | -0.102473 | 65912.7 | 60649.5 | $539 | -0.063 |
| CRV | -25397.7 | 0.217677 | 0.1745 | $1,097 | -0.045 |
| ATOM | 3503.24 | 1.28664 | 1.2194 | $-236 | +0.044 |
| TAO | 20.8813 | 191.168 | 184.29 | $-144 | +0.039 |
| BNB | -6.85319 | 580.021 | 555.315 | $169 | -0.039 |
| FIL | -5804.68 | 0.620713 | 0.5405 | $466 | -0.032 |
| RENDER | 2307.71 | 1.53278 | 1.33225 | $-463 | +0.031 |
| APT | -7937.59 | 0.43262 | 0.3851 | $377 | -0.031 |
| ALGO | 34845.6 | 0.0802361 | 0.082285 | $71 | +0.029 |
| NEAR | 890.083 | 3.11366 | 3.06045 | $-47 | +0.028 |
| ETH | -1.41038 | 1804.39 | 1693.15 | $157 | -0.024 |
| JUP | 10022.4 | 0.263176 | 0.23813 | $-251 | +0.024 |
| HBAR | -43327 | 0.0558795 | 0.054168 | $74 | -0.024 |
| ENA | -21996.7 | 0.108918 | 0.100755 | $180 | -0.023 |
| ARB | -17977.9 | 0.113056 | 0.11376 | $-13 | -0.021 |
| TIA | 5612.94 | 0.336231 | 0.33693 | $4 | +0.019 |
| AVAX | 270.178 | 7.89216 | 6.6031 | $-348 | +0.018 |
| ETC | 298.103 | 5.74918 | 5.922 | $52 | +0.018 |
| LTC | -39.2061 | 40.3115 | 43.851 | $-139 | -0.018 |
| LDO | -6736.27 | 0.229782 | 0.23523 | $-37 | -0.016 |
| WLD | -5872.05 | 0.34434 | 0.24194 | $601 | -0.015 |
| LINK | 138.354 | 8.85022 | 9.30155 | $62 | +0.013 |
| XRP | 1350.36 | 0.937042 | 0.945985 | $12 | +0.013 |
| DOGE | -21725 | 0.0643319 | 0.056592 | $168 | -0.013 |
| INJ | 195.192 | 7.33527 | 6.23635 | $-215 | +0.012 |
| ADA | 7625.04 | 0.148758 | 0.14875 | $-0 | +0.012 |
| DOT | -1201.93 | 0.841601 | 0.67496 | $200 | -0.008 |
| PENDLE | -615.592 | 1.20948 | 1.21055 | $-1 | -0.008 |
| SOL | 10.4549 | 71.1921 | 71.1885 | $-0 | +0.008 |
| ZEC | -0.965052 | 688.749 | 688.8 | $-0 | -0.007 |
| kPEPE | -163259 | 0.0034895 | 0.00349 | $-0 | -0.006 |
| SEI | -14698.5 | 0.0328373 | 0.038683 | $-86 | -0.006 |
| OP | -6924.47 | 0.0705215 | 0.06968 | $6 | -0.005 |
| BCH | 2.1652 | 210.626 | 210.615 | $-0 | +0.005 |
| UNI | 96.3288 | 7.08573 | 4.6697 | $-233 | +0.005 |
| AAVE | 3.48168 | 108.591 | 94.8455 | $-48 | +0.003 |
| ONDO | 657.524 | 0.392809 | 0.287955 | $-69 | +0.002 |
| XLM | -126.553 | 0.128021 | 0.126795 | $0 | -0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-23 07:10 (due today): hold token BTC 0.05. BTC 61.5k, rebounding from the 11-20 low of ~56k (+4%/24h), but still far below the 72k reclaim level. BTC funding is back to +11% APR (longs paying again after the negative-funding flush), F&G 10-12 for 2 weeks (r64: extreme fear has marked continuation, not bottoms, in 2024+). Mixed macro: Ackman warns of a December Fed 'mistake', gold +2.4%, SP500 +1%. Venue/crypto risk headlines: Kraken flash print to $17K, Chainflip $12M hack. No edge to add. The systematic books are net short (beta -0.07), so keep the small long as `
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"CRV": -2.68, "FIL": -1.64, "HBAR": -1.64, "SEI": -3.18, "ZEC": -4.87}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"CRV": -2.68, "FIL": -1.64, "HBAR": -1.64, "SEI": -3.18, "ZEC": -4.87}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 8.7, "PENDLE": 7.5, "ZEC": 9.3}, "last_day": "2026-11-27", "top_apr": {"DOGE": 3.2, "ENA": 7.4, "ETC": 10.3, "HYPE": 8.7, "LDO": 6.7, "LINK": 4.6, "NEAR": 2.8, "PENDLE": 7.5, "RENDER": 7.5, "ZEC": 9.3}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-26T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-26T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-27"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-26T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-22 23:04: 36 trades, 38 names, weekend xs-dispersion 2.05%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-27T00:04 | xs_momentum | LTC | buy | $1,241 | 43.8548 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | ALGO | buy | $1,232 | 0.0823132 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | SOL | buy | $1,198 | 71.1921 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | BTC | buy | $1,188 | 60652.5 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | HBAR | buy | $1,187 | 0.0541809 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | ADA | buy | $1,134 | 0.148758 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | BCH | buy | $995 | 210.626 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | ZEC | sell | $992 | 688.749 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | kPEPE | sell | $1,174 | 0.0034895 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | JUP | sell | $1,241 | 0.238095 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | INJ | sell | $1,592 | 6.23509 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_momentum | PENDLE | sell | $1,655 | 1.20948 | signal 2026-11-26 23h |
| 2026-11-27T00:04 | xs_funding | DOT | buy | $5,385 | 0.67508 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | TAO | buy | $4,613 | 184.344 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | SEI | sell | $1,026 | 0.0386779 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | ZEC | sell | $1,082 | 688.766 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | AAVE | sell | $1,084 | 94.8408 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | ATOM | sell | $5,641 | 1.2186 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | xs_funding | ETH | sell | $6,922 | 1693.07 | daily xs-funding (Binance recon) |
| 2026-11-27T00:04 | trend_breakout_majors | ETH | sell | $2,616 | 1693.07 | signal 2026-11-26 23h |
