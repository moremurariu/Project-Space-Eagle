"""Compact market brief for hourly reviews / discretionary decisions.

    python -m ptrade.tools_brief
"""
from __future__ import annotations

import pandas as pd

from .data import hyperliquid as hl
from .data import sentiment


def main():
    m = hl.meta_and_ctxs()
    m = m[~m.delisted]
    m["chg24"] = m["mark"] / m["prev_day_px"] - 1
    m["apr"] = m["funding"] * 8760 * 100
    print("== Majors")
    for c in ["BTC", "ETH", "SOL", "HYPE"]:
        r = m.loc[c]
        print(f"  {c:5s} {r.mark:>10.4g}  24h {r.chg24*100:+5.1f}%  funding {r.apr:+6.1f}% APR  OI ${r.oi_usd/1e6:,.0f}m")
    liq = m[m.day_ntl_vlm > 2e6]
    print("== Biggest 24h movers (vol > $2m)")
    for c, r in pd.concat([liq.nsmallest(4, "chg24"), liq.nlargest(4, "chg24")]).iterrows():
        print(f"  {c:8s} {r.chg24*100:+6.1f}%  funding {r.apr:+7.1f}% APR")
    print("== Funding extremes (OI > $5m)")
    big = m[m.oi_usd > 5e6]
    for c, r in pd.concat([big.nlargest(5, "apr"), big.nsmallest(5, "apr")]).iterrows():
        print(f"  {c:8s} {r.apr:+7.1f}% APR  24h {r.chg24*100:+5.1f}%")
    try:
        x = hl.meta_and_ctxs("xyz")
        x["chg24"] = x["mark"] / x["prev_day_px"] - 1
        print("== TradFi perps (HL xyz)")
        for c in ["xyz:SP500", "xyz:XYZ100", "xyz:GOLD", "xyz:CL", "xyz:EUR", "xyz:DXY"]:
            if c in x.index:
                r = x.loc[c]
                print(f"  {c:10s} {r.mark:>10.4g}  24h {r.chg24*100:+5.2f}%")
    except Exception as e:  # noqa: BLE001
        print("xyz failed", e)
    try:
        fng = sentiment.fear_greed(7)
        print("== Fear & Greed (7d):", " ".join(str(v) for v in fng.values))
    except Exception as e:  # noqa: BLE001
        print("fng failed", e)
    print("== Headlines")
    for h in sentiment.headlines(12):
        print(f"  - {h['title']} ({h['published'][:22]})")


if __name__ == "__main__":
    main()
