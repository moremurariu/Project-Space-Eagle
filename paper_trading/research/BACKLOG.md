# Research backlog (top = next)

1. XS funding: DONE (r21-r22) — live on Binance funding reconstructed from 1m premium archives. Monitor live vs
   backtest; if Coinalyze key arrives, cross-check reconstruction against live Binance rates.
2. Deribit DVOL: r09/r10/r29 done (standalone + sizing rejected; market-wide crash filter promoted). Left: option skew — forward collector live since 2026-11-02 03:00
   (`deribit.collect_skew` → `state/options/skew_YYYY-MM.csv`: ATM IV, 25Δ RR/BF at ~7d/30d, put/call OI & volume
   ratios, BTC+ETH, hourly). Study RR25 extremes vs fwd returns after ~1-2 months.
3. Crash-rebound: r10/r16/r17/r28/r30 done (market-wide book; hourly-close signals; scale-in tranches promoted).
   Exit timing r63 done: keep fixed 24h after last signal; stops hurt, longer holds OOS-only, TP 8% marginal. Left: live-vs-backtest tracking once events occur.
4. ~~TradFi trend on HL xyz~~ — r07 + r08 rejected. r32: weekend single-stock overreaction on xyz → experimental
   `xyz_weekend_fade` book (kill if net < 0 after 12 weekends). 2026-11-08: liquidity cap added (≤25bp est. slippage/name);
   TODO re-run r32 with name-level costs from L2 depth snapshots (thin pre-IPO names overstated). Weekend crypto → Monday xyz lead-lag: r33 rejected (inconclusive).
5. ML cross-sectional: v1 rejected (r14), v2 marginal (r26, Sharpe 0.25). v3 needs new features (Binance funding recon,
   positioning, HL premium) once enough history exists.
6. Ensemble / allocator: r15 + r40 done (equal-weight beats inverse-vol; portfolio OOS Sharpe ~2.0 at ~10% vol).
   After ~4 weeks live: if live ≈ backtest, consider 1.5x book-level leverage. Later: dynamic allocation from LIVE
   book returns once ~2 months of data exist; portfolio-level drawdown control.
7. OI / positioning: r18 done (weak, rejected standalone). `binance.load_metrics` available as ML features.
8. **Prediction markets (Polymarket)** — forward collector live since 2026-10-31 20:05 (`ptrade/data/polymarket.py`,
   hourly near-expiry + daily top-500 snapshots → `state/polymarket/`). After ~3-4 weeks: join with resolved outcomes
   (`resolved_outcome`) → calibration by price bucket & time-to-expiry (favourite-longshot, near-certain carry).
   Trades API only exposes the latest 10k trades/market (offset cap) and CLOB history is empty after resolution.
   r53: daily BTC 'above $K' digitals vs BS (Deribit IV) — no edge on 7 dates; redo at ~30 dates with skew-adjusted
   digitals + barrier ('reach/dip') markets.
   r57: favourite-longshot as a strategy (one quote per market, 1-72h before close, crossing the spread) — NO edge on 955
   markets / 601 events; re-test by category and time-to-expiry at ~3,000 markets.
9. XS momentum: r38/r39 live (`xs_momentum`, vol-scaled). Low-risk anomalies r46: BAB promoted then retired by r47
   (residual beta, hedged OOS 0.14–0.37, not additive); low-vol / low-MAX rejected.
   ~~Pairs / cointegration~~ (r25 rejected: IS negative, OOS driven by one pair).
10. **News/LLM sentiment**: forward headline collector live since 2026-11-01 (`state/news/`, 7 feeds). In ~3-4 weeks:
    classify headlines (topic/sentiment), event-study coin/market returns after shocks (hacks, listings, ETF, macro).
11. ~~Weekend effects~~ (r24 rejected). Trend regime gate: r27 promoted ER30 gate for universe LO.
13. Wider XS universe — r51 REJECTED for xs_momentum (not robust to N, worse 2026). (scoped 2026-11-08 06:xx): HL main dex has 179 perps; 109 not yet in the universe have a Binance
    USDT spot pair (`research/universe_candidates_2026-11-08.csv`, HL volume/OI snapshot). ~30 trade ≥ $0.3M/day on HL,
    the same as the current universe's tail (ETC/ATOM/OP/SEI/LDO 0.2–0.3M on a Sunday). Most listed 2024-25, so
    test 2024+ only: fetch Binance 1h klines for the top ~30 by OI (excl. PAXG), run xs_momentum (and xs_funding if
    funding recon is feasible) on 40 vs ~70 coins over the same window, and check that the added names are not just
    noise (capacity, slippage at a 2-3% weight).
12. Vol-managed books (r48): promoted de-risk-only scaling for universe LO; r49 rejected cap-1.0 for trend_majors,
    xs_funding, xs_momentum. Left: crash books with an event-aware vol estimate (low priority).
