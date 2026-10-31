# Yahoo Finance daily history (one-off import)

Pulled 2026-10-31 from `query1.finance.yahoo.com/v8/finance/chart` at maximum range, daily bars.
Columns: `date,open,high,low,close,adj_close,volume`. Dates are exchange-local trading days.
Rows with no close were dropped.

| File | Symbol | What | First day | Rows |
|---|---|---|---|---|
| GC_F.csv | GC=F | Gold futures, continuous front month | 2000-08-30 | 6568 |
| SI_F.csv | SI=F | Silver futures, continuous front month | 2000-08-30 | 6569 |
| HG_F.csv | HG=F | Copper futures, continuous front month | 2000-08-30 | 6572 |
| GSPC.csv | ^GSPC | S&P 500 index | 1927-12-30 | 24826 |
| VIX.csv | ^VIX | CBOE VIX | 1990-01-02 | 9278 |
| DX-Y_NYB.csv | DX-Y.NYB | US Dollar Index | 1971-01-04 | 14176 |
| NDX.csv | ^NDX | Nasdaq-100 index | 1985-10-01 | 10351 |

Last bar in every file: 2026-10-30.

Caveats:
- `=F` series are Yahoo's continuous front-month contracts with **no roll adjustment**, so
  there are price jumps on roll dates. Treat large single-day moves near expiries with suspicion
  (or difference returns and clip at rolls) before running trend backtests.
- Index series (`^GSPC`, `^NDX`) are price indices with no dividends; `adj_close` equals `close`.
- Early `^GSPC` history has open/high/low equal to close (only closes were recorded).
- Volume is 0 or missing for indices and some early futures bars.
