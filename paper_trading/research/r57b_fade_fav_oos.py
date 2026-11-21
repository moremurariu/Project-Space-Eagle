"""r57b: pre-registered out-of-sample re-test of the r57 hypothesis (journal 2026-11-13):
"fade near-expiry favourites: buy NO at (1 - best_bid) when mid is 0.80-0.95 and the quote is <=6h before close".

Exactly the r57 procedure: per resolved non-election market, take the latest quote 1-72h before close; trade only if that
quote is <=6h before close and its mid is 0.80-0.95. Event-clustered (close date + question stem).
Time split: IS = markets closed before 2026-11-13 00:00 UTC (the data r57 saw); OOS = closed from 2026-11-14 00:00 UTC
(never seen). Nov 13 is a gap. The pre-registered verdict uses the OOS bucket on all non-election markets; the sports-only
subset is descriptive.
"""
import glob
import sys
import warnings

import numpy as np
import pandas as pd

sys.path.insert(0, ".")
warnings.filterwarnings("ignore")

IS_END = pd.Timestamp("2026-11-13", tz="UTC")
OOS_START = pd.Timestamp("2026-11-14", tz="UTC")
SPORTS = r"\bvs\.?\b| v | x |NBA|NFL|NHL|MLB|UFC|Dota|LoL|CS2|Counter-Strike|Valorant|FC\b|United|City|Premier|La Liga|Serie A|Bundesliga|Champions|Open|ATP|WTA|win on 20"


def main():
    o = pd.read_csv("state/polymarket/outcomes.csv")
    o["closed_time"] = pd.to_datetime(o.closed_time, utc=True, errors="coerce")
    D = pd.concat([pd.read_csv(f) for f in sorted(glob.glob("state/polymarket/snap_*.csv.gz"))], ignore_index=True)
    D["ts"] = pd.to_datetime(D.ts, utc=True)
    D = D.merge(o, on="id", how="inner")
    elec = D.question.astype(str).str.contains(
        r"Republican|Democrat|election|Senate|House|Governor|mayor|primary|electoral|seats", case=False)
    D = D[~elec & D.best_bid.notna() & D.best_ask.notna() & (D.best_ask > D.best_bid) & D.closed_time.notna()]
    D["hrs"] = (D.closed_time - D.ts).dt.total_seconds() / 3600
    D = D[(D.hrs >= 1) & (D.hrs <= 72)]
    q = D.sort_values("ts").groupby("id").tail(1).copy()
    q["mid"] = (q.best_bid + q.best_ask) / 2
    q["event"] = q.closed_time.dt.strftime("%Y-%m-%d") + "|" + q.question.astype(str).str.replace(
        r"\d[\d,.$%]*", "#", regex=True).str[:40]
    q["sports"] = q.question.astype(str).str.contains(SPORTS, case=False, regex=True)
    q["px"] = 1 - q.best_bid
    q["pay"] = 1 - q.yes_won
    q["ret"] = q.pay / q.px - 1
    sel = q.mid.between(0.80, 0.95) & (q.hrs <= 6)
    print(f"markets {len(q)} (IS {int((q.closed_time < IS_END).sum())}, OOS {int((q.closed_time >= OOS_START).sum())})")
    for lab, per in (("IS", q.closed_time < IS_END), ("OOS", q.closed_time >= OOS_START)):
        for sub, m in (("all", True), ("sports", q.sports)):
            t = q[sel & per & m]
            if not len(t):
                print(f"{lab:3s} {sub:6s} none")
                continue
            ev = t.groupby("event").ret.mean()
            tt = ev.mean() / ev.std() * np.sqrt(len(ev)) if len(ev) > 2 else np.nan
            print(f"{lab:3s} {sub:6s} n {len(t):4d} events {len(ev):4d} | avg NO px {t.px.mean():.3f} implied {1 - t.mid.mean():.1%} "
                  f"NO won {t.pay.mean():.1%} | ret/$ {t.ret.mean():+.1%} | event-clustered {ev.mean():+.1%} t {tt:+.2f}")
    t = q[sel & (q.closed_time >= OOS_START)]
    print("\nOOS markets sample:")
    for _, r in t.sort_values("closed_time").head(25).iterrows():
        print(f"  {r.closed_time:%m-%d %H} mid {r['mid']:.3f} {r.hrs:4.1f}h NOwon {int(r.pay)}  {str(r.question)[:70]}")


if __name__ == "__main__":
    main()
