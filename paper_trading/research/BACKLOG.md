# Research backlog (top = next)

1. **HL-native funding backtests**: once `research/fetch_hl_funding.py` has cached HL funding, backtest
   DN carry and XS funding on actual HL rates (incl. premium/basis P&L) and tune thresholds.
2. **Deribit implied-vol signals** (new data, `ptrade/data/deribit.py`): DVOL hourly since 2021 → vol-risk-premium
   (DVOL vs realised) as regime filter for trend / crash-rebound; DVOL spikes as timing; skew from option chain.
3. **Crash-rebound refinements**: market-wide vs idiosyncratic crashes, volume/funding filters, BTC-regime
   filter, scale-in, using 1m data for realistic real-time triggers (Binance 1m klines).
4. ~~TradFi trend on HL xyz~~ — r07 + r08 (Yahoo history incl. metals, TOM, VIX) all rejected. Revisit: weekend lead-lag (xyz trades 24/7), index seasonality.
5. **ML cross-sectional model** (LightGBM, walk-forward): features = multi-horizon returns, vol, funding,
   volume z-scores, BTC beta; target = next 24h residual return. Strict purged walk-forward.
6. **Ensemble / risk allocator**: allocate capital across books by trailing risk-adjusted performance
   (with shrinkage), portfolio-level vol targeting and drawdown control.
7. **Open interest + funding regime** signals for BTC/ETH timing (OI spikes + extreme funding → fade).
8. **Prediction markets (Polymarket gamma API)**: systematic favourite-longshot / time-decay strategies.
9. **Pairs / cointegration** within sectors (L1s, DeFi, memes), hourly mean reversion with costs.
10. **News/LLM sentiment**: headline-shock event study (exchange hacks, ETF flows, macro prints).
11. Weekend vs weekday trend behaviour; volatility-regime switching between trend and mean reversion.
