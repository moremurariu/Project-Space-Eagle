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

### r08 TradFi retest with Yahoo history (gold/silver/copper/S&P since 1928/NDX/DXY/VIX) — REJECTED
Data: `data_imports/yahoo/` (one-off import by a full-network agent; =F series not roll-adjusted).
- Trend, 11 markets (metals, S&P, NDX, DXY, WTI, Brent, EUR, JPY, natgas), 12% vol target, 0.12%/side +
  5.5% APR long financing: Sharpe 2000-09 0.10-0.21, 2010-19 -0.19…-0.38, 2020-26 0.04-0.45 (long-only best).
  Metals only: ~0.1-0.3. Cost drag ≈ 4-6%/yr at 30-50x turnover is most of the problem.
- S&P turn-of-month (last day + first 3): Sharpe 0.54 (1928-59) → 0.31 → -0.04 (1990-09) → -0.09 (2010-26). Arbitraged away.
- VIX regime: fwd 20d S&P return rises monotonically with VIX in 2010-26 (0.5% at VIX<15 → 7.6% at VIX>40)
  but not in 1990-09; as a timing rule Sharpe 0.16-0.26 vs buy & hold 0.75. Not promoted.
Takeaway: with HL xyz costs and a short instrument list, TradFi trend/seasonality doesn't clear the bar.
Remaining TradFi angle worth testing: weekend price discovery on xyz perps vs Monday open (24/7 venue).

### r09 Deribit DVOL signals (2021-03 → 2026-10, hourly)
- VRP (DVOL − 30d realised) quintiles vs BTC fwd 7d: no consistent pattern across 2021-23 / 2024-26.
- 24h DVOL change quintiles vs BTC fwd 24h: monotonic in both periods (q0 −0.16/−0.12%, q4 +0.39/+0.27%).
  As a standalone "long BTC after vol spike" rule (expanding-quantile threshold, lag 1h): q80 IS 0.27 / OOS
  −0.02 Sharpe; q90 IS 0.05-0.39 / OOS 0.36-0.49. Not robust → rejected as standalone.
- Crash events (−8% 4h) rebound far more when context is market-wide: BTC 4h < median → +506/+718bp fwd
  24h vs +233/+213bp; high DVOL → +447/+991bp vs +150/+350bp. Consistent in both periods.

### r10 crash-rebound v2 (lag 1h, 0.045% fee + 0.1% slippage)
| Variant | IS Sharpe | IS MaxDD | OOS Sharpe | OOS CAGR | OOS MaxDD |
|---|---:|---:|---:|---:|---:|
| baseline thr −10% (live `crash_rebound`) | 0.90 | −46% | 0.78 | 21% | −29% |
| thr −8% | 0.68 | −56% | 0.91 | 34% | −35% |
| **thr −10% & BTC 4h < −3% (market-wide)** | 0.83 | −47% | **1.20** | 31% | −21% |
| thr −10% & BTC 4h < −5% | 1.05 | −34% | 1.18 | 18% | −16% |
| size 22.5% market-wide / 7.5% idiosyncratic | 0.89 | −45% | 0.69 | 17% | −27% |
→ Promoted `crash_rebound_mkt` (market-wide filter, BTC 4h < −3%) as a parallel live book alongside the
unfiltered one, to compare live.

### r11 HL-native funding backtests (23 coins with cached HL funding, 2024-01 → 2026-10)
HL cross-sectional mean funding APR by quarter: 2024Q1 46%, Q2 9%, Q3 4%, Q4 25%, 2025Q1 4%, Q2 5%, Q3 14%,
Q4 2%, 2026Q1 2%, Q2 6%, Q3 11%, Q4 9%.
DN carry (short perp + long spot, P&L = funding − Δpremium, 0.20%/side for both legs):
| Rule | CAGR | Vol | MaxDD | 2024 | 2025 | 2026 |
|---|---:|---:|---:|---:|---:|---:|
| entry 10% / exit 3% (old live) | 9.4% | 1.6% | −1.9% | 22.5% | 4.6% | 0.7% |
| **entry 15% / exit 5%** | **13.7%** | 1.4% | −1.4% | 25.8% | 7.7% | 6.3% |
| entry 20% / exit 8% | 13.0% | 1.3% | −1.3% | 24.9% | 7.1% | 5.6% |
| entry 10%/3%, 72h lookback | 1.5% | 2.0% | −9.8% | 14.8% | −2.9% | −6.4% |
| always-on BTC+ETH, 0.9 notional | 12.7% | — | — | | | |
→ live DN carry moved to entry 15% / exit 5%. Short lookbacks chase noise; 7d is right.
Caveat: basis P&L proxied by HL `premium` changes; real spot-leg execution elsewhere adds friction.

Directional XS funding with the **HL** funding signal (15 coins): Sharpe −1.5 (CAGR −32%) — opposite of r06.

### r12 diagnosing XS funding (2024+)
A Binance signal, 40 coins: Sharpe 1.24 · B Binance signal, same 15 coins: −0.08 · C HL signal, 15 coins:
−1.52 · D HL signal reversed: +0.60 (2026 negative) · E Binance signal w/ HL funding paid: −0.11.
HL vs Binance 7d funding rank corr = 0.53. The edge needs Binance funding AND a broad universe.
### r13 OKX as live proxy for Binance funding — NO
OKX swap funding (40/40 coins, last ~3 months) vs Binance: rank corr 0.50, top-quartile overlap 48%.
→ `xs_funding` PAUSED (positions closed at −0.43% total). Binance fapi is geo-blocked so the validated
signal can't be computed live. Next: re-test HL signal on all 40 coins once the fetcher has them.

### r14 ML cross-sectional model (LightGBM, walk-forward yearly, 40 coins, 2021-2026) — REJECTED (v1)
20 features from Binance spot only (multi-horizon returns, realised vol, volume z-scores, taker-buy share,
BTC beta/residual, distance to 30d high/low, BTC regime); target = rank of next-24h relative log return.
- Predictive power is real: mean daily rank IC 0.045-0.095 every year (IC t-stat 3-7.5 annualised).
- Gross top-minus-bottom quintile spread: 2021 12bp/d, 2022 9, 2023 1, 2024 16, 2025 33, 2026 −21bp/d.
- Net backtests all strongly negative (−17% to −34% CAGR) even with score smoothing + no-trade buffers
  (turnover 86-330x/yr). Diagnosis: the rank/log target rewards predicting *volatility drag* — the short side
  loads on high-vol coins whose median return is low but whose mean simple return isn't (σ²/2 ≈ 12bp/day at
  5% daily vol), so rank-IC does not convert into P&L. (Also fixed a bug: inverse-vol sizing on rank-normalised
  vol put almost all weight in one coin.)
- v2 ideas (backlog): regress clipped *simple* returns, vol-scaled target, real inverse-vol sizing, weekly horizon.
