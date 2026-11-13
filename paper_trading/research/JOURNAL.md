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

### r24 weekend effects — REJECTED
Fri→Mon (00 UTC) moves vs Mon→Wed: BTC corr +0.09 (2019-23) / −0.03 (2024-26); cross-sectional weekend winners
minus losers over Mon-Tue +20bp (t 0.55) / −5bp (t −0.16). No continuation or reversal edge. Day-of-week means noisy.

### r25 pairs mean reversion (8 economically-linked pairs, log-ratio z-score, β from 60d) — REJECTED
Equal-weight pair portfolio: window 20d/entry 2.0 → IS 0.05 / OOS 0.36; 60d/2.0 → IS −0.64 / OOS 0.56;
20d/2.5 → IS −0.74 / OOS 0.13. OOS positives driven by a single pair (UNI/AAVE 0.9-1.9) → data-mining risk;
ETH/BTC and ARB/OP negative OOS. Crypto relative-value spreads trend more than they revert at daily horizons.

### r26 ML cross-sectional v2 — marginal, not promoted
Fixes from r14 diagnosis: target = vol-scaled, demeaned 7d *simple* return (clipped ±3), weekly rebalance, true
inverse-vol sizing, walk-forward yearly 2021-2026. q=0.2: CAGR 3.3%, Sharpe 0.25, MaxDD −40% (yearly −29/+10/+4/
+26/+28/−8%); q=0.3: Sharpe 0.19. Huge improvement over v1 (−17…−34% CAGR) but too weak/unstable to trade.
Next ML attempt only with new information sources (reconstructed Binance funding, positioning, HL premium) once the
reconstructed funding history is long enough to train on.

### r27 trend books gated by Kaufman efficiency ratio (trendiness) — PROMOTED for universe LO
Gate = clip((ER_n − lo)/(hi − lo), 0, 1) per asset, multiplied into daily trend weights.
| Book | Variant | IS Sharpe / MaxDD | OOS Sharpe / MaxDD |
|---|---|---|---|
| majors L/S (pvt .25) | baseline | 1.71 / −33% | 0.60 / −35% |
| | ER30 [.1,.3] | 1.81 / −21% | 0.56 / −26% |
| | ER30 [.15,.4] | 1.60 / −20% | 0.64 / −18% |
| universe LO | baseline | 1.38 / −35% | 0.53 / −24% |
| | **ER30 [.1,.3]** | **1.46 / −20%** | **0.67 / −20%** |
| | ER30 [.15,.4] | 1.32 / −15% | 0.86 / −17% |
ER60 worse everywhere. Universe LO with ER30 [0.1, 0.3] improves Sharpe and drawdown in BOTH periods → live
(`signals.efficiency_gate`). Majors: Sharpe mixed → unchanged.

### r28 crash-rebound: continuous 5-minute scanning vs hourly-close scanning (5m data, 2024-01 → 2026-09)
Spot 5m klines for 40 coins (monthly archives, `research/fetch_5m.py`). Rolling 4h drop < −10% at 5m resolution,
entry 5 min later, hold 24h, 15%/position, fees + 0.1% slippage.
| Scan | Filter | Sharpe | CAGR | MaxDD | entries |
|---|---|---:|---:|---:|---:|
| every 5m | none | 0.77 | 27% | −60% | 797 |
| hourly closes | none | 0.56 | 13% | −28% | 531 |
| every 5m | market-wide | 0.63 | 16% | −65% | 607 |
| **hourly closes** | **market-wide** | **0.95** | **24%** | **−24%** | 411 |
Continuous scanning enters mid-cascade (falling knives) → much deeper drawdowns. Hourly-close signals are as good
or better risk-adjusted. → Live crash books switched to hourly-bar signals only (no more 3-minute ticker scans);
an always-on host for faster scanning is NOT worth it.

### r29 trend majors sized by implied vol (Deribit DVOL vs its 1y median) — REJECTED
Baseline (realised-vol target): 2021-23 Sharpe 0.84 / DD −33%; 2024-26 0.60 / −35%.
× (median DVOL/DVOL)^0.5: 0.76 / −36% | 0.63 / −34%. ×(…)^1.0: 0.73 / −39% | 0.67 / −33%.
Helps recent period slightly, hurts 2021-23 → inconsistent; realised-vol targeting kept.

### 2026-11-01 15:05 — forward news dataset started (24h into live trading)
Historical timestamped crypto news isn't available from free APIs here (GDELT 429, RSS = latest items only), so every
full cycle now appends deduplicated headlines from 7 RSS feeds (cointelegraph, coindesk, decrypt, theblock, dlnews,
bitcoinmagazine, cryptoslate) to `state/news/headlines_YYYY-MM.csv` (seen_at, published, source, title). First pull:
164 headlines. Enables a headline-shock event study (hacks, ETF flows, macro, listings) once weeks of data accrue —
optionally with LLM-classified sentiment/topic per headline.

### r30 crash-rebound entry timing: scale-in vs delay vs confirmation (hourly, lag 1h) — SCALE-IN PROMOTED
Same setup as r10 (40 coins, 4h drop < −10% on hourly closes, hold 24h, 0.045% fee + 0.1% slippage), 15% total/coin.
| Variant | all: IS Sharpe / DD | all: OOS Sharpe / DD | mkt-wide: IS | mkt-wide: OOS |
|---|---|---|---|---|
| baseline 15% at signal (old live) | 0.90 / −46% | 0.78 / −29% | 0.83 / −47% | 1.20 / −21% |
| delay entry 1h / 2h / 3h | 0.68 / 0.47 / 0.48 | 0.73 / 0.63 / 0.75 | 0.56 / 0.40 / 0.33 | 0.98 / 0.84 / 0.66 |
| first green hour within 6h | 0.56 / −50% | 0.57 / −28% | 0.42 / −50% | 0.79 / −21% |
| 7.5% + 7.5% if close ≤ signal −3% within 12h | 0.99 / −36% | 0.99 / −28% | 0.92 / −36% | 1.28 / −20% |
| **7.5% + 7.5% if close ≤ signal −5%** | **1.02 / −36%** | **0.93 / −27%** | **0.93 / −36%** | **1.23 / −21%** |
| 7.5% + 7.5% if close ≤ signal −8% | 1.06 / −36% | 0.89 / −26% | 0.93 / −36% | 1.23 / −21% |
| 10% + 10% add @ −5% | 0.99 / −39% | 0.95 / −28% | 0.89 / −40% | 1.26 / −21% |
The rebound is strongest right at the signal close (every hour of delay costs Sharpe; waiting for a green hour is
worse still), but averaging down on a further drop improves Sharpe in BOTH periods for BOTH books and cuts IS
MaxDD −46% → −36%. Robust to the add threshold (−3/−5/−8%). → Both live crash books now trade two tranches:
7.5% at the signal close, +7.5% on an hourly close ≥5% below the signal close within 12h, each held 24h from its
own trigger (`CrashRebound` tracks `tranches`/`watch` in book meta; bars processed chronologically on catch-up).

### r31 crash-rebound exit timing (on top of r30 scale-in) — NOT PROMOTED (24h kept)
| Exit | all: IS Sharpe / DD | all: OOS Sharpe / DD | mkt: IS | mkt: OOS |
|---|---|---|---|---|
| hold 12h | 0.69 / −36% | 1.09 / −23% | 0.59 / −38% | 1.94 / −14% |
| hold 18h | 0.71 / −38% | 0.99 / −25% | 0.69 / −38% | 1.92 / −14% |
| **hold 24h (live)** | **1.02 / −36%** | **0.93 / −27%** | **0.93 / −36%** | **1.23 / −21%** |
| hold 30h | 0.84 / −43% | 0.97 / −28% | 0.70 / −43% | 1.26 / −21% |
| hold 36h | 0.56 / −49% | 1.25 / −26% | 0.47 / −48% | 1.53 / −21% |
| hold 48h | 0.54 / −55% | 1.29 / −25% | 0.51 / −55% | 1.50 / −21% |
| 24h + take-profit +5% / +10% | 0.81 / 0.94 | 0.81 / 0.98 | 0.75 / 0.83 | 1.12 / 1.28 |
| 48h + take-profit +10% | 0.69 / −42% | 1.22 / −22% | 0.60 / −42% | 1.53 / −21% |
| ensemble of holds 12/24/48h | 0.78 / −41% | 1.18 / −25% | 0.70 / −39% | 1.63 / −15% |
No exit rule beats 24h in both periods. Warning sign: IS Sharpe spikes at exactly 24h (0.69 at 18h, 0.84 at 30h), so
the IS figure of the live rule is parameter-lucky (24h was originally chosen in-sample); OOS prefers 12-18h for the
market-wide book and longer holds for the unfiltered one. A hold-length ensemble is the robust choice in principle
(OOS +0.25/+0.40 Sharpe) but loses IS → by the promotion rule, not adopted. Expect live crash-book Sharpe nearer
~0.7-1.0 than the headline 1.2. Take-profit exits cut both tails and don't help.

### 2026-11-02 03:05 — forward option-skew dataset started
Deribit has no free skew history (DVOL only), so every full cycle now snapshots BTC+ETH options
(`deribit.skew_snapshot`): for the expiries nearest 7d and 30d, ATM IV, 25-delta risk reversal and butterfly (BS
deltas from mark IV, interpolated over OTM strikes) plus put/call OI and volume ratios → `state/options/skew_YYYY-MM.csv`.
First reading: BTC 30d ATM 37.5%, RR25 −4.9 (put skew; post-hack fear), ETH 30d ATM 51.2%, RR25 −5.3.
Plan: in 1-2 months test RR25 extremes / changes as a contrarian signal for BTC/ETH (with DVOL history as context).

### r32 weekend price discovery on HL xyz perps (24/7 equities) — EXPERIMENTAL BOOK `xyz_weekend_fade`
Data: HL 1h candles, 44 xyz perps by OI (HL keeps 5000 bars → 2026-04-07 … 2026-11-02 = 29 weekends, 1135 obs).
Weekend move = Fri US close → Sun 23:00 UTC; next = Sun 23:00 → Mon US close.
- Pooled corr(weekend, Monday): all −0.13 (β −0.26), equity-like −0.14 (β −0.29), commodities/FX −0.07; indices
  SP500/XYZ100 ≈ 0 → the effect is single-stock overreaction while cash markets are shut, not index-level.
- Per-weekend cross-sectional β: mean −0.56, t −2.6, negative in 72% of weekends.
- Fade (long weekend losers / short winners, Sun 23:00 → Mon close, 0.18% round-trip):
  | Weights | net bp/weekend | t | hit | H1 / H2 (bp) |
  |---|---:|---:|---:|---|
  | sign, mean split | +24.8 | 1.07 | 59% | +38 / +12 |
  | sign, median split | +16.4 | 0.83 | 59% | |
  | linear ∝ −x (dollar-neutral) | +88.3 | 2.05 | 66% | +160 / +21 |
  | **linear, ≤10%/name** | **+51.3** | **1.73** | **62%** | **+92 / +13** |
Positive after costs in both halves but only 29 weekends (no multi-year OOS) → NOT a full promotion; started as an
explicitly experimental $100k book (linear, ≤10%/name, gross 1, HIP-3 fee 0.09%/side + book-walk slippage; live
entry cost ≈ 0.12%/side). Kill criterion: cumulative net < 0 after 12 live weekends. Also: Market.fee now charges
0.09% for HIP-3 (xyz) perps instead of 0.045%.

### r33 BTC → xyz crypto-equities (COIN/MSTR/HOOD/CRCL) while US cash is closed — REJECTED (inconclusive)
HL xyz 1h, 147 sessions (2026-04 → 11), overnight = US close → 23:00 UTC before the next session.
xyz crypto-equities already track BTC fully overnight (β ≈ 1.0). Next-session return on BTC's overnight move:
weekends −0.53 (t −2.5, but 4 names × 29 weekends are not independent → clustered t ≈ −1.3), weeknights +0.57
(t +1.7); idiosyncratic residual ≈ 0. Opposite signs across windows, small samples → nothing to trade. The weekend
reversal is consistent with r32 and partly captured by `xyz_weekend_fade`.

### r34 xyz overnight reversal: weeknights and weekend entry/exit timing — weeknights REJECTED, weekend book kept
Same linear XS fade as r32 (≤10%/name, 0.18% round trip), HL xyz 1h, 2026-04 → 11.
- Weeknights (US close → next day entry, exit next US close, 118 nights): β −0.22/+0.01/−0.00 for entry 23/08/13 UTC;
  net −17/−27/−25bp per night → no overnight overreaction on weeknights (pre-market trading prices it in).
- Weekend reversal happens Sunday night → Monday morning: entering Monday 08:00 or 13:00 UTC leaves nothing
  (β −0.09, net −15/+5bp). Entry/exit grid (net bp per weekend, t): every combination of entry Sun 18/21/23 UTC
  × exit Mon 03/08/13 UTC/US close is positive (+26…+51bp, t 1.3-2.4); exit Mon 08:00 has the best t (≈2.1-2.2),
  US-close exit the highest mean but more noise. Live book unchanged (entry Sun 23:00, exit Mon US close; entry
  catch-up window ends Mon 03:00 UTC, which is right — later entries miss the reversal).

### r35 hour-of-day seasonality (Binance hourly, BTC & equal-weight 40) — REJECTED
Correlation of the 24 hourly mean returns IS (2019-23) vs OOS (2024+): BTC +0.16, EW40 −0.13, EW40−BTC −0.01 → the
hour-of-day profile is noise, except one persistent BTC pattern: 21:00-22:59 UTC (around the US close) +4.8/+3.4bp IS,
+3.5/+3.0bp OOS (t ≥ 2 both), and 23:00 UTC −2.8/−2.7bp. ≈ +6-8bp/day for a 2-hour BTC long vs ≈ 10-12bp taker round
trip + slippage → not tradable as a taker strategy; timing the daily 00 UTC rebalance around it is worth < 1bp/day of
turnover. Possible maker-only revisit if limit-order paper fills are ever modelled.

### r36 universe-LO trend filtered by funding crowding (real Binance funding paid) — REJECTED
| Variant | IS Sharpe / CAGR / DD | OOS Sharpe / CAGR / DD |
|---|---|---|
| live (breakout + ER30 gate) | 1.30 / 32% / −20% | 0.65 / 10% / −20% |
| × gate 1 below 20% APR → 0 at 50% | 1.24 / 19% / −17% | 0.73 / 9% / −18% |
| × gate 30% → 80% | 1.36 / 24% / −19% | 0.66 / 9% / −20% |
| × gate 50% → 100% | 1.46 / 29% / −19% | 0.61 / 9% / −20% |
| drop top funding tercile | 1.29 / 17% / −16% | 0.66 / 8% / −12% |
| only below-median funding | 1.16 / 8% / −10% | 1.03 / 10% / −9% |
12% of held coin-hours have 7d funding > 50% APR. No variant improves Sharpe in both periods (tight filters help
OOS, loose ones IS); filters mostly just cut exposure. Crowding information is already monetised by `xs_funding`
(long low-funding / short high-funding), so not adopted.

### r37 slow 24h bleeds (24h < thr with no 4h < −10% cascade) — positive standalone, NOT promoted (not additive)
Prompted by RUNE −15.6%/24h (THORChain halt) not triggering the 4h crash books. Event study (fwd 24h from 1h after
signal, de-overlapped): 4h cascade +243/+408bp (IS/OOS); slow 24h < −12% +167/+102bp; < −15% +215/+261bp (hit
62-63%); < −20% +235/+613bp (67/43 events). Idiosyncratic subset (BTC 24h > −3%) weaker and noisy.
Backtest (15%/pos, hold 24h, lag 1h, fees + 0.1% slip): slow < −15% Sharpe 0.82 IS / 0.69 OOS (DD −30%);
< −20% 0.43 / 1.23. But daily corr with the live crash book (r30) is 0.52, and a 50/50 mix vs crash alone:
IS 1.12 vs 1.10, OOS 0.99 vs 1.06 → no improvement in the crash sleeve, so no new book. Rebound after selling
pressure is one effect; the 4h-cascade trigger captures it better.

### r38 cross-sectional momentum: lookback robustness + blend with XS funding — PROMOTED (`xs_momentum`)
40 coins, long top / short bottom quartile by past return, inverse-vol, gross 1, weekly; real Binance funding paid.
| Lookback (single offset) | IS Sharpe | OOS Sharpe | | Staggered (avg of 7 weekly offsets) | IS | OOS |
|---|---:|---:|---|---|---:|---:|
| 7d | 0.45 | −0.31 | | 7d | 0.74 | 0.39 |
| 14d | 0.76 | 0.74 | | 14d | 0.93 | 1.11 |
| 21d | 0.63 | −0.14 | | 21d | 1.21 | 0.33 |
| 30d | 1.13 | −0.14 | | 30d | 1.30 | −0.04 |
| 60d | 0.94 | −0.00 | | 60d | 0.72 | 0.01 |
14d alone looks cherry-picked (neighbours fail OOS), so use a lookback ensemble. Staggering offsets (removes
rebalance-day luck) helps every lookback. Daily corr with XS funding ≈ 0.03.
| Portfolio | IS Sharpe / DD | OOS Sharpe / DD |
|---|---|---|
| XS funding alone (lag 48h, live-like) | 1.31 / −27% | 1.44 / −22% |
| ensemble 7/14/21 staggered alone | 1.14 / −26% | 0.68 / −19% |
| ensemble 7/14/21/30/60 alone | 1.27 / −23% | 0.48 / −16% |
| **2/3 funding + 1/3 ens 7/14/21** | **1.65 / −16%** | **1.59 / −9%** |
| 1/2 funding + 1/2 ens 7/14/21 | 1.71 / −14% | 1.49 / −10% |
Positive alone in both periods and improves the XS sleeve in both → new live book `xs_momentum` (ensemble
7/14/21d × 7 staggered weekly offsets averaged on a daily grid; ≈15%/day one-way turnover). Caveat: momentum is
−13% YTD in 2026 (blend still positive); survivorship bias of the 40-coin universe flatters momentum somewhat.

### r39 XS momentum ranking: raw vs BTC-residual vs vol-scaled (7/14/21d ensemble, staggered weekly) — VOL-SCALED PROMOTED
| Ranking | IS Sharpe / DD | OOS Sharpe / DD | 2026 YTD | 2/3 funding + 1/3 mom: IS / OOS |
|---|---|---|---:|---|
| raw return (r38 live) | 1.14 / −26% | 0.68 / −19% | −13% | 1.66 / 1.59 |
| BTC-residual return (60d beta) | 1.38 / −32% | 0.37 / −22% | −16% | 1.75 / 1.40 |
| **return / (30d vol·√lookback)** | **1.22 / −24%** | **0.88 / −19%** | **−6%** | **1.66 / 1.63** |
Variants are 0.88-0.90 correlated with each other, ~0.04 with XS funding. Vol-scaled ranking (a t-stat-like score
that stops high-vol coins dominating both tails) improves standalone Sharpe in both periods → live `xs_momentum`
switched (`signals.xs_momentum(vol_scaled=True)`); re-signal forced at 11:10. Residual momentum helps IS only.

### r40 portfolio view v2 (current live books, daily streams; DN carry & xyz weekend excluded — short history)
| Book | IS 2020-23 CAGR / vol / Sharpe / DD | OOS 2024+ CAGR / vol / Sharpe / DD |
|---|---|---|
| trend_majors (pvt .25) | 37% / 26% / 1.32 / −34% | 11% / 26% / 0.52 / −35% |
| trend_univ_lo (ER30 gate) | 35% / 22% / 1.46 / −19% | 10% / 16% / 0.70 / −20% |
| xs_funding (lag 48h) | 32% / 23% / 1.34 / −27% | 26% / 17% / 1.41 / −20% |
| xs_momentum (vol-scaled ens.) | 21% / 21% / 1.00 / −24% | 13% / 15% / 0.85 / −17% |
| crash (r30 scale-in) | 56% / 41% / 1.29 / −30% | 25% / 18% / 1.31 / −12% |
| crash_mkt (r30 scale-in) | 48% / 39% / 1.20 / −30% | 31% / 24% / 1.25 / −12% |
| BTC hold | 55% / 68% / 0.99 / −77% | 24% / 47% / 0.69 / −53% |
| **equal-weight 6 books** | **42% / 16.5% / 2.21 / −12.5%** | **21% / 9.8% / 1.96 / −6.5%** |
| inverse-vol (90d, lagged) | 27% / 14% / 1.74 / −14% | 12% / 9% / 1.33 / −8% |
Correlations: trend books 0.48 with each other, crash↔crash_mkt 0.88, everything else |ρ| ≤ 0.25 (XS books ≈ 0
with all). Equal weight beats inverse-vol (crash-book vol spikes right after events, so inverse-vol de-risks them
exactly when they pay). Portfolio runs at only ~10% vol OOS → a 1.5x book-level scale would target ~30% CAGR /
~15% vol / ~−10% DD. Decision: keep 1x until ~4 weeks of live data confirm live ≈ backtest (fees, slippage,
funding), then revisit leverage (backlog #6).

### r41 long/short trend breadth (breakout, pvt 0.25) — NOT PROMOTED (mixed)
| Universe | IS Sharpe / DD | OOS Sharpe / DD | + ER30 gate: IS / OOS (DD) |
|---|---|---|---|
| majors 3 (live) | 1.53 / −36% | 0.53 / −36% | 1.64 / 0.50 (−23% / −26%) |
| top 6 (+XRP, BNB, DOGE) | 1.26 / −42% | 0.75 / −29% | 1.43 / 0.81 (−29% / −20%) |
| top 10 | 1.28 / −35% | 0.63 / −27% | 1.42 / 0.64 (−23% / −19%) |
| all 40 | 1.04 / −24% | 0.26 / −19% | 1.24 / 0.46 (−16% / −11%) |
Broader L/S trend trades IS Sharpe for OOS Sharpe (top6+gate: −0.10 IS, +0.28 OOS, lower DD in both) — fails the
"better in both periods" bar, and majors-3 was itself chosen on IS data. Keep live as is; top6 + ER30 gate is the
fallback if `trend_breakout_majors` keeps lagging live. All-40 L/S is clearly worse (short side of alts whipsaws).

### 2026-11-02 14:40 — live execution-cost check (first ~47h, $955k traded, 151 fills)
Paper fills walk the live HL L2 book. Realised per side: fee 5.0bp (perps 4.5, spot 7.0), slippage 2.9bp — perps
0.5bp (majors) to 3.4bp (xs_funding alts), xs_momentum 2.0bp, DN-carry spot legs 5.8bp. Backtests assume
4.5 + 5bp per side for perps → assumptions are conservative at current $1-20k order sizes (slippage scales
with size: ~1.5bp at $1k, ~4bp at $5k for alts). No change needed; re-check if leverage raises order sizes.

### r42 Polymarket calibration — PRELIMINARY (2 days of forward snapshots, 250 resolved markets, 547 obs)
Outcomes cached in `state/polymarket/outcomes.csv` (590 of 1053 ended markets fetched so far). One obs per market ×
time-to-expiry bucket, spread ≤ 10c. Pooled by quoted mid: 0-5c → 0/118 won (mid 1.1%); 5-15c → 0/31 (mid 8.6%);
15-35c → 24% (mid 25%); 35-65c → 51% (mid 48%); 65-85c → 91% ± 5% (mid 74%); 85-95c → 80% (n=10); 95c+ → 100%.
Direction matches the favourite-longshot bias (longshots overpriced, 65-85c favourites underpriced), but the sample
is tiny, obs within a market are correlated, and it is dominated by short-dated crypto-price and sports markets.
No strategy yet; rerun weekly as outcomes accumulate (`python research/r42_polymarket_calib.py 600` fetches the
next batch of outcomes). Target: ≥1,500 resolved markets before testing "buy 65-90c favourites / sell 5-15c
longshots ≤24h before expiry" net of the spread.

### r43 XS on the change in funding (not the level) — REJECTED
Short coins whose 7d funding rose most vs 72h/168h earlier, long those where it fell most (lag 48h, gross 1):
IS −0.10 / OOS −0.89 (72h), IS −0.20 / OOS +0.09 (168h) vs the level signal (live) 1.31 / 1.45. Blending halves the
Sharpe. The XS-funding edge is in the level (persistent crowding / carry), not in funding momentum.

### r44 xs_funding: beta-neutral legs — PROMOTED
Live attribution (2026-11-03 08:00, book −0.94%): longs −$2,045 (CRV/ATOM/WLD/JUP −7…−9% in the alt sell-off), shorts
+$1,164 (SOL/LINK −4…−5%) → the dollar-neutral book was long alt beta. Backtest: scale the long and short legs so
Σ w·β = 0 (β = 60d hourly beta to the equal-weight 40-coin market, clipped [0.2, 3], leg scale ≤ 2, gross 1):
| Variant | IS Sharpe / DD | OOS Sharpe / DD | beta to EW market |
|---|---|---|---|
| dollar-neutral (live until now) | 1.31 / −27% | 1.46 / −22% | −0.03 |
| **beta-neutral legs** | **1.37 / −31%** | **1.95 / −15%** | +0.01 |
Average beta was already ≈ 0, but it swings with which coins sit in each leg; neutralising it improves Sharpe in both
periods (IS DD slightly deeper). → live (`XSFunding._beta_neutral`), re-balance forced 08:15. Today it shrinks the
long leg 0.50 → 0.465 and grows the short leg to 0.535.

### r45 trend majors: signal update frequency (prompted by the 2026-11-04 short squeeze, book −3.9% in one hour) — NOT CHANGED
| Variant | IS Sharpe / DD | OOS Sharpe / DD | OOS turnover |
|---|---|---|---:|
| once daily at 00 UTC (live) | 1.53 / −36% | 0.49 / −36% | 20x/yr |
| re-evaluate the daily-bar signal every 6h | 1.05 / −43% | 0.22 / −37% | 126x |
| re-evaluate every hour | 0.60 / −51% | −0.38 / −53% | 277x |
| every hour, min_trade 0.05 | 0.87 / −45% | 0.07 / −43% | 154x |
| average of 4 six-hour-offset books (stagger) | 1.31 / −40% | 0.56 / −35% | 19x |
Reacting faster to intraday moves whipsaws (turnover ×6-14) and is worse in both periods; staggering is mixed
(IS −0.22, OOS +0.07). The once-daily book is right; squeezes like Nov 4 are the cost of the strategy — the next
00 UTC rebalance halved the short (−0.79 → −0.42) exactly as the signal preview predicted.

### 2026-11-05 18:06 — first live crash-rebound event (live-vs-backtest log)
Broad selloff (BTC −6%/24h to 75.7k, ETH −8%, alts −7…−10% over 4h; no clear headline). ENA closed the 18:00 bar
−10.15% over 4h with BTC 4h < −3% → both `crash_rebound` and `crash_rebound_mkt` bought tranche 1 (7.5%) at ~$0.16,
fees $4 each; exit 2026-11-06 18:00 UTC; tranche 2 armed if an hourly close ≤ 5% below the signal close within 12h.
TIA (−9.5%) and FIL (−8.4%) were close. Track the realised 24h P&L vs the r30 event-study expectation (+2-4%/event).

### r42 update 2026-11-06 — Polymarket calibration with midterm markets resolved
Pooled by quoted mid (n obs / mid / win rate): 0-5c 644 / 0.7% / 0%; 5-15c 69 / 8.4% / 0%; 15-35c 142 / 25% / 23%;
35-65c 334 / 49% / 52%; 65-85c 77 / 74% / 83% (± 4%); 85-95c 49 / 91% / 92%; 95c+ 100 / 99% / 100%.
The sample is now dominated by election markets ("Will Republicans…" 452 obs), which are highly correlated with each
other. The favourite-longshot pattern persists (5-15c longshots 0/69; 65-85c favourites +9pp) but the effective
sample is still small. Keep collecting; test a strategy once ≥1,500 resolved non-election markets exist.
→ 2026-11-06 18:08 exit (24h): ENA bought 0.16032, sold 0.16583 → +3.4% gross on the position, ≈ +3.3% after fees,
slippage and funding; book +0.25% (7.5% position). Tranche 2 never armed (no further −5%). In line with the
r10/r30 event-study expectation (+2-4% per event). Live crash events so far: 1, both books +$249.

### 2026-11-07 00:05 — first live week (2026-10-31 15:34 → 11-07 00:05)
BTC 78.4k → 75.9k (benchmark −2.1%, max DD −7.2%). Combined books −0.11% on $1.0M (ex-benchmark).
| Book | Return | Notes |
|---|---:|---|
| xs_momentum (from 11-02) | +0.51% | best book; vol-scaled ranking since r39 |
| crash_rebound / _mkt | +0.25% each | one event (ENA, +3.3% net on the position) |
| dn_funding_carry | +0.10% | $195 funding vs $189 entry fees → now net positive |
| xs_funding | −0.17% | recovered from −1.13%; beta-neutral legs since r44 |
| trend_breakout_majors | −0.53% | short squeeze 11-04 (−3.9% in 1h), short halved next day |
| claude_discretionary | −1.38% | long BTC into a falling market; ETH re-entry 2418 → 2227 (−7.9% on a 10% slot ≈ −0.8% of book) |
| xyz_weekend_fade | 0 | first entry Sunday 11-08 23:00 UTC |
Execution costs below backtest assumptions (fees 5bp + slippage ~3bp per side). No errors in a week of hourly cycles.
Sample far too short to judge any book; next portfolio review after ~4 weeks (leverage decision, backlog #6).

### r46 cross-sectional low-risk anomalies: low-vol, MAX, betting-against-beta — BAB PROMOTED (experimental `xs_bab`)
Binance 40-coin hourly panel, daily signal at 00 UTC, quartile legs, inverse-vol within legs, weekly rebalance averaged
over 7 staggered offsets, 0.045% fee + 5bp slippage, real Binance funding paid. Beta = 60d beta to the equal-weight market.
| Variant | IS 2019-23 Sharpe / CAGR / DD | OOS 2024+ Sharpe / CAGR / DD |
|---|---|---|
| low-vol, $-neutral | −0.76 / −25% / −81% | −0.22 / −9% / −36% |
| low-vol, beta-neutral | 0.02 / −3% / −55% | 0.34 / 4% / −20% |
| low-MAX, $-neutral | −0.93 / −27% / −80% | −0.12 / −5% / −28% |
| low-MAX, beta-neutral | −0.15 / −7% / −51% | 0.51 / 7% / −17% |
| BAB, $-neutral | 0.36 / 6% / −45% | −0.09 / −6% / −30% |
| **BAB, beta-neutral** | **1.46 / 43% / −33%** | **0.59 / 9% / −22%** |
$-neutral low-risk books are just short beta, so they lose in crypto. Beta-matching the legs fixes that. r46b robustness
for BAB beta-neutral: beta window 30/60/120d × quantile 0.25/0.33 → IS 1.07–1.47, OOS 0.59–0.88, all positive.
~~Realised beta of the book's daily returns is −0.01 vs both the EW market and BTC.~~ **Wrong: the check
compared returns offset by one day. Corrected in r47: beta ≈ 0.1–0.2, correlation ≈ 0.5.** Net dollar exposure is +0.34 on average:
long more notional in low-beta coins, but beta-matched. Sharpe by year: 2019 1.4, 2020 1.7, 2021 2.6, 2022 0.0,
2023 0.8, 2024 1.4, 2025 1.5, **2026 YTD −1.9** (recent weakness). Daily correlation with xs_momentum is 0.03.
The library implementation `signals.xs_bab` zero-fills weights for coins with missing vol. The research script let
those NaN cells forward-fill old weights, which flattered it slightly. The clean version gives IS 1.40 / OOS 0.45,
still positive both. Promoted as **experimental** book `xs_bab` (live from 2026-11-08 01:xx UTC; first signal was
long BTC/BNB/ETH/SOL/TRX/XRP, short LDO/UNI/APT/WLD/ENA/NEAR; gross 1, net +0.42).
Kill rule: live Sharpe < 0 after 3 months.

### r47 portfolio view v3 (r40 books + xs_bab) and xs_bab beta re-check — xs_bab RETIRED (2026-11-08 ~02:15)
Equal-weight portfolio with vs without BAB: IS Sharpe 2.27 vs 2.21 (DD −10.2% vs −12.5%); **OOS 1.95 vs 1.95, CAGR
18.8% vs 20.6%, DD −8.1% vs −6.5%**. So BAB adds nothing out of sample. In correctly aligned daily returns xs_bab
correlates 0.51 with BTC hold. r47b, with the market exposure regressed out:
| Period | beta vs EW / BTC | corr vs EW / BTC | raw Sharpe | hedged Sharpe (EW / BTC) |
|---|---|---|---:|---|
| IS 2020-23 | 0.16 / 0.22 | 0.48 / 0.52 | 1.31 | 0.79 / 0.93 |
| OOS 2024+ | 0.09 / 0.18 | 0.37 / 0.47 | 0.44 | 0.37 / 0.14 |
| 2026 YTD | 0.08 / 0.21 | 0.30 / 0.59 | −1.65 | −1.77 / −1.92 |
Hedging with a rolling 90d lagged beta gives IS 0.58 / OOS 0.34. The ex-ante beta-matching against the EW market
leaves a residual long-market tilt: net $ is +0.35 on average, long BTC/majors. That tilt explains part of the r46
Sharpe. What remains is weak OOS, fading in 2026, and does not improve the portfolio. Under the r37 rule
("positive standalone but not additive → not promoted"), the r46 promotion was a mistake: it rested on the
misaligned beta check. `xs_bab` is set `paused` and its positions closed after ~1h live. Cost ≈ $63 fees plus
~1h P&L (−0.08%). Lesson: compute realised beta from the same hourly→daily returns as the book.

### r48 volatility-managed books (scale by own long-run vol / recent vol, lagged) — PROMOTED for universe LO (de-risk only)
r48 (daily-stream approximation, scale capped at 2x, scaling turnover charged): mixed for most books.
Crash: IS 1.29 → ~1.0, OOS better. trend_majors: OOS worse at 20d. xs_momentum: IS worse. xs_funding: mixed by window.
Only **trend_univ_lo** improved in both periods (IS 1.46 → 1.53–1.63, OOS 0.69 → 1.06–1.08).
r48b robustness for trend_univ_lo, which is flat 47% of days: estimating vol from active days only
(gross > 5%) kept the gain, so it is not a flat-period artifact. All 8 variants (window 20/60, cap 1.5/2,
all/active days) gave IS 1.45–1.69 and OOS 1.01–1.11. 2024, 2025 and 2026 each improved vs base (1.44 / −0.19 / 0.02).
Full hourly engine (real funding, scaling turnover, min_trade 0.02), like-for-like base IS 1.35 / OOS 0.64:
| Variant | IS 2020-23 Sharpe / CAGR / DD | OOS 2024+ Sharpe / CAGR / DD |
|---|---|---|
| base | 1.35 / 35.1% / −19.7% | 0.64 / 10.1% / −20.4% |
| cap 1.5, win 60 | 1.41 / 42.2% / −27.0% | 0.93 / 20.2% / −21.9% |
| cap 1.25, win 60 | 1.44 / 40.5% / −25.1% | 0.87 / 17.0% / −21.2% |
| cap 1.0, win 60 | 1.41 / 34.8% / −18.8% | 0.80 / 12.3% / −18.9% |
| **cap 1.0, win 20** | **1.53 / 36.4% / −18.9%** | **0.85 / 11.7% / −15.0%** |
Promoted the de-risk-only version: scale = min(1, own 365d vol / 20d vol), using active days, lagged one day.
It needs no leverage and improves Sharpe and drawdown in both periods; the 60d window also improves both,
so the choice is not fragile. Live in `trend_breakout_universe_lo` from 2026-11-08 (`vol_scale`, recomputed daily
from the book's own backtest returns over the last 520 days). Currently the book is ~flat (gross 1%), scale 1.0.

### r49 de-risk-only vol management (r48 method, cap 1.0) on the other books — REJECTED for all three
Full hourly engine, real funding, scaling turnover charged.
| Book | base IS / OOS Sharpe | win 20 IS / OOS | win 60 IS / OOS |
|---|---|---|---|
| trend_majors | 1.33 / 0.53 | 1.31 / 0.48 | 1.36 / 0.61 |
| xs_funding | 1.27 / 1.43 | 1.23 / 1.60 | 1.15 / 1.53 |
| xs_momentum | 1.04 / 0.89 | 1.13 / 0.77 | 1.04 / 0.74 |
trend_majors: only the 60d window helps, and only slightly, so it depends on the window → no. xs_funding: OOS
better with much lower DD (−22 → −14%), but IS worse in both windows → no; it may be worth revisiting after a live
drawdown. xs_momentum: OOS worse → no. Vol management stays on universe LO only (r48).

### r50 xyz weekend fade vs weekend cross-sectional dispersion — no filter (plus pre-flight of tonight's first entry)
Pre-flight: a dry run of `XYZWeekendFade` at a simulated 2026-11-08 23:05 UTC (temp book) gave 36 trades / 38 names,
gross 1.00, net +0.01. Friday close resolved to 2026-11-06 21:00 UTC (EST, after the DST change) and exit to Mon
2026-11-09 21:00 UTC. A re-run an hour later made no second entry. Entry fees ≈ $90/side; weekend dispersion so far
0.78%, which is low.
r50 (r32 data refreshed: 29 weekends 2026-04-13 → 11-02, linear ≤10%/name, 0.18% round trip). Weekend dispersion =
cross-sectional std of demeaned weekend returns; median 1.5%, 10th pct 0.75%. corr(dispersion, net P&L) = 0.05.
Terciles: low (median 0.86%) +46bp, 70% hit; mid +3bp; high +67bp. Thresholds 0.5–1.25% change nothing (above
+32…+47bp, below +29…+149bp). → No dispersion filter; low-dispersion weekends still clear costs.
Caution: the last 10 weekends net −45bp/weekend on average (−273, −201, −145bp losers), vs the 29-week mean +36bp.
The edge may be decaying. The live kill rule (cumulative net < 0 after 12 weekends) stands.

### 2026-11-08 06:xx — scoping a wider XS universe (backlog #13)
HL main dex: 179 live perps, 40 in the universe, 109 more with a Binance USDT spot pair. HL 24h volume (Sunday
morning) of the current universe ranges from 0.2M (ETC) to 515M (BTC); the median name trades ~1.5M. Candidates by HL volume:
HYPE 138M, PUMP 45M, then XPL, AERO, MET, TRUMP, GRAM (2–4M), MEGA, ZRO, STRK, WLFI (1.5–1.8M), then ~20 names at
0.3–0.9M (ASTER, ETHFI, MORPHO, RUNE, JTO, PENGU, kSHIB, DASH, VIRTUAL, ICP, FET, PYTH, W, GMX, …). Next: fetch
history and test xs_momentum breadth 40 vs ~70 on 2024+.

### r51 xs_momentum on a wider universe (40 + top-N HL candidates by OI, Binance 1h, 2024+) — REJECTED (not robust)
The 30 candidates' first bars span 2023-06 … 2026-09 (the panel starts 2023-06; 15 of them listed in 2025-26).
Same live recipe (vol-scaled 7/14/21d, staggered weekly, 0.045% + 5bp, no funding).
| Universe | 2024 | 2025 | 2026 YTD | 2024+ Sharpe / CAGR / DD |
|---|---:|---:|---:|---|
| 40 (live) | 1.58 | 0.98 | −0.46 | 0.87 / 12.5% / −18.3% |
| 40+30 | 1.11 | 1.66 | −1.63 | 0.65 / 8.3% / −18.5% |
| 40+30, skip first 90d after listing | 1.47 | 1.44 | −1.97 | 0.65 / 8.4% / −21.5% |
| 40+15, skip 90d | 1.29 | 1.63 | −0.60 | 0.93 / 13.3% / −19.4% |
More breadth helps in 2025 and hurts in 2026 in every variant, and the full-period result depends on N. There is no
in-sample period for the young names. Not adopted; the universe stays at 40. Revisit with xs_funding breadth
only if funding reconstruction for the extra names becomes cheap.

### 2026-11-08 08:xx — ops: guard against partial Binance data gaps at the daily rebalance (+ discretionary review)
Prompted by the Binance-hack news flow, since the live signals depend on Binance public mirrors. Failure mode found:
if one symbol's kline refresh failed, `binance.load_many` dropped it from the panel. The XS/trend signals then gave
it weight 0, and the 00 UTC rebalance would have closed that position, then reopened it a day later (two sets of
fees, random exposure gap). `xs_funding` dropped any coin whose last price was NaN in the same way. Fixes:
(1) `load_many` falls back to the cached history when a refresh fails, so the coin stays in the panel, stale.
(2) New `hold_for_data_gaps` in `live/strategies.py`, used by every `DailySignal` and by `xs_funding`. If a coin
expected on the signal bar has no price there but had data in the prior 72h, the rebalance is skipped and retried
each cycle (`meta.data_wait`), for at most 6h, then it trades anyway. Tested on a synthetic gap: held, then traded after the 6h
limit; a clean panel is unaffected.
Discretionary review: hold BTC 0.25 / ETH 0. BTC 75-76k for 48h; F&G 26-37; put skew persists (BTC RR25 −4.9/−4.2 at
7d/30d); Binance hack aftermath (CFO out, staged withdrawals). Rules unchanged (<72k → 0.10, >78k → 0.30); review by 11-11.

### r52 time-series trend on alt/BTC ratios (Donchian 20/55/120 on each alt/BTC, alt leg w, BTC leg −Σw) — REJECTED
| Variant | IS 2020-23 Sharpe / CAGR / DD | OOS 2024+ Sharpe / CAGR / DD | avg gross |
|---|---|---|---:|
| L/S, pvt 0.25 | 0.72 / 16% / −30% | 0.07 / −1% / −29% | 1.23 |
| long alt-vs-BTC only, pvt 0.25 | 1.01 / 15% / −19% | −0.29 / −3% / −19% | 0.37 |
| L/S, pvt 0.40 | 0.79 / 25% / −39% | 0.01 / −6% / −40% | 1.79 |
| long only, pvt 0.40 | 0.97 / 15% / −21% | −0.32 / −3% / −20% | 0.37 |
Alt/BTC ratio trends worked in the 2020-21 alt cycles and are flat-to-negative since 2024, when alts bled steadily
against BTC with sharp squeezes. Correlation 0.3 with xs_momentum, which already captures the relative-strength part
better (OOS 0.89). Not promoted.

### r53 Polymarket daily BTC digitals ("above $K on <date>", 12:00 ET fix) vs Black-Scholes with Deribit 7d ATM IV — no edge yet (tiny sample)
Collector snapshots 2026-10-31 → 11-08: 67 markets, 1272 hourly quotes, only **7 resolution dates** (11-01 … 11-07).
Outcomes come from the Binance BTCUSDT hourly close at 12:00 ET. Spreads are tight (median 0.1c); IV median 39%.
Brier on informative quotes (3–97c, n 418): market mid 0.178, BS model 0.183, average of the two 0.180. The market
is at least as good as a naive lognormal. Trading on model-vs-quote gaps (cross the spread, first signal per market):
edge > 3/5/8/12c → +0.1 / −2.9 / +2.0 / +6.6c per contract, all t < 0.3, from 26/16/11/4 markets. Nothing there.
Outcomes within a date share one BTC fix, so the effective sample is ~7. In the 90-100c bucket only 80% won, because
BTC fell through strikes on 11-05. Revisit once ~30 dates have resolved (early December), adding skew (RR25) to the
digital (vanilla-spread approximation) and the "reach/dip" barrier markets (touch probabilities).

### r54 xs_momentum with a funding-crowding filter (drop longs in the top / shorts in the bottom funding quantile) — REJECTED
Trailing-7d Binance funding with a 48h lag (live-like); legs renormalised to the original gross; real funding paid.
| Variant | IS 2020-23 Sharpe / CAGR / DD | OOS 2024+ Sharpe / CAGR / DD |
|---|---|---|
| base | 1.04 / 21.2% / −24.1% | 0.89 / 12.8% / −18.5% |
| filter q 0.25 | 0.84 / 19.7% / −36.4% | 1.02 / 16.2% / −16.5% |
| filter q 0.15 | 1.11 / 25.8% / −23.2% | 0.95 / 14.3% / −15.6% |
| filter q 0.10 | 1.20 / 27.1% / −23.2% | 0.79 / 11.3% / −15.8% |
The sign of the effect flips with the quantile (IS and OOS improve together only at q 0.15, and only by +0.06–0.07),
so it is noise-level. Funding crowding is already priced by the xs_funding book. Not adopted.

### r55 live vs backtest tracking (same recipe on the Binance panel over the identical live window) — tracking good
| Book | Window | live | backtest | daily corr | live costs |
|---|---|---:|---:|---:|---|
| xs_momentum | 11-02 11h → 11-08 11h | +0.66% | +0.55% | 0.99 | fees $53, slippage $17, funding −$19 |
| trend_breakout_majors | 10-31 15h → 11-08 11h | −0.22% | −0.42% | 0.99 | fees $49, slippage $6, funding +$114 |
Daily returns match to within ~0.3pp on every day, including the 11-04 squeeze (−3.13% live vs −3.46% bt) and the
11-05 rebound (+3.11% vs +2.82%). Live ≥ backtest so far, helped by HL funding received on the majors short and
book-walk slippage below the 5bp assumption. Hourly correlation is low (0.16/0.39) only because of timestamps: live equity
is marked at ~:04 while backtest bars are bar-open indexed. Evidence for backlog #6 (leverage) once ~4 weeks exist; re-run weekly
(`research/r55_live_vs_bt.py`; add more books as their history grows).
→ r55 update (13:xx): **xs_funding** added, with r44 beta-neutral legs in the backtest.
Found a research-data gap first: `cache/binance_funding_*.parquet` (monthly archives) ends 2026-09-30, so
`load_binance_funding_panel` gave every coin the flat default rate for October–November. The backtest's xs_funding signal
over the live window was then pure ties (daily corr with live −0.66). After splicing in the live 1m-premium reconstruction:
live −0.15% vs bt +0.15% (10-31 → 11-08), daily corr 0.38. Holdings at the latest rebalance: weight corr 0.84; the same
names sit at both extremes (short BTC/XRP/ETC/LINK, long TRX/ETH/XLM/ATOM/SEI/JUP); the differences are quartile-boundary
names (TAO, LDO, SUI, OP, LTC, CRV, PENDLE, XLM). The live trailing window ends 11-07 00:00; the bt shift-48h window
ends 11-06 00:00. The rest of the P&L gap: live was not beta-neutral until 11-03 (bt is throughout), and it entered
at 11-01 02:20. Verdict: same book; tracking is noisier than for xs_momentum because edge names flip.
**Fix:** `load_binance_funding_panel(..., splice_recon=True)` (opt-in) fills each symbol after its last archive with the
cached reconstruction. Use it in all research from now on. Effect on earlier work: r44–r54 "2026 YTD" numbers had flat
funding for Oct–Nov, ~5 weeks out of 7 years, so their conclusions stand, but xs_funding's Oct 2026 backtest P&L was noise.

### r56 xs_funding re-run with corrected (spliced) funding — conclusions unchanged; the backtest's recent months are weak
`load_binance_funding_panel(splice_recon=True)` (r55 fix), same r44 recipes, IS 2019-23 / OOS 2024+:
| Variant | IS Sharpe / DD | OOS Sharpe / DD (r44, flat Oct-Nov funding) |
|---|---|---|
| dollar-neutral | 1.31 / −27% | **1.28** / −22% (was 1.46) |
| beta-neutral legs (live) | 1.37 / −31% | **1.88** / −15% (was 1.95) |
Beta-neutral is still clearly better, and the live recipe still passes. Sharpe by year: 2019 1.66, 2020 0.07, 2021 3.35, 2022 0.87,
2023 0.19, 2024 1.91, 2025 2.22, 2026 1.49. Monthly 2026: Jul −1.87%, Aug −2.48%, Sep −0.49%, **Oct −0.67%**, Nov +0.46% (MTD).
The edge has been soft for four months before and into the live start (live −0.32% so far). Expectations for the first live
month should be modest. It is not a kill signal: comparable flat stretches happened in 2020 and 2023 before strong years.

### 2026-11-08 15:xx — dn_funding_carry income check
Holdings (trailing-7d HL funding APR): UNI 13.6, HYPE 13.3, ZEC 12.9, PENDLE 11.8, ENA 10.8. All sit just above HL's
baseline 0.01%/8h ≈ 10.95% APR (the interest-rate component), so the book now harvests mostly the baseline. Funding by
coin since start: UNI $50, HYPE $46, ZEC $41, PENDLE $38, ENA $33, kPEPE $9 (exited). Last 24h: +$17.8 on the five
held, which is ≈ $6.5k/yr ≈ 6.5% on the $100k book at gross 1.4 (≈ 0.70 short notional → ~9% on the short leg).
kPEPE's −$5.2 in the last 24h was its exit day, when its funding had turned negative.
Entry fees ($215) are now paid back (cumulative funding $217). Each further day adds ~$18 with no turnover.
r11's 13.7% CAGR came from 2024–25 funding spikes; with the market at baseline, expect ~6–7%/yr on the book.
No change. The 5% exit threshold sits below the 11% baseline, so positions persist unless funding turns negative,
which is intended.

### 2026-11-08 16:xx — news collector health check (backlog #10 prep) — two feed bugs fixed
First 8 days: 802 unique headlines. Weekday volume ~90–120/day across feeds, weekends ~10–30. Median detection
lag 48 min (collected at each hourly wake-up). Keyword tags: hack/exploit 178 (dominated by the $6.3B Binance hack
story), ETF 27, SEC/regulation/sanctions 27, macro/Fed 18, liquidation 5, listing 3.
Findings: (1) dlnews: its 40 rows were all collected on 11-01, and **its RSS feed has not published since 10-28** (newest item
10-28 22:30). It is a stale feed, not a parser problem; it stays configured in case it resumes. An earlier draft of this note
blamed the global `max_items` cap. That was wrong: ~165 items across feeds never reached 500. The cap is now per feed anyway,
which is harmless. (2) **bitcoinmagazine returns 403** to the Python client since ~11-07, though curl gets 200, which
looks like deliberate blocking of automated clients. The feed was removed rather than worked around with a spoofed
User-Agent. Live sources now: cointelegraph, coindesk, decrypt, theblock, cryptoslate (+ dlnews if it resumes).

### 2026-11-08 17:xx — live book correlations (11-02 12h → 11-08 17h, 4h returns; ~37 obs, very noisy)
Equal-weight live portfolio (9 active books): annualised vol 3.8% vs 9.7% for the average book, so diversification
ratio 2.55. Its **correlation to BTC hold is −0.69**, so the live mix is currently a net short-BTC bet. Drivers:
trend_majors is net short (−0.59, corr −0.89 to BTC), and xs_funding (−0.42) and xs_momentum (−0.41) both lean short
majors right now (BTC is in xs_funding's short leg). Only discretionary (+0.25 BTC) and dn_carry (+0.17) lean the
other way. In the long backtest the portfolio has ≈ 0 beta, so this is regime-dependent (BTC downtrend → trend short).
Other pairs: crash ↔ xs_funding +0.52, dn_carry ↔ xs_funding −0.54 (both from the single 11-05 event, so ignore),
trend_majors ↔ xs_momentum +0.49. For the ~4-week portfolio review (backlog #6): add a combined-portfolio BTC-beta line
to the status report, and consider a portfolio-level beta cap before any leverage decision.
→ 18:xx: status report now prints **"Combined BTC exposure (7d realised, 4h returns)"**, a beta of the combined
ex-benchmark book to the BTC-hold book (`report._combined_btc_beta`). A first version summed book equities, so each newly
added book showed up as a +10% return (xs_bab 11-08 01h, xs_momentum/xyz 11-02); it now uses per-book returns weighted
by prior equity. Current reading: beta −0.049 ≈ −0.53 books of BTC (≈ −$53k BTC-equivalent: trend_majors −$59k,
discretionary +$25k, XS legs), corr −0.72.

### r42 update 2026-11-08 19:xx — Polymarket outcomes 1,063 → 1,633 resolved (1,409 non-election)
Pooled by quoted mid (n obs / mid / win rate ± se): 0-5c 744 / 0.8% / 0%; 5-15c 107 / 8.8% / 3.7% ± 1.8;
15-35c 208 / 25% / 24%; 35-65c 617 / 49% / 53% ± 2.0; 65-85c 149 / 74% / **83% ± 3.1**; 85-95c 74 / 91% / 93%;
95c+ 158 / 99% / 100%. The favourite-longshot bias persists with the bigger sample: longshots are overpriced (5-15c win
3.7% vs 8.8% implied), and favourites at 65-85c win ~9pp more than priced (≈ 3 se). Non-election resolved markets: 1,409,
just under the 1,500 bar. At the next weekly refresh (~11-15), test "buy 65-85c favourites / sell 5-15c longshots
near expiry" with spread, time-to-expiry buckets and per-event clustering.

### r57 Polymarket favourite-longshot as a strategy (prelim) — NO EDGE; the r42 calibration gap is not tradeable as measured
One quote per resolved non-election market: the latest snapshot 1–72h before close (median 2h). Cross the spread
(median 1c), no fees, $ returns averaged within events (close date + question stem) to handle clustering.
955 markets / 601 events.
| Trade | n / events | avg price | win | ret per $ | event-clustered mean (t) |
|---|---|---:|---:|---:|---|
| buy YES, mid 0.60–0.80 | 124 / 86 | 0.72 | 74% | +3.0% | −5.1% (−0.73) |
| buy YES, mid 0.65–0.85 | 123 / 84 | 0.77 | 76% | −0.2% | −5.3% (−0.82) |
| buy YES, mid 0.70–0.90 | 127 / 88 | 0.81 | 78% | −3.6% | −5.7% (−0.99) |
| buy YES, mid 0.80–0.95 | 81 / 62 | 0.88 | 80% | −8.9% | −8.0% (−1.40) |
| buy NO, mid 0.03–0.10 | 66 / 57 | 0.95 | 88% | −7.6% | −4.1% (−1.03) |
| buy NO, mid 0.05–0.15 | 87 / 73 | 0.92 | 87% | −4.6% | −0.4% (−0.12) |
| buy NO, mid 0.10–0.25 | 105 / 95 | 0.85 | 85% | +0.1% | −0.8% (−0.18) |
r42's pooled "favourites win +9pp" came from counting every hourly quote, so long-lived markets were heavily repeated
and correlated, and election markets dominated. Taken once per market at a tradeable price, the bias disappears or
reverses: near-expiry favourites at 80–95c win only 80%. No strategy. Keep collecting; at ~3,000 markets re-test by
category (sports vs crypto vs politics) and by time to expiry (24–72h vs < 6h), since the bias may live in one segment only.

### r58 xyz weekend fade: US-listed names vs names without a US cash session — keep the full universe
Prompted by tonight's planned book. Dry run at 21:00 for the 23:05 entry: 35 trades, 38 names, weekend dispersion
1.13%. The largest shorts are ANT −10.9% (new perp, $72M OI, not in the backtest cache), CXMT −9%, MRNA, ZHIPU, SPCX;
the largest longs are PURRDAT, NVDA, SOXL, MSTR, LITE, CRCL. Several of these are private/pre-IPO or Asia-listed, so they
have no US cash close for the r32 "overreaction while cash is shut" story.
Split (r50 data, 29 weekends, linear fade, 0.18% round trip): US-listed only (~25 names/wk) +30bp/wk, t 0.96, H1 +65 /
H2 −2bp; non-US/private only (CXMT, ZHIPU, SPCX, PURRDAT, UNITREE, SKHX, SKHY, SMSN; ~7 names/wk, 19 weekends, cap 35%)
+83bp, t 1.73, H1 +187 / H2 −11bp; all equity-like +40bp, t 1.29. Pooled Monday-on-weekend XS beta: −0.29 US-listed,
−0.33 non-US/private. Both groups revert, so there is no reason to exclude the non-US names. Both halves have weakened recently
(the last 10 weekends are negative in both groups), consistent with r50's caution. Entry tonight unchanged; the kill rule stands.

### 2026-11-08 22:04 — Sunday-evening sell-off (ops log)
BTC 75.9k → 74.6k (−1.4%/24h), ETH 2,240 → 2,130 (−4.6% over 4h); alts −4…−5% over 4h (PENDLE, ETH, ENA, JUP, SEI). No crash
signal (thr −10%). No clear headline: Binance France closures and the CFO exit are old news, US tech perps are soft (XYZ100 −0.5%).
trend_majors short +1.75%/24h → book +1.50%. Combined books turn positive (+0.03%) vs benchmark −3.73%. Combined BTC
exposure −0.55 books (corr −0.74) paid off here. Discretionary rules not hit (BTC > 72k). The xyz_weekend_fade entry runs at the 23:03
wake-up: its weekend returns are measured vs the Friday close, so the crypto-led drop feeds in via COIN/MSTR/HOOD/CRCL.

### 2026-11-08 23:04 — first live xyz_weekend_fade entry: execution cost 3x budget → liquidity cap added
Entry at 23:04: 35 trades, gross 1.00, net −0.01, weekend dispersion 1.55%. Fees $90 (9bp, as expected), but **slippage $222
(22bp)**, so the book opened at −0.31% against the ~0.12%/side budget. $204 of that came from two names: ANT (short, $10.1k) and
PURRDAT (long, $10.3k), each charged the 1% MAX_SLIP cap. Their L2 top-5 depth was only ~$0.7–2k on a Sunday night despite
$24–72M OI. The real cost would have been even higher (the cap binds). The other 33 names paid 1–5bp.
Fix for future weekends: `XYZWeekendFade._liquidity_cap` halves a name's target until its estimated book-walk
slippage ≤ 25bp (up to 6 halvings, else drop). Gross is not re-levered. A re-run now gives ANT and PURRDAT ×0.25, slippage $26
(vs $222), gross 0.84. `meta.liquidity_capped` records which names were cut. This week's positions are kept until
the Monday 21:00 UTC exit (selling now would pay the same slippage again; US-hours books should be deeper).
**Research caveat:** r32/r50/r58 charged a flat 0.18% round trip to every name. The thin pre-IPO/private names that
looked strongest in r58 (+83bp/wk) are exactly the ones with ~2%+ realistic round trips, so their edge is overstated.
TODO: re-run the fade with name-level costs (from L2 depth snapshots across a few weekends) before judging the book.

### 2026-11-09 01:xx — new forward dataset: xyz L2 depth (execution costs per name)
`hyperliquid.collect_xyz_depth` runs on every full cycle → `state/xyz_depth/depth_YYYY-MM.csv`: top-45 xyz perps by OI,
spread and book-walk slippage at $2.5k/$5k/$10k on both sides, plus OI and 24h volume. First snapshot (Mon 01:00 UTC):
median $5k slippage ≈ 1bp; the visible 20-level book of ANT cannot fill $5k ($72M OI!); PURRDAT 14–37bp at $5–10k
(it was >100bp at Sunday 23:00); NBIS, KORU, CXMT 4–12bp. After 3–4 weekends: re-run r32/r58 with name- and
hour-specific costs (Sun 23:00 entry, Mon 21:00 exit) and decide whether thin names should be excluded or just capped.
xyz_weekend_fade so far: −0.12% at 01:00 (entry cost 0.31%, +0.19% gross since).
→ 02:xx attribution of the first live weekend (mark-to-mid −$276, book −0.51% incl. entry costs): the worst names are all shorts
of private/Asia-listed perps that kept rising into Monday's Asia session: ANT −$274 (+2.7%), ZHIPU −$255 (+7.3%), UNITREE
−$176 (+8.8%), CXMT −$78, SPCX −$59. The best are US-listed longs: SOXL +$284, NBIS +$99, MSTR +$71, SNDK +$55, COIN +$47,
CRCL +$40. One weekend proves nothing, but the hypothesis is plausible: for names whose home market opens Monday morning
Asia time (ZHIPU/UNITREE/CXMT in China/HK, SMSN/SKHY in Korea), the weekend perp move anticipates that session, so it
continues rather than reverts. r58's split counted them as "non-US/private" together with SPCX/PURRDAT (private,
no session). Next r58 re-run: split Asia-listed vs private-no-session vs US-listed, using the name-level costs.

### r58b xyz weekend move → Monday: Asia-listed vs private vs US-listed — inconclusive; US names flipped to continuation lately
r50 data (29 weekends to 11-02). Pooled XS beta of the Monday return on the demeaned weekend move: US-listed (n 733) −0.29,
corr −0.12; Asia-listed CXMT/ZHIPU/UNITREE/SKHX/SKHY/SMSN (n 123) −0.33, corr −0.13; private SPCX/PURRDAT (n 49) −0.36.
Pooled, all three revert similarly. Per-weekend betas for the small groups are unstable (2–6 names a weekend). So the
"Asia names continue into their home session" idea from today's attribution is not supported by the backtest; one live weekend is not evidence.
**More important:** over the last 10 weekends the US-listed pooled correlation is **+0.12**, i.e. continuation, not reversal.
This is the bulk of the book and explains the recent losing weekends (r50: last 10 ≈ −45bp/wk). If the next 3–4 live weekends
also show continuation, retire the book early instead of waiting for the 12-weekend kill rule.

### 2026-11-09 04:xx — Polymarket collector health
10 daily files (10-31 → 11-09): 23–24 hourly snapshots every full day (one missed hour on 11-02), 850–1,500 distinct markets
per day, quotes present on 68–94% of rows, median 24h volume per market $30–110k. Healthy. With ~570 newly
resolved markets a week (r42), the ~3,000-market re-test of r57 by category is due around late November.

### 2026-11-09 06:xx — xs_funding live attribution (book −0.66%)
Current positions: long leg $50.9k (avg −4.25%, uPnL −$1,800; ATOM −$675, SEI −$524, TIA −$369), short leg −$43.8k (avg
−4.15%, uPnL +$2,042; XRP +$776, LINK +$504). The beta-neutral legs hedge each other as designed: the selloff hit both
sides about equally, so net is +$242 on open positions. The loss is costs: $418 fees+slippage on 5.5x turnover in 9 days, funding received
only +$80. Turnover by day: 10-31 2.0x (initial build/close), 11-01 1.0x (resume on reconstructed funding), 11-03 0.75x (r44
forced re-signal). Those three one-offs cost $284. Steady-state days (11-02, 11-05 … 11-09) average ~0.30x/day vs the
backtest's 0.28–0.30x/day, so execution is in line. The live book carries about −0.3% of one-off setup cost; nothing to change.
(05:04 check-in: the tool safety check failed repeatedly, so that hour's state was committed at 06:08.)

### 2026-11-09 07:xx — options-skew collector sanity (backlog #2)
BTC: 171 hourly rows (11-02 04h → 11-09 07h, ~1 missing). 7d RR25 −8.1 … −0.9 (median −4.5), 30d RR25 −6.3 … −1.7 (median −4.3):
put skew all week. ATM IV 7d 34–45%, 30d 34–41%. Skew is coincident with price: corr(30d RR25 level, trailing-24h BTC return)
0.63, corr(ΔRR25 24h, 24h return) 0.71. Puts get bid after drops. Any predictive test (backlog #2, after 1–2 months) must
therefore use skew *relative to* what the price move implies (residual of RR25 on trailing return), not the raw level.

### 2026-11-09 08:xx — weekend fade mid-Monday: −1.21% (mark P&L −$1,177 + $312 entry cost)
Losers have spread from the Asia-listed shorts (02:00) to the US longs: SOXL −$211, PURRDAT −$189, NVDA −$140, UNITREE −$140,
ANT −$111. The weekend losers keep falling (continuation, as in the last-10-weekend backtest pattern, r58b). Structural note:
after a crypto-led weekend selloff the fade is net long crypto-equities (COIN/CRCL/HOOD/MSTR −STRC = +13% of the book, −$253 so
far), so it adds crypto beta exactly when crypto is falling. If the r32 re-test with name-level costs keeps the book alive,
consider hedging that leg with a BTC perp short (or excluding crypto-equities when the weekend crypto move is large).
Discretionary: BTC 73.8k, 2.4% above the 72k cut-to-0.10 rule; to be applied at the first check-in below 72k.

### r59 xyz weekend fade: hedge the crypto-equity leg (COIN/MSTR/HOOD/CRCL/STRC) with a BTC perp — REJECTED
r50 data, 29 weekends; hedge = −(net crypto-equity weight) × BTC return Sun 23:00 → Mon US close, β≈1 (r33), +5bp/side on the hedge.
Unhedged +46.6bp/wk (t 1.52, std 165bp); hedged +39.8bp (t 1.38, std 155bp); last 10 weekends −33 vs −34bp. The net crypto-equity
weight averages |0.12|, and its BTC component correlates +0.42 with the book's P&L. Crypto also tends to revert after the weekend
move, so that exposure has paid on average. Hedging lowers risk a little and the mean more. No hedge; the 08:00 note is withdrawn.

### 2026-11-09 11:xx — trend_majors risk map (book +2.14%, short BTC −20% / ETH −19% / SOL −23%)
Daily closes are at the 20d and 55d channel lows for all three (pos −1.00). On the 120d channel BTC sits mid-range (+0.03), ETH −0.30,
SOL −0.42. A rally back to the 20d/55d midpoints (BTC +6/+8%, ETH +8/+15%, SOL +12/+21%) would neutralise those components;
the EWM smoothing spreads that over several days. So a sharp squeeze like 11-04 (−3.9% in an hour) is the main risk, and
r45 already showed faster signal updates do not help. Nothing to change.

### 2026-11-09 12:xx — ops log scan
`cache/daemon.log` (2.6k lines): all cycles report `"errors": {}`; no data-gap holds and no Binance fetch fallbacks triggered.
`cache/fetch_hl_funding.log`: one transient HL `fundingHistory` POST failure (SOL), retried by the next pass. Clean.

### 2026-11-09 21:04 — xyz_weekend_fade live weekend #1 result: **+1.22% net** (+$1,220)
Entry Sun 23:04 (35 names, dispersion 1.55%), exit Mon 21:04. Costs: entry $90 fees + $222 slippage, exit $96 + $133 = **$541
(0.54% of book)**. ANT alone cost $209 (1% capped slippage both ways); with the new ≤25bp liquidity cap it would have been ~$50.
Gross ≈ +1.76%. Path: −1.21% at Mon 08:00, while the weekend move continued through Asia/Europe (Asia-listed shorts ran
+7–9%), then reversal at the US cash open: +0.71% at 15:00 and +1.81% at 16:00, exit at +1.22%. Backtest expectation is
+40–50bp/week with ~165bp std, so this weekend is +0.5σ. Running tally: 1 weekend, cumulative net +1.22% (kill rule:
cumulative < 0 after 12 weekends; retire early if continuation persists, r58b). The timing detail matches r34: the
reversal happens in the US session, so an entry at Monday ~13:00 UTC (just before the US open) might capture it with less
exposure to the Asia-session continuation. r34 found Mon 13:00 entry ≈ +5bp (worse), so no change; keep watching.

### r60 HL hourly funding spikes → forward relative returns (event study) — REJECTED (no edge either way after costs)
HL hourly funding (cache, 2024-01 → 2026-11, universe coins), events = first hour with |APR| > 50/100/200% after 24h quiet.
Forward 8/24/72h return from the next hour's close minus the EW-40 return; fade (short positive / long negative spikes) net of a 0.19% round trip.
IS 2024 / OOS 2025+: the fade is negative almost everywhere (e.g. >100% APR positive spikes: 8h −46/−36bp, 24h −24/−85bp;
>50% negative spikes: 8h −25/−23bp, t −2.6/−4.1). Gross, the effect is mild continuation, but following the spike nets
≈ 0 to −15bp after paying the round trip twice. Funding extremes are not a short-horizon signal at these costs.
The slow version (xs_funding: trailing-7d ranking, daily) remains the way funding is used.

### r61 turn-of-the-month effect (last 2 + first 3 calendar days) — REJECTED (IS only, reverses OOS)
Daily 00 UTC closes. BTC: TOM +42bp/day vs rest +15bp in IS 2019-23 (diff t 1.3); OOS 2024+ −3 vs +11bp (t −0.7).
EW-40: IS +72 vs +18bp (t 2.3); OOS −17 vs +10bp (t −0.8). A long-only-in-TOM strategy: IS +187/+340bp per window
(BTC/EW), OOS −36/−104bp. This is a classic data-mined calendar effect that disappeared once crypto institutionalised,
the same pattern as r03/r24/r35 (hour-of-day, day-of-week, weekend).

### 2026-11-10 16:06 — discretionary: trim BTC 0.25 → 0.15 (sold 0.125 BTC at 73,209)
New macro information beyond the price rule: headlines put Fed rate-HIKE odds at ~96% after a Waller speech; gold −2.1% and oil −2.9%
over 24h (broad risk-off); BlackRock BTC ETF outflows $1.66B over 7 sessions; BTC −1.8% in an hour to 73.2k, ETH 2,027.
The systematic books are net short BTC (~−0.5 books; trend_majors +4.32%, combined +0.35% at a new high). The discretionary
book expresses my view: near-term bearish, so a smaller long, keeping a core for a fear-driven rebound (F&G 29–32).
New rules: BTC < 72k → 0.05; BTC reclaims 78k → 0.25. Review by 11-13.
Ops note: a `fast` cycle does not apply discretionary target changes; it needs a full cycle (`run_cycle(fast=False, only=[...])`).

### 2026-11-10 17:04 — live crash-rebound event #2: ZEC −12.6% over 4h → `crash_rebound` t1
ZEC closed the 16:00 bar −12.6% over 4h (signal close 1,010.95). `crash_rebound` bought tranche 1: 7.39 ZEC at 1,017.34
(mid 1,017.15), $7,520 (7.5%), fees $3.4 + slippage $1.4. Exit 2026-11-11 17:00 UTC. Tranche 2 armed if an hourly close is
≤ 960.4 (−5% vs signal) by 11-11 05:00. `crash_rebound_mkt` did not trade: BTC 4h > −3%, so it treats this as idiosyncratic.
Context: market-wide risk-off (hawkish Fed, BTC 73k, alts −4% over 4h), and ZEC had rallied on the NU7 upgrade (11-06) and a
proposed block-time cut (11-09). No specific negative headline; it looks like a crowded-long unwind. ZEC is also held in
dn_funding_carry (short perp / long spot, delta-neutral), unaffected. Backtest expectation +2–4% per event (r10/r30).
→ 2026-11-11 07:04 ZEC tranche 2 triggered: the 06:00 hourly close was ≤ 986 (−5% vs the re-set signal ref 1,037.87; the watch
was extended by the 17:00/18:00 re-signals). Bought 7.70 ZEC (7.5%); exit 2026-11-12 07:00. Book gross 0.15 at −0.09%. Tranche 1
exits 11-11 19:00. Note: the re-signals moved the t2 reference *up* (to the later, higher signal closes), so t2 bought
about 3% below the t1 entry rather than 5%. That is by design (r30 uses the latest signal close), but noted for the
live-vs-backtest log.

### 2026-11-11 10:xx — xs_momentum soft patch (book −0.85%, max DD −1.98%)
Open positions net +$428 (longs $43.2k uPnL −$2,103: ZEC −$756 after its −12.6% crash, UNI −$419, BCH, ETH; shorts −$42.6k uPnL
+$2,531: SOL +$747, ADA, WLD, CRV). The book's loss is realised P&L from earlier rotations plus $154 fees. Overlap: ZEC is
a momentum long *and* the crash_rebound position, so the two books' ZEC P&L moves together. The backtest's worst drawdown was
−24% OOS, so this is well within the normal range; no action.
→ 2026-11-11 19:04 ZEC tranche 1 exit (24h after the last re-signal): bought 1,017.34, sold 988.38, **−2.85% gross ≈ −3.0% net**
(−$221 on $7.5k, i.e. −0.22% of the book). Tranche 2 (bought 973.34, ≈ +1.5% now) is open until 11-12 07:00.
Book +0.14% overall. Live crash tally: ENA +3.3% (11-05); ZEC t1 −3.0% (t2 pending). The backtest mean is +2–4%/event with a
~35–40% loss rate, so a losing tranche is expected. Note the extension mechanics: re-signals at 17:00/18:00 pushed t1's exit from
17:00 to 19:00 and raised the t2 reference; t2 then bought lower, which is the averaging-down that r30 found adds value.
→ 2026-11-12 07:04 ZEC tranche 2 exit: bought 973.34, sold 987.17, **+1.4% gross ≈ +1.3% net (+$97)**. Event #2 total:
t1 −$221 + t2 +$97 ≈ **−$127 (−0.13% of book; ≈ −0.85% on the $15k deployed)**. crash_rebound book +0.25% → +0.12%.
Live tally: 2 events, ENA +3.3%, ZEC −0.85% (blended) → both books in profit since start (+0.12% / +0.25%). Scale-in
helped: t1 alone was −3.0%, and averaging down via t2 cut the event loss to about a quarter. ZEC kept falling for about
14h after the first signal (re-signals at 17:00/18:00) before rebounding. That is the "slow crash" pattern r37 found
weaker than sharp 4h cascades.

### r62 crash-rebound: repeated signals and exit anchoring (event study, prompted by ZEC) — live rule kept
Events = first 4h < −10% hourly close per coin after ≥24h quiet (957 IS / 507 OOS), entry +1h, 0.2% round trip.
| Group | IS n / exit 24h after first / after last (live) | OOS n / first / last |
|---|---|---|
| repeated (≥2 signal bars in 6h) | 463 / +3.16% / +3.79% | 269 / +4.20% / +4.17% |
| single | 494 / +1.36% / +2.08% | 238 / +3.49% / +3.25% |
| all | +2.23% / **+2.91%** | +3.87% / **+3.74%** |
Repeated-signal (cascading) events rebound *more* on average, not less. Anchoring the exit to the last re-signal (the live
extension) improves IS by +0.7pp and is neutral OOS (−0.1pp). The live rule stays. ZEC (11-10, chain of 3 signal bars) was
a losing draw from a positive-expectancy distribution; hit rate ~0.6–0.65 means 35–40% of events lose.

### 2026-11-12 12:10 — discretionary: cut BTC 0.15 → 0.05 (review due 11-13, done early)
BTC 72.9k, 1.2% above the 72k cut rule. New information: stablecoin supply −$14.2B over 30d (largest since June 2022), i.e. liquidity
leaving; BlackRock ETF outflows $2B over 9 sessions; MSCI confirmed Strategy (MSTR) index removal at end-Nov (forced selling of
a large BTC proxy); gold −2.5%/24h; ETH 7d RR25 −6.6; F&G 25–32 with no capitulation signal. Cut to a token 5% long. Rules: reclaim
78k → 0.20; review by 11-16. The book is at −2.35% since start, mostly from the early-November long BTC/ETH into the decline.

### r42/r57 weekly update 2026-11-13 — 2,202 resolved markets (1,903 non-election); new hypothesis: near-expiry favourites OVERpriced
Pooled calibration (all hourly quotes): 65–85c now wins 78.3% vs 73.7% priced (+4.6pp ± 2.9; was +9pp). The classic favourite-longshot gap shrinks
as the sample grows. Tradeable test (r57: one quote per market 1–72h before close, median 2h; crossing the spread; event-clustered; 1,273 markets / 836 events):
buying favourites loses everywhere (0.80–0.95c: −9.6%/event, t −1.9; they win 78% vs 88% implied). Mirror trade, buy NO
against favourites (all within 6h of close; mostly in-play sports: NBA/NFL/Dota 2/football): mid 0.75–0.90 +20%/event (t 0.8),
**0.80–0.95 +57% (t 1.54, 82 events, NO at ~15c winning 22%)**, 0.85–0.97 +46% (t 1.0). That is the reverse of the classic FLB:
near-expiry in-play favourites look overpriced. NOT actionable: t < 2, lottery-like payoff, many buckets examined (multiple
testing), no time split, and in-play quotes may be stale relative to game state. Hypothesis for the ~3,000-market re-test (late Nov):
"fade in-play sports favourites priced 80–95c within 6h of close", with a pre-registered single bucket and a time split.

### 2026-11-13 01:xx — weekly r55 live vs backtest (two weeks live)
| Book | Window | live | backtest | daily corr | live costs (fees / slippage / funding) |
|---|---|---:|---:|---:|---|
| trend_breakout_majors | 10-31 → 11-13 | +5.42% | +5.72% | 0.99 | $62 / $7 / +$154 |
| xs_momentum | 11-02 → 11-13 | −1.68% | −1.50% | 0.97 | $81 / $30 / −$14 |
| xs_funding (recon funding) | 10-31 → 11-13 | +1.56% | +2.12% | 0.65 | $251 / $191 / +$100 |
xs_funding holdings now match the backtest closely (weight corr 0.91, sign agreement 82%, up from 0.84 / 67% last week). The
remaining gap is the one-off setup turnover (~$284) and edge names (LDO, OP, ETC, ARB). All three systematic books track their
backtests: live is within 0.2–0.6pp of backtest over two weeks, and execution costs are at or below assumptions. Live P&L so far is
consistent with the backtested edges. Leverage review (backlog #6) in ~2 weeks.
