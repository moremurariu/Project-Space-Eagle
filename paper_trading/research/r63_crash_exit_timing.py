"""r63 crash-rebound exit timing (backlog #3): fixed hold horizons vs take-profit / stop-loss / trailing variants.
Events as r62 (first 4h<-10% hourly close after >=24h quiet per coin; entry at close of signal bar +1h). Hold anchored to
the LAST signal in the chain (live rule). Cost 0.2% round trip. Returns are per event; t-stat clusters by signal day
(crash events are cross-sectionally correlated)."""
import sys, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.research_utils import load_panel
P = load_panel(); r4 = P / P.shift(4) - 1; sig = r4 < -0.10
COST = 0.002; MAXH = 96
ev = []
for c in P.columns:
    s = sig[c].fillna(False).to_numpy(); px = P[c].to_numpy(); idx = P.index; n = len(s); i = 0
    while i < n - MAXH - 30:
        if s[i] and not s[max(0, i - 24):i].any():
            last = i; j = i + 1
            while j < min(n, last + 24):
                if s[j]: last = j
                j += 1
            e = i + 1
            if np.isfinite(px[e]) and np.isfinite(px[last + 1: last + MAXH + 2]).all():
                path = px[e:last + MAXH + 2] / px[e]             # path from entry
                pre = px[max(0, i - 4)] / px[e]                    # pre-crash level relative to entry
                ev.append({"coin": c, "t": idx[i], "off": last + 1 - e, "path": path, "pre": pre})
            i = last + 24
        else:
            i += 1
print(f"events {len(ev)}")

def rule(evt, hold=24, tp=None, sl=None, trail=None, tp_pre=None):
    path = evt["path"]; end = evt["off"] + hold          # exit index (hours since entry)
    peak = 1.0
    for h in range(1, end + 1):
        p = path[h]; peak = max(peak, p)
        if tp is not None and p >= 1 + tp: return p - 1
        if tp_pre is not None and p >= 1 + tp_pre * (evt["pre"] - 1): return p - 1   # recover a fraction of the crash
        if sl is not None and p <= 1 - sl: return p - 1
        if trail is not None and peak > 1 + trail and p <= peak * (1 - trail): return p - 1
    return path[end] - 1

variants = {f"hold {h}h": dict(hold=h) for h in (6, 12, 24, 36, 48, 72)}
variants.update({
    "24h + TP 5%": dict(hold=24, tp=0.05), "24h + TP 8%": dict(hold=24, tp=0.08), "48h + TP 8%": dict(hold=48, tp=0.08),
    "24h + SL 8%": dict(hold=24, sl=0.08), "24h + SL 12%": dict(hold=24, sl=0.12),
    "48h + trail 5%": dict(hold=48, trail=0.05), "72h + trail 8%": dict(hold=72, trail=0.08),
    "48h + recover 50% of drop": dict(hold=48, tp_pre=0.5), "24h + recover 50% of drop": dict(hold=24, tp_pre=0.5),
})
E = pd.DataFrame([{k: v for k, v in x.items() if k != "path"} for x in ev])
E["per"] = np.where(E.t < pd.Timestamp("2024-01-01", tz="UTC"), "IS", "OOS"); E["day"] = E.t.dt.floor("D")
out = []
for name, kw in variants.items():
    E[name] = [rule(x, **kw) - COST for x in ev]
    for per in ("IS", "OOS"):
        x = E[E.per == per]; d = x.groupby("day")[name].mean()          # day-clustered
        out.append({"variant": name, "per": per, "n": len(x), "days": len(d), "mean%": x[name].mean() * 100,
                    "hit": (x[name] > 0).mean(), "t_day": d.mean() / d.std(ddof=1) * np.sqrt(len(d)),
                    "p5%": x[name].quantile(0.05) * 100})
R = pd.DataFrame(out).pivot(index="variant", columns="per", values=["mean%", "t_day", "hit", "p5%"])
R = R.reindex(list(variants)); print(R.round(2).to_string())
# robustness: OOS by year for the main candidates
E["yr"] = E.t.dt.year
print((E.groupby("yr")[["hold 24h", "hold 36h", "hold 48h", "24h + TP 8%", "48h + trail 5%"]].mean() * 100).round(2).assign(n=E.groupby("yr").size()).to_string())
