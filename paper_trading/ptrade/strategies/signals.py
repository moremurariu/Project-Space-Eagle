"""Strategy signal library.

Every function maps an hourly close panel (T x N, UTC index) to a target-weight panel on the same
hourly index. Rows are NaN except at rebalance times (NaN = hold current positions). The same functions
are used for backtesting and for live paper trading (live uses the last rebalance row).
"""
from __future__ import annotations

import numpy as np
import pandas as pd


# ----------------------------------------------------------------------------- helpers

def to_daily(prices_h: pd.DataFrame, hour: int = 0) -> pd.DataFrame:
    """Daily closes sampled at `hour`:00 UTC (the close of the bar starting at hour-1)."""
    # hourly bar index is bar *open* time; close of bar opening at 23:00 is the daily close at 00:00
    closes = prices_h.copy()
    closes.index = closes.index + pd.Timedelta(hours=1)  # index by close time
    d = closes[closes.index.hour == hour]
    return d


def daily_to_hourly(w_daily: pd.DataFrame, hourly_index: pd.DatetimeIndex) -> pd.DataFrame:
    """Place daily target rows on the hourly grid (at the bar whose close equals the daily timestamp)."""
    w = w_daily.copy()
    w.index = w.index - pd.Timedelta(hours=1)  # back to bar-open convention
    return w.reindex(hourly_index)


def ewm_vol(rets: pd.DataFrame, span: int, min_periods: int | None = None) -> pd.DataFrame:
    return rets.ewm(span=span, min_periods=min_periods or span // 2).std()


def cap_gross(w: pd.DataFrame, max_gross: float) -> pd.DataFrame:
    g = w.abs().sum(axis=1)
    scale = (max_gross / g).clip(upper=1.0).fillna(1.0)
    return w.mul(scale, axis=0)


def portfolio_vol_target(w: pd.DataFrame, rets: pd.DataFrame, target: float, periods: int, span: int = 60,
                         max_lev: float = 3.0) -> pd.DataFrame:
    """Scale a weight panel so the *ex-ante* portfolio vol (rolling cov) hits `target` (annualised)."""
    port = (w.shift(1) * rets).sum(axis=1)  # realised returns of the unscaled book
    pv = port.ewm(span=span, min_periods=span // 2).std() * np.sqrt(periods)
    scale = (target / pv).clip(upper=max_lev).shift(0).fillna(0.0)
    return w.mul(scale, axis=0)


# ----------------------------------------------------------------------------- trend (time-series momentum)

def ewmac_forecast(daily: pd.DataFrame, fast: int, slow: int, vol_span: int = 36) -> pd.DataFrame:
    """Carver-style EWMA crossover, normalised by price volatility; raw (unscaled) forecast."""
    fe = daily.ewm(span=fast, min_periods=fast).mean()
    se = daily.ewm(span=slow, min_periods=slow).mean()
    pvol = daily.diff().ewm(span=vol_span, min_periods=vol_span // 2).std()
    return (fe - se) / pvol


def trend_ewmac(prices_h: pd.DataFrame, speeds=((8, 32), (16, 64), (32, 128), (64, 256)),
                asset_vol_target: float = 0.5, port_vol_target: float | None = 0.4, long_only: bool = False,
                cap: float = 2.0, hour: int = 0, max_gross: float = 3.0, max_w: float = 1.0,
                scalar_window: int = 500) -> pd.DataFrame:
    """Multi-speed trend following, inverse-vol sized, optional portfolio vol targeting. Daily rebalance."""
    d = to_daily(prices_h, hour)
    rets = d.pct_change(fill_method=None)
    fc_sum = None
    for fast, slow in speeds:
        f = ewmac_forecast(d, fast, slow)
        # scale each speed so its avg |forecast| ~ 1 (expanding, no look-ahead)
        denom = f.abs().expanding(min_periods=scalar_window // 5).mean().mean(axis=1)
        f = f.div(denom, axis=0)
        fc_sum = f if fc_sum is None else fc_sum.add(f, fill_value=0)
    fc = (fc_sum / len(speeds)).clip(-cap, cap)
    if long_only:
        fc = fc.clip(lower=0)
    avol = ewm_vol(rets, 36) * np.sqrt(365)
    w = fc * (asset_vol_target / avol)
    n_active = w.notna().sum(axis=1).replace(0, np.nan)
    w = w.div(n_active, axis=0).fillna(0.0)
    w = w.clip(-max_w, max_w)
    if port_vol_target:
        w = portfolio_vol_target(w, rets.fillna(0), port_vol_target, 365, span=60)
    w = cap_gross(w, max_gross)
    return daily_to_hourly(w, prices_h.index)


def trend_breakout(prices_h: pd.DataFrame, lookbacks=(20, 55, 120), asset_vol_target: float = 0.5,
                   port_vol_target: float | None = 0.4, long_only: bool = False, hour: int = 0,
                   max_gross: float = 3.0) -> pd.DataFrame:
    """Donchian-channel position: (close - mid)/(range/2) averaged over lookbacks, smoothed."""
    d = to_daily(prices_h, hour)
    rets = d.pct_change(fill_method=None)
    sigs = []
    for lb in lookbacks:
        hi = d.rolling(lb, min_periods=lb).max()
        lo = d.rolling(lb, min_periods=lb).min()
        mid = (hi + lo) / 2
        s = ((d - mid) / ((hi - lo) / 2)).ewm(span=max(2, lb // 4)).mean()
        sigs.append(s)
    fc = sum(sigs) / len(sigs)
    if long_only:
        fc = fc.clip(lower=0)
    avol = ewm_vol(rets, 36) * np.sqrt(365)
    w = fc * (asset_vol_target / avol)
    n_active = w.notna().sum(axis=1).replace(0, np.nan)
    w = w.div(n_active, axis=0).fillna(0.0)
    if port_vol_target:
        w = portfolio_vol_target(w, rets.fillna(0), port_vol_target, 365, span=60)
    w = cap_gross(w, max_gross)
    return daily_to_hourly(w, prices_h.index)


# ----------------------------------------------------------------------------- cross-sectional

def xs_momentum(prices_h: pd.DataFrame, lookback_d: int = 30, skip_d: int = 0, rebalance_d: int = 7,
                quantile: float = 0.25, gross: float = 1.0, min_assets: int = 8, hour: int = 0,
                inv_vol: bool = True, reverse: bool = False, offset_d: int = 0) -> pd.DataFrame:
    """Long top-quantile / short bottom-quantile by past return (reverse=True -> reversal)."""
    d = to_daily(prices_h, hour)
    rets = d.pct_change(fill_method=None)
    past = d.shift(skip_d) / d.shift(skip_d + lookback_d) - 1
    if reverse:
        past = -past
    vol = ewm_vol(rets, 30)
    out = pd.DataFrame(np.nan, index=d.index, columns=d.columns)
    for i, t in enumerate(d.index):
        if (i - offset_d) % rebalance_d:
            continue
        s = past.loc[t].dropna()
        if len(s) < min_assets:
            out.loc[t] = 0.0
            continue
        k = max(1, int(round(len(s) * quantile)))
        r = s.sort_values()
        longs, shorts = r.index[-k:], r.index[:k]
        row = pd.Series(0.0, index=d.columns)
        if inv_vol:
            iv = 1 / vol.loc[t]
            lw = iv[longs] / iv[longs].sum()
            sw = iv[shorts] / iv[shorts].sum()
        else:
            lw = pd.Series(1 / k, index=longs)
            sw = pd.Series(1 / k, index=shorts)
        row[longs] = lw * gross / 2
        row[shorts] = -sw * gross / 2
        out.loc[t] = row.fillna(0.0)
    return daily_to_hourly(out, prices_h.index)


def xs_funding_carry(prices_h: pd.DataFrame, funding_h: pd.DataFrame, lookback_h: int = 72,
                     rebalance_h: int = 24, quantile: float = 0.25, gross: float = 1.0,
                     min_assets: int = 8, inv_vol: bool = True) -> pd.DataFrame:
    """Short the highest-funding perps, long the lowest (collect carry + fade crowding)."""
    fund = funding_h.reindex(index=prices_h.index, columns=prices_h.columns)
    avg_f = fund.rolling(lookback_h, min_periods=lookback_h // 2).sum()
    rets = prices_h.pct_change(fill_method=None)
    vol = rets.rolling(24 * 30, min_periods=24 * 10).std()
    out = pd.DataFrame(np.nan, index=prices_h.index, columns=prices_h.columns)
    idx = np.arange(len(prices_h.index))
    for i in idx[(prices_h.index.hour == 0)]:
        if (i // 24) % max(1, rebalance_h // 24):
            continue
        t = prices_h.index[i]
        s = avg_f.loc[t].dropna()
        s = s[prices_h.loc[t, s.index].notna()]
        if len(s) < min_assets:
            out.loc[t] = 0.0
            continue
        k = max(1, int(round(len(s) * quantile)))
        r = s.sort_values()
        longs, shorts = r.index[:k], r.index[-k:]
        row = pd.Series(0.0, index=prices_h.columns)
        if inv_vol:
            iv = 1 / vol.loc[t]
            lw = iv[longs] / iv[longs].sum()
            sw = iv[shorts] / iv[shorts].sum()
        else:
            lw = pd.Series(1 / k, index=longs)
            sw = pd.Series(1 / k, index=shorts)
        row[longs] = lw * gross / 2
        row[shorts] = -sw * gross / 2
        out.loc[t] = row.fillna(0.0)
    return out


# ----------------------------------------------------------------------------- mean reversion / pairs

def pair_zscore(prices_h: pd.DataFrame, a: str, b: str, window_h: int = 24 * 20, entry: float = 2.0,
                exit_: float = 0.5, gross: float = 1.0, beta_window_h: int | None = None) -> pd.DataFrame:
    """Log-ratio z-score mean reversion between two assets (hourly decisions)."""
    la, lb = np.log(prices_h[a]), np.log(prices_h[b])
    if beta_window_h:
        cov = la.diff().rolling(beta_window_h).cov(lb.diff())
        var = lb.diff().rolling(beta_window_h).var()
        beta = (cov / var).clip(0.2, 3)
    else:
        beta = pd.Series(1.0, index=prices_h.index)
    spread = la - beta * lb
    z = (spread - spread.rolling(window_h).mean()) / spread.rolling(window_h).std()
    pos = np.zeros(len(z))
    zz = z.to_numpy()
    cur = 0.0
    for i, v in enumerate(zz):
        if not np.isfinite(v):
            pos[i] = 0.0
            continue
        if cur == 0:
            if v > entry:
                cur = -1.0
            elif v < -entry:
                cur = 1.0
        elif cur > 0 and v > -exit_:
            cur = 0.0
        elif cur < 0 and v < exit_:
            cur = 0.0
        pos[i] = cur
    pos = pd.Series(pos, index=prices_h.index)
    w = pd.DataFrame(0.0, index=prices_h.index, columns=prices_h.columns)
    w[a] = pos * gross / 2
    w[b] = -pos * gross / 2 * beta.fillna(1.0)
    # only emit rows when the position changes (NaN otherwise -> hold, lets weights drift)
    chg = pos.diff().fillna(pos) != 0
    return w.where(chg, np.nan)


def intraday_reversal(prices_h: pd.DataFrame, lookback_h: int = 24, rebalance_h: int = 24,
                      quantile: float = 0.2, gross: float = 1.0, min_assets: int = 8) -> pd.DataFrame:
    rets = prices_h / prices_h.shift(lookback_h) - 1
    out = pd.DataFrame(np.nan, index=prices_h.index, columns=prices_h.columns)
    for i in range(lookback_h, len(prices_h), rebalance_h):
        s = rets.iloc[i].dropna()
        if len(s) < min_assets:
            continue
        k = max(1, int(round(len(s) * quantile)))
        r = s.sort_values()
        row = pd.Series(0.0, index=prices_h.columns)
        row[r.index[:k]] = gross / 2 / k
        row[r.index[-k:]] = -gross / 2 / k
        out.iloc[i] = row
    return out


# ----------------------------------------------------------------------------- daily-data trend (TradFi)

def trend_daily(d: pd.DataFrame, periods: int = 252, lookbacks=(20, 55, 120), speeds=((16, 64), (32, 128), (64, 256)),
                asset_vol_target: float = 0.15, port_vol_target: float | None = 0.12, long_only: bool = False,
                max_gross: float = 4.0, max_w: float = 1.5, blend: float = 0.5) -> pd.DataFrame:
    """Daily trend weights from daily closes: blend of Donchian breakout and EWMAC forecasts (each ~[-1, 1]),
    inverse-vol sized to `asset_vol_target`, equal risk across assets, optional portfolio vol target."""
    rets = d.pct_change(fill_method=None)
    sigs = []
    for lb in lookbacks:
        hi = d.rolling(lb, min_periods=lb).max()
        lo = d.rolling(lb, min_periods=lb).min()
        sigs.append(((d - (hi + lo) / 2) / ((hi - lo) / 2)).ewm(span=max(2, lb // 4)).mean())
    brk = sum(sigs) / len(sigs)
    ew = None
    for fast, slow in speeds:
        f = ewmac_forecast(d, fast, slow)
        f = f.div(f.abs().expanding(min_periods=100).mean(), axis=0)
        ew = f if ew is None else ew.add(f, fill_value=0)
    ew = (ew / len(speeds)).clip(-2, 2) / 2
    fc = blend * brk + (1 - blend) * ew
    if long_only:
        fc = fc.clip(lower=0)
    avol = ewm_vol(rets, 36) * np.sqrt(periods)
    w = fc * (asset_vol_target / avol)
    n_active = w.notna().sum(axis=1).replace(0, np.nan)
    w = w.div(n_active, axis=0).mul(np.sqrt(n_active), axis=0).fillna(0.0)  # diversification-aware
    w = w.clip(-max_w, max_w)
    if port_vol_target:
        w = portfolio_vol_target(w, rets.fillna(0), port_vol_target, periods, span=60, max_lev=max_gross)
    return cap_gross(w, max_gross)
