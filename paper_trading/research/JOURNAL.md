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
