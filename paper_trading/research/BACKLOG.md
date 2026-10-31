# Research backlog (top = next)

1. **XS funding re-test on HL rates for all 40 coins** (fetcher now includes the full universe). r11-r13 done:
   DN carry retuned to 15%/5%; xs_funding paused. Also try HL funding *momentum* (reversed) on full universe.
2. Deribit DVOL: r09/r10 done (standalone rejected; market-wide crash filter promoted). Left: option-chain skew,
   DVOL as sizing input for trend books.
3. **Crash-rebound refinements**: market-wide vs idiosyncratic crashes, volume/funding filters, BTC-regime
   filter, scale-in, using 1m data for realistic real-time triggers (Binance 1m klines).
4. ~~TradFi trend on HL xyz~~ — r07 + r08 (Yahoo history incl. metals, TOM, VIX) all rejected. Revisit: weekend lead-lag (xyz trades 24/7), index seasonality.
5. ML cross-sectional v2 (r14 v1 rejected: rank-IC real but captures volatility drag): simple-return /
   vol-scaled regression target, true inverse-vol sizing, weekly horizon to cut turnover.
6. **Ensemble / risk allocator**: allocate capital across books by trailing risk-adjusted performance
   (with shrinkage), portfolio-level vol targeting and drawdown control.
7. **Open interest + funding regime** signals for BTC/ETH timing (OI spikes + extreme funding → fade).
8. **Prediction markets (Polymarket gamma API)**: systematic favourite-longshot / time-decay strategies.
9. **Pairs / cointegration** within sectors (L1s, DeFi, memes), hourly mean reversion with costs.
10. **News/LLM sentiment**: headline-shock event study (exchange hacks, ETF flows, macro prints).
11. Weekend vs weekday trend behaviour; volatility-regime switching between trend and mean reversion.
