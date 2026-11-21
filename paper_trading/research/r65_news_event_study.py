"""r65: first-pass news event study on the forward headline dataset (state/news/, since 2026-11-01).

Keyword-tag headlines by category (hack, listing, ETF, regulatory, liquidation, unlock, whale), map them to
universe coins by name/ticker, then measure the coin's BTC-adjusted forward return from the first hour the
headline was seen (signal at the close of the hour it was first seen; no look-ahead).
Small sample (~3 weeks): descriptive only, no promotion.
"""
from __future__ import annotations

import re
import sys

import numpy as np
import pandas as pd

sys.path.insert(0, ".")
from ptrade.config import STATE  # noqa: E402
from ptrade.research_utils import load_panel  # noqa: E402

NAMES = {
    "BTC": ["bitcoin", "btc"], "ETH": ["ethereum", "ether", "eth"], "SOL": ["solana", "sol"], "XRP": ["xrp", "ripple"],
    "BNB": ["bnb", "binance coin"], "DOGE": ["dogecoin", "doge"], "ADA": ["cardano", "ada"], "AVAX": ["avalanche", "avax"],
    "LINK": ["chainlink"], "DOT": ["polkadot", "dot"], "LTC": ["litecoin", "ltc"], "BCH": ["bitcoin cash", "bch"],
    "TRX": ["tron", "trx"], "NEAR": ["near protocol"], "ATOM": ["cosmos", "atom"], "UNI": ["uniswap"],
    "AAVE": ["aave"], "FIL": ["filecoin", "fil"], "ARB": ["arbitrum", "arb"], "OP": ["optimism"], "SUI": ["sui"],
    "APT": ["aptos"], "INJ": ["injective", "inj"], "TIA": ["celestia", "tia"], "SEI": ["sei"],
    "WLD": ["worldcoin", "wld"], "ENA": ["ethena", "ena", "usde"], "PEPE": ["pepe"],
    "XLM": ["stellar", "xlm"], "ETC": ["ethereum classic", "etc"], "HBAR": ["hedera", "hbar"], "TAO": ["bittensor", "tao"],
    "ZEC": ["zcash", "zec"], "CRV": ["curve finance", "crv"], "LDO": ["lido", "ldo"], "ONDO": ["ondo"], "JUP": ["jupiter", "jup"],
    "PENDLE": ["pendle"], "RENDER": ["render"], "ALGO": ["algorand"],
}
CATS = {
    "hack": r"\b(hack|hacked|hacker|exploit|exploited|drain|drained|stolen|attack|breach|phish)",
    "listing": r"\b(lists|listing|listed|to list|delist)",
    "etf": r"\betf",
    "regulatory": r"\b(sec|cftc|lawsuit|sue[sd]?|charged|ban|regulat|court|doj)\b",
    "liquidation": r"\b(liquidat)",
    "unlock": r"\b(unlock|token release)",
    "whale": r"\b(whale|treasury|buys \$|bought \$|accumulat)",
}
H = [1, 4, 24]


def tag(title: str) -> tuple[list[str], list[str]]:
    t = " " + title.lower() + " "
    coins = []
    for c, keys in NAMES.items():
        for k in keys:
            if k and re.search(rf"(?<![a-z0-9]){re.escape(k)}(?![a-z0-9])", t):
                coins.append(c)
                break
    if "BCH" in coins or "ETC" in coins:  # "bitcoin cash"/"ethereum classic" also match BTC/ETH
        coins = [c for c in coins if not (c == "BTC" and "BCH" in coins) and not (c == "ETH" and "ETC" in coins)]
    cats = [k for k, rx in CATS.items() if re.search(rx, t)]
    return coins, cats


def main():
    df = pd.concat([pd.read_csv(f) for f in sorted((STATE / "news").glob("headlines_*.csv"))])
    df["seen_at"] = pd.to_datetime(df["seen_at"], utc=True, format="ISO8601")
    df["key"] = df["title"].str.lower().str.replace(r"[^a-z0-9 ]", "", regex=True).str.strip()
    df = df.sort_values("seen_at").drop_duplicates("key")           # first sighting across feeds
    df[["coins", "cats"]] = df["title"].apply(lambda s: pd.Series(tag(str(s))))
    print(f"{len(df)} unique headlines {df.seen_at.min():%m-%d} -> {df.seen_at.max():%m-%d}")

    px = load_panel()
    px.columns = [c.removesuffix("USDT") for c in px.columns]
    lr = np.log(px)
    rows = []
    for _, r in df.iterrows():
        t0 = r["seen_at"].floor("h") + pd.Timedelta(hours=1)          # close of the hour first seen
        if t0 not in lr.index:
            continue
        for c in r["coins"]:
            if c not in lr.columns:
                continue
            out = {"t0": t0, "coin": c, "cats": r["cats"], "title": r["title"]}
            for h in H:
                t1 = t0 + pd.Timedelta(hours=h)
                if t1 > lr.index[-1]:
                    out[f"r{h}"] = np.nan
                    continue
                raw = lr.at[t1, c] - lr.at[t0, c]
                mkt = lr.at[t1, "BTC"] - lr.at[t0, "BTC"]
                out[f"r{h}"] = raw - (mkt if c != "BTC" else 0.0)
                out[f"raw{h}"] = raw
            rows.append(out)
    ev = pd.DataFrame(rows)
    print(f"{len(ev)} coin-headline events; coins mentioned: {ev.coin.value_counts().head(8).to_dict()}")

    # baseline: all non-BTC coin-hours (unconditional BTC-adjusted fwd return)
    alt = ev[ev.coin != "BTC"]
    print("\nBTC-adjusted forward returns of mentioned alts (bp), by category; day-clustered t on r24")
    print(f"{'cat':12s} {'n':>4s} {'days':>4s} {'r1':>7s} {'r4':>7s} {'r24':>7s} {'t24':>6s}")
    for cat in ["(any)", "(none)"] + list(CATS):
        if cat == "(any)":
            s = alt
        elif cat == "(none)":
            s = alt[alt.cats.str.len() == 0]
        else:
            s = alt[alt.cats.apply(lambda x: cat in x)]
        if len(s) < 3:
            print(f"{cat:12s} {len(s):4d}")
            continue
        s = s.drop_duplicates(["coin", "t0"])
        d = s.dropna(subset=["r24"]).groupby(s.t0.dt.floor("D"))["r24"].mean()
        t = d.mean() / (d.std(ddof=1) / np.sqrt(len(d))) if len(d) > 2 else np.nan
        print(f"{cat:12s} {len(s):4d} {len(d):4d} {s.r1.mean()*1e4:7.0f} {s.r4.mean()*1e4:7.0f} {s.r24.mean()*1e4:7.0f} {t:6.2f}")
    hk = alt[alt.cats.apply(lambda x: "hack" in x)].drop_duplicates(["coin", "t0"])
    if len(hk):
        print("\nhack-tagged alt events:")
        for _, r in hk.iterrows():
            print(f"  {r.t0:%m-%d %H} {r.coin:6s} r4 {r.r4*1e4:6.0f}bp r24 {r.r24*1e4:6.0f}bp  {r.title[:80]}")


if __name__ == "__main__":
    main()
