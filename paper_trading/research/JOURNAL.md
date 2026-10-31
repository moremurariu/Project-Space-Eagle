# Research journal

Costs assumed in backtests unless noted: Hyperliquid taker 0.045% + 0.05% slippage per side; perp
funding modelled as 0.01%/8h paid by longs (or real Binance funding where noted). In-sample (IS) =
2019-01 → 2023-12, out-of-sample (OOS) = 2024-01 → 2026-10. Hourly Binance spot closes for 40 coins that
are also listed as Hyperliquid perps (some survivorship bias in the universe — keep in mind).

## 2026-10-31 — initial research round

### r01 trend following (daily rebalance, vol-targeted)
| Variant | IS Sharpe | OOS Sharpe | OOS CAGR | OOS MaxDD |
|---|---:|---:|---:|---:|
| BTC buy & hold | 1.06 | 0.69 | 23.8% | -54% |
| Breakout majors L/S (pvt 0.4) | 1.65 | 0.56 | 15.5% | -52% |
| Breakout majors long-only | 2.13 | 0.56 | 13.5% | -35% |
| Breakout 40-coin long-only | 1.38 | 0.53 | 10.0% | -24% |
| EWMAC majors long-only (pvt 0.3) | 1.69 | 0.32 | 5.2% | -34% |
| EWMAC 40-coin L/S | 0.70 | 0.27 | 3.7% | -31% |

Trend worked brilliantly 2019-2023, much weaker in the choppier 2024-26 regime. Breakout > EWMAC OOS.

### r02 cross-sectional
- XS momentum 30d/weekly: IS Sharpe 1.17 → OOS -0.23. 14d/weekly: 0.74 / 0.63. Unstable → not live.
- Short-term reversal (1-3d) and intraday reversal: strongly negative after costs (turnover kills it).

### r03 anomalies
- Hour-of-day profile does not persist (IS vs OOS correlation 0.16). Day-of-week: weak.
- Fear & Greed buckets: extreme greed → strong fwd 7d returns IS, nothing OOS. Not used.
- **Large 4h drops are followed by strong rebounds** (pooled, all coins): after a -8% 4h move, fwd 24h
  mean +391bp IS / +418bp OOS, median +323/+303bp, hit rate 64%/66%. Pumps are also followed by positive
  drift (weaker OOS).

### r04/r05 crash-rebound strategy (buy after 4h drop < thr, hold H hours)
- thr -10%, hold 24h, 15%/position, max gross 1: Sharpe 1.11 at lag 0, 0.85 at lag 1h, 0.68 at lag 2h.
  Yearly (lag 0): 2019 +9%, 2020 +62%, 2021 +410%, 2022 +12%, 2023 +34%, 2024 +58%, 2025 -5%, 2026 +3%.
- Edge decays fast with entry delay → live version scans every 3 minutes using Binance rolling-4h ticker.
- BTC-hedged version roughly halves returns; unhedged kept.

### r06 funding carry
- Binance cross-sectional mean funding APR: 2020 18.7%, 2021 37.3%, 2022 -3.1%, 2023 5.0%, 2024 11.3%,
  2025 0.1%, 2026 -0.3% → delta-neutral carry on Binance alts is ~dead since 2025.
- Hyperliquid funding is structurally higher (11% APR baseline when premium ~0; BTC ~9% APR in 2026Q3).
  Live DN carry uses HL rates (entry >10%, exit <3%, top-6).
- **Directional XS funding** (long lowest 7d funding, short highest, inverse-vol, gross 1):
  lb 168h daily: IS 0.91 / OOS 1.11 Sharpe; lb 168h weekly: 0.54 / 1.49. Promoted to live (daily).

### Live books started 2026-10-31 15:34 UTC ($100k each)
trend_breakout_majors, trend_breakout_universe_lo, trend_ewmac_majors_lo, xs_funding, dn_funding_carry,
crash_rebound, claude_discretionary, benchmark_btc_hold.

### r07 TradFi trend on HL xyz instruments (FRED daily history, 2000-2026) — REJECTED
Instruments: SP500 (2016+), NDX→XYZ100, WTI→CL, Brent, EURUSD, USDJPY, Henry Hub→NATGAS, TLT proxy
from 10y yield, gold (PAXG, 2020+). Costs 0.09%/side + 5.5% APR financing for longs.
Breakout/EWMAC blend, 12% vol target: Sharpe 2000-09 ≈ 0.0, 2010-19 ≈ -0.1…-0.4, 2020-26 ≈ 0.15;
long-only 2020-26 ≈ 0.45 but negative 2010-19. Too few markets for trend to work; not promoted.
Possible revisit: equity-index carry/seasonality or crypto-equity lead-lag (xyz trades 24/7 incl. weekends).
