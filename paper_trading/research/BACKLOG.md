# Research backlog (top = next)

1. XS funding: DONE (r21-r22) — live on Binance funding reconstructed from 1m premium archives. Monitor live vs
   backtest; if Coinalyze key arrives, cross-check reconstruction against live Binance rates.
2. Deribit DVOL: r09/r10 done (standalone rejected; market-wide crash filter promoted). Left: option-chain skew,
   DVOL as sizing input for trend books.
3. **Crash-rebound refinements**: market-wide vs idiosyncratic crashes, volume/funding filters, BTC-regime
   filter, scale-in, using 1m data for realistic real-time triggers (Binance 1m klines).
4. ~~TradFi trend on HL xyz~~ — r07 + r08 (Yahoo history incl. metals, TOM, VIX) all rejected. Revisit: weekend lead-lag (xyz trades 24/7), index seasonality.
5. ML cross-sectional: v1 rejected (r14), v2 marginal (r26, Sharpe 0.25). v3 needs new features (Binance funding recon,
   positioning, HL premium) once enough history exists.
6. Ensemble / allocator: r15 done (equal-weight + risk balancing adopted). Later: dynamic allocation from LIVE
   book returns once ~2 months of data exist; portfolio-level drawdown control.
7. OI / positioning: r18 done (weak, rejected standalone). `binance.load_metrics` available as ML features.
8. **Prediction markets (Polymarket)** — forward collector live since 2026-10-31 20:05 (`ptrade/data/polymarket.py`,
   hourly near-expiry + daily top-500 snapshots → `state/polymarket/`). After ~3-4 weeks: join with resolved outcomes
   (`resolved_outcome`) → calibration by price bucket & time-to-expiry (favourite-longshot, near-certain carry).
   Trades API only exposes the latest 10k trades/market (offset cap) and CLOB history is empty after resolution.
9. ~~Pairs / cointegration~~ (r25 rejected: IS negative, OOS driven by one pair).
10. **News/LLM sentiment**: headline-shock event study (exchange hacks, ETF flows, macro prints).
11. ~~Weekend effects~~ (r24 rejected). Trend regime gate: r27 promoted ER30 gate for universe LO.
