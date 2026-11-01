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

### 2026-10-31 19:04 — ops + Polymarket probe
- New `python -m ptrade.live.wakeup` bundles sync / catch-up / daemon + fetcher restart / status (HOURLY.md updated).
- Polymarket data access mapped (see backlog #8); resolved-market price history must be rebuilt from trades.

### 2026-10-31 20:03 — Polymarket forward data collection started
Historical calibration from APIs is biased (trades API: no time filter, offset cap 10k → only the latest trades of big
markets; CLOB prices-history empty once resolved). Started an hourly forward collector inside the full cycle:
markets ending within 7 days hourly + top-500 daily (bid/ask/last/24h volume/liquidity). Calibration study once a
few hundred snapshotted markets have resolved.

### r15 portfolio view of live strategies (daily streams, 2024-01 → 2026-10)
| Stream | CAGR | Vol | Sharpe | MaxDD |
|---|---:|---:|---:|---:|
| trend_brk_majors (L/S, pvt 0.4) | 12.7% | 42.4% | 0.49 | −51% |
| trend_brk_univ_lo | 9.9% | 21.0% | 0.56 | −23% |
| trend_ewmac_maj_lo | 5.5% | 27.7% | 0.33 | −33% |
| crash (lag 1h) | 24.3% | 22.5% | 1.08 | −15% |
| crash_mkt (lag 1h) | 33.3% | 25.5% | 1.25 | −12% |
| dn_carry (HL, 15/5) | 13.2% | 1.4% | 9.2 | −0.4% |
| BTC hold | 23.9% | 46.8% | 0.69 | −53% |
Correlations: trend↔crash ≈ 0, carry ↔ everything ≈ 0, trend books inter-corr 0.45-0.67, crash↔crash_mkt 0.66.
Equal-weight books: Sharpe 1.22, CAGR 18.8%, MaxDD −14%. Inverse-vol: Sharpe 2.2 but dominated by carry (whose
vol is likely understated by the premium proxy) → not adopted.
Sensitivity: drop ewmac → 1.41; also scale majors trend ×0.6 → **1.52 Sharpe, 19.5% CAGR, −5.9% MaxDD**.
→ Retired `trend_ewmac_majors_lo`; `trend_breakout_majors` vol target 0.40 → 0.25. (Chosen on the same OOS window —
treat the uplift as optimistic; rationale is risk balance + removing a redundant book.)

### r16/r17 crash-rebound conditioning (−10% 4h events, entry lag 1h, hold 24h) — no change
Event study (3,869 pooled events, overlapping): BTC near its 50d MA ("flat") → rebounds ≈ 0 (+40/+62bp) vs
+400…+920bp when BTC trends; coin already −20% over 24h → +955/+778bp vs ~+250bp; volume spikes inconsistent.
Backtest (fees + 0.1% slip): regime filter OOS 0.78→1.02 but IS 0.90→0.65 (drops most of 2021's rebound);
deep-capitulation ×1.5 sizing ≈ no effect; market-wide + regime + deep: OOS 1.33 vs 1.20 but IS 0.68 vs 0.83.
Inconsistent across periods → live crash books unchanged. (Event-study edges shrink once overlap and the 100%
gross cap are applied.)

### r18 Binance positioning metrics (OI, long/short ratios, taker ratio) — REJECTED as standalone
New data: `binance.load_metrics()` ← data.binance.vision daily `metrics` archives (5-min OI, top-trader and global
long/short ratios, taker L/S volume; published with ~1 day lag) — cached hourly for BTC/ETH/SOL since 2022-01.
Daily features lagged a full extra day for publication; Spearman IC vs fwd 1/3/7d, pooled BTC/ETH/SOL:
- Sign-consistent but tiny: retail L/S z (−0.06/−0.01 at 1d, −0.02/−0.04 at 7d), top-vs-retail (+0.05/+0.02),
  OI 7d change (−0.04/−0.00 at 1d, −0.09/−0.02 at 7d) — 2022-23 / 2024-26.
- Unstable: top-trader position z (−0.11 → +0.01 at 7d), taker ratio flips sign.
Too weak for a standalone book; keep as candidate features for an ML/composite model (backlog #5).

### 2026-11-01 00:04 — daily rebalance + fixes
- Daily signals applied: trend majors net −0.58 (2 trades), universe long-only opened first small position (+0.02),
  DN carry kept HYPE/PENDLE/UNI/ENA/kPEPE/ZEC (all ≥ 5% exit threshold).
- Polymarket gamma API now caps pages at 100 → collector paginates (499 binary markets in the daily snapshot).
- Discretionary unchanged (BTC 0.30 / ETH 0.15): Binance reports 90% of stolen assets replenished, BTC steady
  ~77.5k, F&G 34 — contrarian-long thesis intact.

### r12b/r19/r20 XS funding: the edge is real but the live signal is out of reach (still PAUSED)
- r12b (HL funding now cached for 35/40 coins): Binance-funding signal Sharpe 1.14 on those 35; HL-funding signal
  0.01; HL reversed −1.15 → HL funding carries no XS information; the edge is Binance-specific.
- r19 lag robustness (Binance actual funding): signal lag 0/24/36/48/72h → Sharpe 1.49/1.28/1.05/1.36/1.24 (2021+),
  1.24/1.12/1.16/1.35/1.37 (2024+). The 7d funding signal is slow, so a 1-3 day stale signal is fine.
- r20 reconstruct Binance funding from daily `premiumIndexKlines` archives (~1 day lag; new loader
  `binance.load_premium_index`, 40 coins cached from 2024-01): rank corr of 7d sums vs actual only 0.76
  (quartile overlap 61% top / 74% bottom); strategy on reconstructed signal Sharpe 0.72 (lag 0) / 0.23 (lag 48h);
  with per-coin 4h/8h settlement intervals 0.80 / 0.60. Not good enough — the edge lives at the extremes where
  Binance's 1-min weighted premium averaging and rate caps matter. xs_funding stays paused.
- Next options: (a) ask for a non-geo-blocked network path to Binance fapi (would make `fundingRate` live),
  (b) try 1-min `premiumIndexKlines` for exact averaging, (c) monthly funding archives with ≤30d lag (test decay).

### r21/r22 Binance funding reconstructed from 1-minute premium index → xs_funding RESUMED (2026-11-01 02:20)
- r21: per-settlement reconstruction F = P + clamp(I − P, ±0.05%), P = linearly time-weighted 1m premium over the
  interval, per-coin interval (4h/8h) and interest baseline from the latest actual archives. Aug-Sep 2026, 40 coins:
  median corr 0.99 per settlement, MAE ≈ 0.05bp (BNB the outlier: ~0 interest component).
- New module `ptrade/data/funding_recon.py` (monthly 1m archives + daily files with ~1-2 day lag; cached per
  month/day as settlement series). 40 coins built from 2024-01 in ~5 min.
- r22 (2024-01 → 2026-10, HL funding *paid* since positions live on HL): 7d-sum rank corr vs actual 0.983.
  | Signal | lag 0h | lag 24h | lag 48h |
  |---|---:|---:|---:|
  | actual Binance funding | 1.34 | 1.15 | 1.38 |
  | reconstructed (1m) | **1.44** (26% CAGR, DD −15%) | 0.82 | 1.20 (21%, −18%) |
  Positive every year 2024/2025/2026 in all variants. Live lag ≈ 48h (yesterday's 1m file not yet published at the
  00 UTC rebalance) → expect Sharpe ~1.
- `xs_funding` switched to this signal and unpaused; holds (no trades) if < 20 coins have funding data ≤ 4 days old.

### r23 HL hourly premium extremes (live-available) → relative forward returns — not promoted
Per-coin 14d z-score of HL premium (perp vs oracle), entry 1h after, cross-sectionally demeaned forward returns:
- z 2..3: +16/+12/+30bp (24h) and +21/+28/+23bp (72h) in 2024/2025/2026 → mild *momentum*, consistent sign.
- z > 3: +63/+4/+38bp (24h); z < −3: −36/+59/−32bp (24h) — 2025 flips the short side.
Edge ≈ one round trip of costs (~10bp+); not worth a book. Consistent with r12 (HL positioning ≈ momentum-ish,
unstable). Possible use: tilt inside a composite model.
