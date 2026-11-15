# Paper trading status — 2026-11-15 06:04 UTC

BTC 64,274 · ETH 1,793.5 · venue: Hyperliquid (paper) · capital per book $100k

| Book | Equity | Total | 24h | 7d | Max DD | Sharpe (ann., ≥14d) | Gross | Net | Pos | Fees | Funding |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| benchmark_btc_hold | $82,905 | -17.10% | -4.08% | -15.19% | -22.40% | -9.18 | 1.00 | +1.00 | 1 | $120 | $0 |
| claude_discretionary | $97,062 | -2.94% | -0.18% | -1.54% | -4.33% | -6.25 | 0.05 | +0.05 | 1 | $53 | $-96 |
| crash_rebound | $100,123 | +0.12% | +0.00% | -0.13% | -0.69% | 0.58 | 0.00 | +0.00 | 0 | $28 | $-7 |
| crash_rebound_mkt | $100,249 | +0.25% | +0.00% | +0.00% | -0.20% | 3.91 | 0.00 | +0.00 | 0 | $11 | $-2 |
| dn_funding_carry | $100,119 | +0.12% | -0.06% | +0.02% | -0.19% | 3.09 | 0.86 | -0.02 | 6 | $293 | $303 |
| trend_breakout_majors | $115,026 | +15.03% | +3.79% | +15.29% | -5.48% | 9.26 | 0.54 | -0.54 | 3 | $115 | $163 |
| trend_breakout_universe_lo | $99,917 | -0.08% | +0.00% | +0.00% | -0.18% | -3.71 | 0.00 | +0.00 | 0 | $3 | $-1 |
| trend_ewmac_majors_lo | $99,979 | -0.02% | +0.00% | +0.00% | -0.05% | -3.88 | 0.00 | +0.00 | 0 | $13 | $-1 |
| xs_bab | $99,840 | -0.16% | +0.00% | +0.00% | -0.16% | nan | 0.00 | +0.00 | 0 | $130 | $-1 |
| xs_funding | $100,973 | +0.97% | +0.70% | +1.27% | -1.56% | 3.00 | 0.94 | +0.02 | 20 | $561 | $131 |
| xs_momentum | $99,439 | -0.56% | +0.40% | -1.08% | -2.98% | nan | 0.88 | -0.02 | 39 | $188 | $-8 |
| xyz_weekend_fade | $101,220 | +1.22% | +0.00% | +1.22% | -1.21% | nan | 0.00 | +0.00 | 0 | $541 | $3 |

**Combined (ex-benchmark): $1,113,948 on $1,100,000 → +1.27%**
Combined BTC exposure (7d realised, 4h returns): beta -0.078 (≈ -0.85 books of BTC), corr -0.87

## Positions

**benchmark_btc_hold**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC/spot | 1.29115 | 77488.8 | 64302.9 | $-17,025 | +1.001 |

**claude_discretionary**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | 0.0770008 | 75652.7 | 64274.5 | $-876 | +0.051 |

**dn_funding_carry**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| ZEC | -17.4301 | 1175 | 891.085 | $4,949 | -0.155 |
| ZEC/spot | 17.43 | 1175.27 | 890.96 | $-4,955 | +0.155 |
| HYPE | -233.128 | 82.7772 | 65.2865 | $4,078 | -0.152 |
| PENDLE | -9671.02 | 1.81759 | 1.4013 | $4,026 | -0.135 |
| PENDLE/spot | 9662.31 | 1.81881 | 1.402 | $-4,027 | +0.135 |
| HYPE/spot | 201.997 | 85.5551 | 65.337 | $-4,084 | +0.132 |

**trend_breakout_majors**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.356716 | 75630.7 | 64274.5 | $4,051 | -0.199 |
| SOL | -281.278 | 91.199 | 70.4145 | $5,846 | -0.172 |
| ETH | -10.8509 | 2187.58 | 1793.55 | $4,276 | -0.169 |

**xs_funding**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| BTC | -0.144359 | 68333.1 | 64274.5 | $586 | -0.092 |
| TRX | 28470.1 | 0.309116 | 0.30677 | $-67 | +0.086 |
| XRP | -7432.17 | 1.17255 | 0.956165 | $1,608 | -0.070 |
| XLM | 44454.5 | 0.151093 | 0.13731 | $-613 | +0.060 |
| ATOM | 4381.77 | 1.45283 | 1.2799 | $-758 | +0.056 |
| BCH | 23.4344 | 238.546 | 204.785 | $-791 | +0.048 |
| SEI | 123581 | 0.0376927 | 0.038345 | $81 | +0.047 |
| JUP | 18056.4 | 0.241363 | 0.261525 | $364 | +0.047 |
| TAO | -25.0244 | 220.686 | 188.05 | $817 | -0.047 |
| LINK | -516.66 | 11.3839 | 9.0832 | $1,189 | -0.046 |
| LDO | -14834.4 | 0.308439 | 0.27134 | $550 | -0.040 |
| RENDER | -2753.67 | 1.56167 | 1.3711 | $525 | -0.037 |
| ARB | 27302.4 | 0.151348 | 0.13179 | $-534 | +0.036 |
| OP | 44197.2 | 0.0926854 | 0.08132 | $-502 | +0.036 |
| PENDLE | -2505.4 | 1.58072 | 1.4013 | $450 | -0.035 |
| TIA | 10070 | 0.345986 | 0.34716 | $12 | +0.035 |
| ZEC | -3.83105 | 862.252 | 891.085 | $-110 | -0.034 |
| NEAR | -997.435 | 3.77006 | 3.11415 | $654 | -0.031 |
| WLD | 10593 | 0.336516 | 0.26019 | $-809 | +0.027 |
| ENA | -22442.6 | 0.146112 | 0.11793 | $632 | -0.026 |

**xs_momentum**

| Key | Qty | Avg px | Px | uPnL | Weight |
|---|---:|---:|---:|---:|---:|
| TRX | 25819.3 | 0.315091 | 0.30677 | $-215 | +0.080 |
| SOL | -87.2593 | 91.9832 | 70.4145 | $1,882 | -0.062 |
| XRP | -6239.14 | 1.02442 | 0.956165 | $426 | -0.060 |
| AVAX | 607.041 | 7.89216 | 7.2081 | $-415 | +0.044 |
| INJ | 607.666 | 7.33527 | 7.01545 | $-194 | +0.043 |
| ARB | 29979.9 | 0.152074 | 0.13179 | $-608 | +0.040 |
| PENDLE | -2522.74 | 1.58296 | 1.4013 | $458 | -0.036 |
| BTC | -0.0540197 | 68049 | 64274.5 | $204 | -0.035 |
| RENDER | 2461.25 | 1.58063 | 1.3711 | $-516 | +0.034 |
| DOT | -4637.48 | 0.841601 | 0.71605 | $582 | -0.033 |
| kPEPE | 803792 | 0.00484647 | 0.003928 | $-738 | +0.032 |
| UNI | 597.681 | 7.08573 | 4.9954 | $-1,249 | +0.030 |
| BCH | -14.0941 | 212.452 | 204.785 | $108 | -0.029 |
| JUP | 10388.9 | 0.261398 | 0.261525 | $1 | +0.027 |
| DOGE | -46908.3 | 0.0665749 | 0.057859 | $409 | -0.027 |
| ETH | -1.46269 | 1817.83 | 1793.55 | $36 | -0.026 |
| WLD | -9976.36 | 0.34434 | 0.26019 | $840 | -0.026 |
| ADA | -16388.2 | 0.190675 | 0.14939 | $677 | -0.025 |
| CRV | -11195.5 | 0.271234 | 0.20687 | $721 | -0.023 |
| OP | 24123.2 | 0.107142 | 0.08132 | $-623 | +0.020 |
| AAVE | 18.1796 | 131.483 | 105.185 | $-478 | +0.019 |
| ATOM | 1186.94 | 1.29824 | 1.2799 | $-22 | +0.015 |
| FIL | -2342.25 | 0.799282 | 0.61395 | $434 | -0.014 |
| SEI | -33311.6 | 0.0472943 | 0.038345 | $298 | -0.013 |
| XLM | -8726.83 | 0.145302 | 0.13731 | $70 | -0.012 |
| LDO | 4201.67 | 0.267639 | 0.27134 | $16 | +0.011 |
| ENA | -8394.07 | 0.115324 | 0.11793 | $-22 | -0.010 |
| HBAR | -16510.2 | 0.0706575 | 0.057267 | $221 | -0.010 |
| TIA | 2451.15 | 0.345826 | 0.34716 | $3 | +0.009 |
| LINK | 77.5637 | 11.0659 | 9.0832 | $-154 | +0.007 |
| APT | -1385.4 | 0.626089 | 0.4531 | $240 | -0.006 |
| ZEC | 0.623195 | 1213.4 | 891.085 | $-201 | +0.006 |
| BNB | 0.917396 | 705.088 | 591.155 | $-105 | +0.005 |
| NEAR | -155.553 | 3.72366 | 3.11415 | $95 | -0.005 |
| TAO | 2.54822 | 211.979 | 188.05 | $-61 | +0.005 |
| ONDO | 657.524 | 0.392809 | 0.299555 | $-61 | +0.002 |
| ALGO | 2092.72 | 0.0922958 | 0.078546 | $-29 | +0.002 |
| LTC | -3.23093 | 54.9478 | 44.48 | $34 | -0.001 |
| SUI | 66.7499 | 0.886044 | 0.71663 | $-11 | +0.000 |

## Strategy notes

- **benchmark_btc_hold** — Benchmark: 100% BTC spot buy & hold (priced at HL oracle).
- **claude_discretionary** — Claude's discretionary book: targets set by hand in state/discretionary.json after reviewing news, funding, positioning and charts. Measures whether judgment adds value.
  - `{"rationale": "Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% since the 11-12 cut at 72.9k); alts -10 to -17%/24h. Panic signs: F&G 10 (lowest reading of this decline), BTC 25d RR -11.5 (was -4 on 11-13), ATM IV 37 -> 49, ETH/SOL/SUI/kPEPE funding negative (crowded shorts), BTC ETF outflows 11 straight sessions. r64: extreme fear has not marked bottoms OOS (2024+), so no add on fear alone; the systematic books are net short (beta -0.07) and the token long is a small hedge against a short squeeze. Rules: BTC reclaims 72k (t`
- **crash_rebound** — Buy coins after a >10% drop over 4h measured on closed hourly bars (liquidation-cascade overshoot). Scale-in (r30): tranche 1 = 7.5% at the signal close, tranche 2 = 7.5% if an hourly close within the next 12h is a further 5% below the signal close; each tranche held 24h (re-trigger extends), max gross 100%. Hourly only: r28 showed 5-minute scanning catches falling knives (MaxDD −60% vs −28%).
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": 3.01, "BTC": 3.02, "DOGE": 3.42, "HBAR": 3.35, "TRX": 0.59}}`
- **crash_rebound_mkt** — Crash rebound, market-wide cascades only: coin down >10% over 4h AND BTC down >3% over the same 4h (forced-liquidation overshoot; skips idiosyncratic news-driven drops). r10: OOS Sharpe 1.20 vs 0.78 for the unfiltered version at 1h lag.
  - `{"tranches": {}, "watch": {}, "worst_4h": {"ALGO": 3.01, "BTC": 3.02, "DOGE": 3.42, "HBAR": 3.35, "TRX": 0.59}}`
- **dn_funding_carry** — Delta-neutral: short HL perp + long spot (priced at HL oracle) on coins whose trailing-7d funding APR > 15% (exit < 5%), top 6, <=15% notional each, daily. r11 (HL data 2024-26): 13.7% CAGR, 1.4% vol, max DD -1.4%.
  - `{"holdings_apr": {"HYPE": 7.5, "PENDLE": 11.0, "ZEC": 9.4}, "last_day": "2026-11-15", "top_apr": {"BTC": 8.5, "CRV": 10.6, "ETC": 7.2, "HYPE": 7.5, "LDO": 11.0, "OP": 9.4, "PENDLE": 11.0, "RENDER": 9.3, "TAO": 7.7, "ZEC": 9.4}}`
- **trend_breakout_majors** — Donchian breakout (20/55/120d) long/short on BTC/ETH/SOL, inverse-vol, 25% vol target, daily (cut from 40% on 2026-10-31 to balance risk across books, r15).
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **trend_breakout_universe_lo** — Long-only Donchian breakout on 40-coin universe, inverse-vol, 40% vol target, daily; positions scaled by 30d efficiency-ratio gate [0.1, 0.3] (r27: Sharpe IS 1.38→1.46, OOS 0.53→0.67, lower DD); vol-managed since 2026-11-08 (r48: × min(1, own 365d vol / 20d vol on active days) — de-risk only; full engine IS 1.35→1.53, OOS 0.64→0.85, max DD −19.7→−18.9% / −20.4→−15.0%).
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **trend_ewmac_majors_lo** — RETIRED — long-only multi-speed EWMA crossover on BTC/ETH/SOL, 30% vol target, daily.
  - `{"last_signal": "2026-10-30T23:00:00+00:00", "paused": "RETIRED 2026-10-31 (r15): weakest live book (OOS Sharpe 0.33) and redundant with the other trend books (corr 0.56-0.67); dropping it lifts the combined backtest Sharpe 1.22 -> 1.41."}`
- **xs_bab** — EXPERIMENTAL (r46): betting-against-beta on the 40-coin universe — long lowest / short highest quartile of 60d beta to the equal-weight market, inverse-vol, legs beta-matched (gross 1, ~zero realised beta), weekly staggered over 7 offsets. Backtest IS 1.46 / OOS 0.59 Sharpe after costs, robust to beta window and quantile; ~0 corr with xs_momentum. Weak 2026 YTD (-1.9 Sharpe): kill if live Sharpe < 0 after 3 months. RETIRED 2026-11-08 02:xx by r47.
  - `{"last_signal": "2026-11-07T23:00:00+00:00", "paused": "r47: residual market beta 0.1-0.2 (r46b check was misaligned), hedged OOS Sharpe 0.14-0.37, not additive to the portfolio OOS"}`
- **xs_funding** — Market-neutral: long lowest / short highest trailing-7d *Binance* funding (quartiles, 40 coins), inverse-vol, gross 1.0, daily. Binance funding is reconstructed from 1-minute premium-index archives (~1-2 day lag; r21 corr 0.99/settlement). r22 backtest 2024-26 w/ HL funding paid: Sharpe 1.44 / 0.82 / 1.20 at 0 / 24 / 48h lag. Legs beta-neutralised since 2026-11-03 (r44: Sharpe IS 1.31→1.37, OOS 1.46→1.95).
  - `{"last_day": "2026-11-15"}`
- **xs_momentum** — Market-neutral cross-sectional momentum on the 40-coin universe: long top / short bottom quartile by vol-scaled past 7/14/21d return (lookback ensemble; r39 vol-scaled ranking), inverse-vol, gross 1, weekly rebalance staggered over 7 offsets (≈1/7 of the book turns daily). r38: IS 1.14 / OOS 0.68 Sharpe alone; ~0 corr with xs_funding, 1/3 blend lifts XS Sharpe 1.31→1.65 IS, 1.44→1.59 OOS and halves drawdowns.
  - `{"last_signal": "2026-11-14T23:00:00+00:00"}`
- **xyz_weekend_fade** — EXPERIMENTAL (r32, only 29 weekends of history): HL xyz single-stock/index perps trade 24/7; Monday reverses ~half of the weekend move cross-sectionally. Sunday 23:00 UTC: long the weekend losers, short the winners, weights ∝ −(weekend move − mean), ≤10%/name, gross 1; exit at Monday's US close.
  - `{"last_signal": "2026-11-08 23:04: 35 trades, 38 names, weekend xs-dispersion 1.55%, exit Mon 21:00 UTC"}`

## Last 20 trades

| Time | Book | Key | Side | Notional | Fill | Reason |
|---|---|---|---|---:|---:|---|
| 2026-11-15T02:05 | claude_discretionary | BTC | buy | $626 | 62964.6 | discretionary: Review 2026-11-15 02:10 (due 11-16): hold a token BTC 0.05 long (restored from drift 0.04). BTC 62.9k (-7.9%/24h, -14% s |
| 2026-11-15T00:04 | dn_funding_carry | ENA | buy | $12,876 | 0.115365 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | UNI | buy | $12,052 | 4.87649 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ZEC/spot | buy | $2,659 | 862.731 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | HYPE | sell | $2,006 | 64.4673 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ZEC | sell | $2,654 | 862.133 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | UNI/spot | sell | $12,045 | 4.87956 | daily dn-carry |
| 2026-11-15T00:04 | dn_funding_carry | ENA/spot | sell | $12,869 | 0.115377 | daily dn-carry |
| 2026-11-15T00:04 | xs_momentum | AVAX | buy | $1,634 | 7.09625 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | TRX | buy | $1,514 | 0.306628 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | TIA | buy | $1,213 | 0.345826 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | LDO | buy | $1,125 | 0.267639 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ARB | buy | $1,117 | 0.127725 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | SEI | buy | $1,043 | 0.0376879 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | XLM | buy | $1,042 | 0.135457 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | INJ | buy | $1,028 | 6.80679 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ATOM | buy | $997 | 1.26138 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | DOT | sell | $1,005 | 0.708701 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ENA | sell | $1,045 | 0.115324 | signal 2026-11-14 23h |
| 2026-11-15T00:04 | xs_momentum | ZEC | sell | $1,142 | 862.252 | signal 2026-11-14 23h |
