# Research backlog (top = next)

1. **HL-native funding backtests**: once `research/fetch_hl_funding.py` has cached HL funding, backtest
   DN carry and XS funding on actual HL rates (incl. premium/basis P&L) and tune thresholds.
2. **Crash-rebound refinements**: market-wide vs idiosyncratic crashes, volume/funding filters, BTC-regime
   filter, scale-in, using 1m data for realistic real-time triggers (Binance 1m klines).
3. ~~TradFi trend on HL xyz~~ — tested r07, rejected (Sharpe ~0). Revisit: weekend lead-lag (xyz trades 24/7), index seasonality.
   DXY using long daily history (FRED / other sources) — classic managed-futures diversification.
4. **ML cross-sectional model** (LightGBM, walk-forward): features = multi-horizon returns, vol, funding,
   volume z-scores, BTC beta; target = next 24h residual return. Strict purged walk-forward.
5. **Ensemble / risk allocator**: allocate capital across books by trailing risk-adjusted performance
   (with shrinkage), portfolio-level vol targeting and drawdown control.
6. **Open interest + funding regime** signals for BTC/ETH timing (OI spikes + extreme funding → fade).
7. **Prediction markets (Polymarket gamma API)**: systematic favourite-longshot / time-decay strategies.
8. **Pairs / cointegration** within sectors (L1s, DeFi, memes), hourly mean reversion with costs.
9. **News/LLM sentiment**: headline-shock event study (exchange hacks, ETF flows, macro prints).
10. Weekend vs weekday trend behaviour; volatility-regime switching between trend and mean reversion.
