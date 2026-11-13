"""r57 Polymarket favourite-longshot strategy (prelim): per resolved market take one quote 1-72h before resolution
(the latest in that window); buy YES at ask if mid in [lo_f, hi_f]; buy NO at (1 - bid) if mid in [lo_l, hi_l].
P&L per $ staked, clustered by event (end date + question stem). Elections excluded. No fees assumed (Polymarket
charges none on most markets); spread paid by crossing."""
import sys, glob, re, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
o = pd.read_csv("state/polymarket/outcomes.csv"); o["closed_time"] = pd.to_datetime(o.closed_time, utc=True, errors="coerce")
D = pd.concat([pd.read_csv(f) for f in sorted(glob.glob("state/polymarket/snap_*.csv.gz"))], ignore_index=True)
D["ts"] = pd.to_datetime(D.ts, utc=True)
D = D.merge(o, on="id", how="inner")
elec = D.question.astype(str).str.contains(r"Republican|Democrat|election|Senate|House|Governor|mayor|primary|electoral|seats", case=False)
D = D[~elec & D.best_bid.notna() & D.best_ask.notna() & (D.best_ask > D.best_bid) & D.closed_time.notna()]
D["hrs"] = (D.closed_time - D.ts).dt.total_seconds() / 3600
D = D[(D.hrs >= 1) & (D.hrs <= 72)]
q = D.sort_values("ts").groupby("id").tail(1).copy()
q["mid"] = (q.best_bid + q.best_ask) / 2; q["spread"] = q.best_ask - q.best_bid
q["event"] = q.closed_time.dt.strftime("%Y-%m-%d") + "|" + q.question.astype(str).str.replace(r"\d[\d,.$%]*", "#", regex=True).str[:40]
print(f"markets {len(q)} | events {q.event.nunique()} | median spread {q.spread.median():.3f} | median hrs to close {q.hrs.median():.0f}")
def test(lab, sel, side):
    t = q[sel].copy()
    if not len(t): print(lab, "none"); return
    t["px"] = t.best_ask if side == "YES" else 1 - t.best_bid
    t["pay"] = t.yes_won if side == "YES" else 1 - t.yes_won
    t["ret"] = t.pay / t.px - 1
    ev = t.groupby("event").ret.mean()
    print(f"{lab:34s} n {len(t):4d} events {len(ev):4d} | avg px {t.px.mean():.3f} win {t.pay.mean():.1%} | ret/$ {t.ret.mean():+.1%} "
          f"| event-clustered mean {ev.mean():+.1%} t {ev.mean() / ev.std() * np.sqrt(len(ev)):+.2f}")
for lo, hi in ((0.60, 0.80), (0.65, 0.85), (0.70, 0.90), (0.80, 0.95)):
    test(f"buy YES favourites mid {lo:.2f}-{hi:.2f}", q.mid.between(lo, hi), "YES")
for lo, hi in ((0.03, 0.10), (0.05, 0.15), (0.10, 0.25)):
    test(f"buy NO vs longshots mid {lo:.2f}-{hi:.2f}", q.mid.between(lo, hi), "NO")
print("--- mirror: fade near-expiry favourites (buy NO at 1-bid)")
for lo, hi in ((0.75, 0.90), (0.80, 0.95), (0.85, 0.97)):
    test(f"buy NO vs favourites mid {lo:.2f}-{hi:.2f}", q.mid.between(lo, hi), "NO")
for hmax in (6, 24):
    sel = q.mid.between(0.80, 0.95) & (q.hrs <= hmax)
    test(f"buy NO vs fav 0.80-0.95, ≤{hmax}h", sel, "NO")
    sel = q.mid.between(0.80, 0.95) & (q.hrs > hmax)
    test(f"buy NO vs fav 0.80-0.95, >{hmax}h", sel, "NO")
cat = q.question.astype(str).str.extract(r"^(\w+ \w+)")[0]
top = q[q.mid.between(0.80, 0.95)].assign(cat=cat).cat.value_counts().head(8)
print("question stems in the 0.80-0.95 bucket:", top.to_dict())
