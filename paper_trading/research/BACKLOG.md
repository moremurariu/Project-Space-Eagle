# Research backlog (top = next)

1. XS funding: DONE (r21-r22) — live on Binance funding reconstructed from 1m premium archives. Monitor live vs
   backtest; if Coinalyze key arrives, cross-check reconstruction against live Binance rates.
2. Deribit DVOL: r09/r10/r29 done (standalone + sizing rejected; market-wide crash filter promoted). Left: option skew — forward collector live since 2026-11-02 03:00
   (`deribit.collect_skew` → `state/options/skew_YYYY-MM.csv`: ATM IV, 25Δ RR/BF at ~7d/30d, put/call OI & volume
   ratios, BTC+ETH, hourly). Study RR25 extremes vs fwd returns after ~1-2 months.
3. Crash-rebound: r10/r16/r17/r28/r30 done (market-wide book; hourly-close signals; scale-in tranches promoted).
   Left: live-vs-backtest tracking once events occur; exit timing (fixed 24h vs target/trailing).
4. ~~TradFi trend on HL xyz~~ — r07 + r08 rejected. r32: weekend single-stock overreaction on xyz → experimental
   `xyz_weekend_fade` book (kill if net < 0 after 12 weekends). Next: weekend crypto move → Monday xyz (COIN/MSTR/HOOD) lead-lag.
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
9. XS momentum: r38 live (`xs_momentum`). Next: residual (BTC-beta-hedged) momentum, vol-scaled ranking.
   ~~Pairs / cointegration~~ (r25 rejected: IS negative, OOS driven by one pair).
10. **News/LLM sentiment**: forward headline collector live since 2026-11-01 (`state/news/`, 7 feeds). In ~3-4 weeks:
    classify headlines (topic/sentiment), event-study coin/market returns after shocks (hacks, listings, ETF, macro).
11. ~~Weekend effects~~ (r24 rejected). Trend regime gate: r27 promoted ER30 gate for universe LO.
