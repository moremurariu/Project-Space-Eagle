"""r62 crash-rebound: do events with repeated signals (4h < −10% on several consecutive hourly closes) rebound less, and
should the exit be anchored to the FIRST signal (no extension) or the LAST (live: re-signals extend the 24h hold)?
Hourly panel, events = first signal after ≥24h quiet per coin; entry = close of signal bar +1h (lag 1); cost 0.2% rt."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
P = load_panel(); r4 = P / P.shift(4) - 1; sig = r4 < -0.10
rows = []
for c in P.columns:
    s = sig[c].fillna(False).to_numpy(); px = P[c].to_numpy(); idx = P.index
    i, n = 0, len(s)
    while i < n - 80:
        if s[i] and not s[max(0, i - 24):i].any():
            # extension chain: a re-signal within the hold keeps extending (live rule); count signals in the next 24h chain
            last = i; j = i + 1
            while j < min(n, last + 24):
                if s[j]: last = j
                j += 1
            k6 = int(s[i:i + 6].sum())
            e = i + 1
            if last + 25 < n and np.isfinite(px[e]):
                r_first = px[i + 25] / px[e] - 1                      # exit 24h after first signal
                r_last = px[last + 25] / px[e] - 1                    # exit 24h after last signal (live)
                rows.append({"coin": c, "t": idx[i], "k6": k6, "chain_h": last - i, "r_first": r_first - 0.002, "r_last": r_last - 0.002})
            i = last + 24
        else:
            i += 1
E = pd.DataFrame(rows); E["per"] = np.where(E.t < pd.Timestamp("2024-01-01", tz="UTC"), "IS", "OOS")
E["grp"] = np.where(E.k6 <= 1, "single (1 signal bar in 6h)", "repeated (≥2 in 6h)")
g = E.groupby(["per", "grp"]).agg(n=("r_first", "size"), first_exit=("r_first", "mean"), last_exit=("r_last", "mean"),
    chain_h=("chain_h", "median"), hit_first=("r_first", lambda x: (x > 0).mean()))
g[["first_exit", "last_exit"]] = (g[["first_exit", "last_exit"]] * 100).round(2); g["hit_first"] = g.hit_first.round(2)
print(g.to_string())
for per in ("IS", "OOS"):
    x = E[E.per == per]; print(f"{per}: all events n {len(x)} | exit@first+24h {x.r_first.mean()*100:+.2f}% | exit@last+24h (live) {x.r_last.mean()*100:+.2f}%")
