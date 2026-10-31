"""Event-accurate (position-drift) portfolio backtester driven by target weights.

Conventions
- `prices`: DataFrame (T x N) of close prices on a regular (hourly) index.
- `weights`: DataFrame (T x N) of target weights (fraction of equity, signed) decided using data up to
  and including bar t, executed at bar t close (+ slippage + fees). NaN = no opinion (keep position).
- `funding`: optional DataFrame (T x N) of per-bar funding rates for perps; a long position pays
  `rate * notional` at each bar (shorts receive).
- Positions drift with prices between rebalances (unit-based simulation), so turnover and costs are
  realistic. A `min_trade` buffer (in weight units) suppresses tiny rebalances.
"""
from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import pandas as pd


@dataclass
class BTResult:
    equity: pd.Series
    returns: pd.Series
    weights: pd.DataFrame  # realised (drifted) weights at bar close after trading
    turnover: pd.Series    # traded notional / equity per bar
    costs: pd.Series       # fees + slippage (fraction of equity) per bar
    funding: pd.Series     # funding PnL (fraction of equity) per bar

    def stats(self, periods_per_year: float = 8760.0) -> dict:
        return perf_stats(self.returns, periods_per_year, turnover=self.turnover, weights=self.weights)


def run(prices: pd.DataFrame, weights: pd.DataFrame, fee: float = 0.00045, slippage: float = 0.0003,
        funding: pd.DataFrame | None = None, min_trade: float = 0.0, lag: int = 0,
        initial: float = 1.0) -> BTResult:
    prices = prices.sort_index()
    weights = weights.reindex(index=prices.index, columns=prices.columns)
    if lag:
        weights = weights.shift(lag)
    P = prices.to_numpy(dtype=float)
    W = weights.to_numpy(dtype=float)
    F = None
    if funding is not None:
        F = funding.reindex(index=prices.index, columns=prices.columns).fillna(0.0).to_numpy(dtype=float)
    T, N = P.shape
    units = np.zeros(N)
    eq = initial
    eq_arr = np.empty(T)
    wt_arr = np.zeros((T, N))
    to_arr = np.zeros(T)
    cost_arr = np.zeros(T)
    fund_arr = np.zeros(T)
    prev_p = np.where(np.isfinite(P[0]), P[0], 0.0)
    c = fee + slippage
    for t in range(T):
        p = P[t]
        ok = np.isfinite(p)
        p_safe = np.where(ok, p, prev_p)
        # mark to market
        eq += float(np.dot(units, p_safe - prev_p))
        if F is not None:
            f = float(-np.dot(units * p_safe, F[t]))
            eq += f
            fund_arr[t] = f / max(eq, 1e-12)
        # rebalance
        w = W[t]
        if np.any(np.isfinite(w)) and eq > 0:
            cur_w = units * p_safe / eq
            tgt_w = np.where(np.isfinite(w), w, cur_w)
            tgt_w = np.where(ok, tgt_w, cur_w)  # cannot trade assets without a price
            dw = tgt_w - cur_w
            if min_trade > 0:
                # only trade names whose deviation exceeds the buffer, or that must go to zero
                trade_mask = (np.abs(dw) > min_trade) | ((tgt_w == 0) & (cur_w != 0))
                dw = np.where(trade_mask, dw, 0.0)
            traded = float(np.abs(dw).sum())
            if traded > 0:
                cost = traded * c * eq
                new_units = units + dw * eq / np.where(p_safe > 0, p_safe, 1.0)
                units = np.where(ok, new_units, units)
                eq -= cost
                to_arr[t] = traded
                cost_arr[t] = cost / max(eq, 1e-12)
        eq_arr[t] = eq
        wt_arr[t] = units * p_safe / eq if eq > 0 else 0.0
        prev_p = p_safe
        if eq <= 0:
            eq_arr[t:] = 0
            break
    equity = pd.Series(eq_arr, index=prices.index)
    rets = equity.pct_change().fillna(0.0)
    return BTResult(equity=equity, returns=rets,
                    weights=pd.DataFrame(wt_arr, index=prices.index, columns=prices.columns),
                    turnover=pd.Series(to_arr, index=prices.index),
                    costs=pd.Series(cost_arr, index=prices.index),
                    funding=pd.Series(fund_arr, index=prices.index))


def perf_stats(rets: pd.Series, periods_per_year: float = 8760.0, turnover: pd.Series | None = None,
               weights: pd.DataFrame | None = None) -> dict:
    rets = rets.dropna()
    if len(rets) < 2:
        return {}
    eq = (1 + rets).cumprod()
    years = len(rets) / periods_per_year
    total = eq.iloc[-1]
    cagr = total ** (1 / years) - 1 if total > 0 and years > 0 else -1.0
    vol = rets.std() * np.sqrt(periods_per_year)
    sharpe = rets.mean() / rets.std() * np.sqrt(periods_per_year) if rets.std() > 0 else 0.0
    downside = rets[rets < 0].std() * np.sqrt(periods_per_year)
    sortino = rets.mean() * periods_per_year / downside if downside > 0 else 0.0
    dd = eq / eq.cummax() - 1
    mdd = dd.min()
    out = {
        "cagr": cagr, "vol": vol, "sharpe": sharpe, "sortino": sortino, "max_dd": mdd,
        "calmar": cagr / abs(mdd) if mdd < 0 else np.nan, "total_return": total - 1, "years": years,
    }
    if turnover is not None:
        out["turnover_per_year"] = turnover.sum() / years
    if weights is not None:
        out["avg_gross"] = weights.abs().sum(axis=1).mean()
        out["avg_net"] = weights.sum(axis=1).mean()
    return out


def yearly_returns(rets: pd.Series) -> pd.Series:
    return (1 + rets).groupby(rets.index.year).prod() - 1


def fmt_stats(d: dict) -> str:
    keys = ["cagr", "vol", "sharpe", "sortino", "max_dd", "calmar", "turnover_per_year", "avg_gross", "avg_net"]
    parts = []
    for k in keys:
        if k in d and d[k] is not None and not (isinstance(d[k], float) and np.isnan(d[k])):
            v = d[k]
            if k in ("cagr", "vol", "max_dd"):
                parts.append(f"{k}={v*100:6.1f}%")
            else:
                parts.append(f"{k}={v:6.2f}")
    return " ".join(parts)
