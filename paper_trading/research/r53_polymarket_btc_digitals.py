"""r53 Polymarket 'Will the price of Bitcoin be above $K on <date>?' vs a Black-Scholes digital using Deribit 7d ATM IV.
Outcome computed from Binance BTCUSDT hourly close at 12:00 America/New_York on the date (Polymarket's rule is the
Binance 1m candle at 12:00 ET; hourly close at that time is used here). Hourly snapshots -> per-market dedupe."""
import sys, glob, re, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
import numpy as np, pandas as pd
from scipy.stats import norm
from ptrade.research_utils import load_panel
D = pd.concat([pd.read_csv(f) for f in sorted(glob.glob("state/polymarket/snap_*.csv.gz"))], ignore_index=True)
D["ts"] = pd.to_datetime(D.ts, utc=True)
rx = re.compile(r"Will the price of Bitcoin be above \$([\d,]+) on (\w+) (\d+)\?")
p = D.question.astype(str).str.extract(rx)
D = D[p[0].notna()].copy(); p = p[p[0].notna()]
D["K"] = p[0].str.replace(",", "").astype(float)
D["res"] = [pd.Timestamp(f"2026 {m} {d} 12:00").tz_localize("America/New_York").tz_convert("UTC") for m, d in zip(p[1], p[2])]
D = D[(D.ts < D.res) & D.best_bid.notna() & D.best_ask.notna() & (D.best_ask > D.best_bid)]
btc = load_panel(["BTCUSDT"])["BTCUSDT"]          # bar-open index; close of bar t at t+1h
close_at = btc.copy(); close_at.index = close_at.index + pd.Timedelta(hours=1)
D["S"] = close_at.reindex(D.ts.dt.floor("h")).to_numpy()
D["S_T"] = close_at.reindex(D.res).to_numpy()
D = D[D.S.notna() & D.S_T.notna()]
sk = pd.read_csv(glob.glob("state/options/skew_*.csv")[0]); sk["ts"] = pd.to_datetime(sk.ts, utc=True)
iv = sk[(sk.currency == "BTC") & (sk.tenor == 7)].set_index("ts").atm_iv.sort_index() / 100
D = D.sort_values("ts")
D["iv"] = pd.merge_asof(D[["ts"]], iv.rename("iv").reset_index(), on="ts", direction="backward")["iv"].to_numpy()
D["iv"] = D.iv.fillna(iv.iloc[0] if len(iv) else 0.40)
tau = (D.res - D.ts).dt.total_seconds() / (365 * 86400)
d2 = (np.log(D.S / D.K) - 0.5 * D.iv ** 2 * tau) / (D.iv * np.sqrt(tau))
D["model"] = norm.cdf(d2); D["mid"] = (D.best_bid + D.best_ask) / 2; D["y"] = (D.S_T > D.K).astype(float)
D["hrs"] = (D.res - D.ts).dt.total_seconds() / 3600
print(f"rows {len(D)}, markets {D.condition_id.nunique()}, dates {D.res.dt.date.nunique()} ({D.res.min():%m-%d} → {D.res.max():%m-%d})")
print(f"spread median {(D.best_ask - D.best_bid).median():.3f}; iv median {D.iv.median():.2f}")
inf = D[(D.mid > 0.03) & (D.mid < 0.97)]   # informative quotes only
for lab, x in (("market mid", inf.mid), ("BS model", inf.model), ("avg(mid, model)", (inf.mid + inf.model) / 2)):
    print(f"Brier {lab:16s} {((x - inf.y) ** 2).mean():.4f}  (n {len(inf)})")
inf = inf.assign(b=pd.cut(inf.mid, [0, .1, .25, .4, .6, .75, .9, 1]))
print(inf.groupby("b").agg(n=("y", "size"), mid=("mid", "mean"), model=("model", "mean"), win=("y", "mean")).round(3).to_string())
# trading: buy YES at ask if model - ask > e; buy NO at (1 - bid) if bid - model > e. One trade per market (first signal).
for e in (0.03, 0.05, 0.08, 0.12):
    t = D[(D.model - D.best_ask > e) | (D.best_bid - D.model > e)].copy()
    t["side"] = np.where(t.model - t.best_ask > e, 1, -1)
    t["px"] = np.where(t.side == 1, t.best_ask, 1 - t.best_bid)
    t["pnl"] = np.where(t.side == 1, t.y, 1 - t.y) - t.px
    f = t.groupby("condition_id").first()
    print(f"edge>{e:.2f}: markets {len(f):3d} | first-signal pnl/contract {f.pnl.mean():+.3f} (t {f.pnl.mean() / f.pnl.std() * np.sqrt(len(f)):+.2f}) "
          f"| return on stake {f.pnl.sum() / f.px.sum():+.1%} | dates {f.res.dt.date.nunique()} | YES share {np.mean(f.side == 1):.0%}")
