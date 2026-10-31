"""Is OKX funding a good live proxy for Binance funding ranks (Binance fapi is geo-blocked)?"""
import sys, time, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.data.http import get
from ptrade.research_utils import load_panel, load_binance_funding_panel
from ptrade.universe import UNIVERSE
def okx_hist(base):
    inst = f"{base}-USDT-SWAP"; rows = []; after = None
    for _ in range(4):
        p = {"instId": inst, "limit": 100}
        if after: p["after"] = after
        d = get("https://www.okx.com/api/v5/public/funding-rate-history", params=p, retries=2).json().get("data", [])
        if not d: break
        rows += d; after = d[-1]["fundingTime"]; time.sleep(0.15)
    if not rows: return None
    s = pd.Series({pd.Timestamp(int(r["fundingTime"]), unit="ms", tz="UTC"): float(r["realizedRate"] or r["fundingRate"]) for r in rows})
    return s.sort_index()
okx = {}
for sym in UNIVERSE:
    base = sym[:-4]
    try:
        s = okx_hist(base)
        if s is not None and len(s) > 50: okx[sym] = s
    except Exception as e:
        pass
print(f"OKX swaps found for {len(okx)}/{len(UNIVERSE)} coins; history from", min(s.index[0] for s in okx.values()))
P = load_panel()
FB = load_binance_funding_panel(P, default_8h=np.nan)
FO = pd.DataFrame({k: v.reindex(P.index.floor('h')).values for k, v in okx.items()}, index=P.index)
FO = pd.DataFrame({k: v.groupby(v.index.floor('h')).sum() for k, v in okx.items()}).reindex(P.index)
start = FO.dropna(how="all").index[0]
end = FB.dropna(how="all").index[-1]
print("overlap window:", start, "->", end)
cols = [c for c in FO.columns if c in FB.columns]
tb = FB.loc[start:end, cols].fillna(0).rolling(168).sum().iloc[168:]
to = FO.loc[start:end, cols].fillna(0).rolling(168).sum().iloc[168:]
rc = tb.rank(axis=1).corrwith(to.rank(axis=1), axis=1)
print(f"rank corr of 7d trailing funding, OKX vs Binance: mean {rc.mean():.3f} median {rc.median():.3f} (n days {len(rc)//24})")
qb = tb.rank(axis=1, pct=True); qo = to.rank(axis=1, pct=True)
agree = ((qb > 0.75) & (qo > 0.75)).sum(axis=1) / (qb > 0.75).sum(axis=1)
print(f"share of Binance top-quartile names also in OKX top quartile: {agree.mean():.2f}")
