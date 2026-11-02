"""r42 preliminary Polymarket calibration from the forward snapshot collector: outcome rate vs quoted mid by
time-to-expiry bucket. Outcomes cached in state/polymarket/outcomes.csv (resolved markets only)."""
import sys, glob, time, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from ptrade.config import STATE
from ptrade.data import polymarket as pm
fs = sorted(glob.glob(str(STATE / "polymarket" / "snap_*.csv.gz")))
df = pd.concat([pd.read_csv(f) for f in fs])
df["end"] = pd.to_datetime(df["end"], errors="coerce", utc=True); df["ts"] = pd.to_datetime(df["ts"], utc=True)
now = pd.Timestamp.now(tz="UTC")
cand = df[df.end < now - pd.Timedelta(hours=2)].id.astype(str).unique()
opath = STATE / "polymarket" / "outcomes.csv"
have = pd.read_csv(opath, dtype={"id": str}) if opath.exists() else pd.DataFrame(columns=["id", "yes_won", "closed_time"])
todo = [m for m in cand if m not in set(have.id)][: int(sys.argv[1]) if len(sys.argv) > 1 else 600]
rows = []
for i, m in enumerate(todo):
    try:
        r = pm.resolved_outcome(m)
        if r:
            rows.append(r)
    except Exception as e:  # noqa: BLE001
        print("err", m, str(e)[:80])
    time.sleep(0.15)
if rows:
    have = pd.concat([have, pd.DataFrame(rows)]).drop_duplicates("id", keep="last"); have.to_csv(opath, index=False)
print(f"outcomes cached: {len(have)} (new {len(rows)}, candidates {len(cand)})")
out = have.dropna(subset=["yes_won"]).copy(); out["yes_won"] = out.yes_won.astype(str).str.lower().eq("true").astype(float)
d = df.copy(); d["id"] = d.id.astype(str); d = d.merge(out[["id", "yes_won"]], on="id")
d["mid"] = (d.best_bid + d.best_ask) / 2; d["spread"] = d.best_ask - d.best_bid
d["hrs"] = (d.end - d.ts).dt.total_seconds() / 3600
d = d[(d.hrs > 0) & d.mid.between(0.001, 0.999) & (d.spread <= 0.1)]
# one observation per market per horizon bucket (the latest snapshot in that bucket)
d["hb"] = pd.cut(d.hrs, [0, 1, 6, 24, 72, 1e9], labels=["<1h", "1-6h", "6-24h", "1-3d", ">3d"])
d = d.sort_values("ts").groupby(["id", "hb"], observed=True).tail(1)
d["pb"] = pd.cut(d.mid, [0, .05, .15, .35, .65, .85, .95, 1])
print(f"markets with outcome: {d.id.nunique()}  obs: {len(d)}")
t = d.groupby(["hb", "pb"], observed=True).agg(n=("yes_won", "size"), mid=("mid", "mean"), won=("yes_won", "mean"))
t["edge_yes"] = t.won - t.mid
print(t.round(3).to_string())
tot = d.groupby("pb", observed=True).agg(n=("yes_won", "size"), mid=("mid", "mean"), won=("yes_won", "mean"))
tot["se"] = np.sqrt(tot.won * (1 - tot.won) / tot.n); print(tot.round(3).to_string())
# common question types (top words) to see what dominates the sample
print(d.question.str.extract(r"^(\w+\s\w+)")[0].value_counts().head(8).to_dict())
