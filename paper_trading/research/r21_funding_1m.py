"""Exact-ish Binance funding reconstruction from 1-minute premium index (Aug-Sep 2026) vs actual settlements."""
import sys, io, zipfile, warnings; sys.path.insert(0, "."); warnings.filterwarnings("ignore")
from concurrent.futures import ThreadPoolExecutor
import numpy as np, pandas as pd
from ptrade.config import CACHE
from ptrade.data.http import get
from ptrade.universe import UNIVERSE
SY = [s if s != "PEPEUSDT" else "1000PEPEUSDT" for s in UNIVERSE]
def month(sym, ym):
    url = f"https://data.binance.vision/data/futures/um/monthly/premiumIndexKlines/{sym}/1m/{sym}-1m-{ym}.zip"
    try:
        r = get(url, retries=2)
    except Exception:
        return None
    z = zipfile.ZipFile(io.BytesIO(r.content)); raw = z.read(z.namelist()[0]).decode().splitlines()
    rows = [ln.split(",")[:5] for ln in raw if ln and ln[0].isdigit()]
    df = pd.DataFrame(rows, columns=["t", "o", "h", "l", "c"]); df["t"] = pd.to_datetime(df.t.astype("int64"), unit="ms", utc=True)
    return df.set_index("t")[["o", "c"]].astype(float)
jobs = [(s, ym) for s in SY for ym in ("2026-08", "2026-09")]
with ThreadPoolExecutor(8) as ex:
    res = list(ex.map(lambda j: (j[0], month(*j)), jobs))
prem = {}
for s, df in res:
    if df is not None:
        prem.setdefault(s, []).append(df)
prem = {s: pd.concat(v).sort_index() for s, v in prem.items()}
print("1m premium loaded for", len(prem), "symbols")
def recon(p1m, interval_h):
    p = (p1m["o"] + p1m["c"]) / 2
    out = {}
    for end in pd.date_range(p.index[0].ceil(f"{interval_h}h"), p.index[-1], freq=f"{interval_h}h"):
        seg = p[(p.index >= end - pd.Timedelta(hours=interval_h)) & (p.index < end)]
        if len(seg) < interval_h * 50: continue
        w = np.arange(1, len(seg) + 1, dtype=float)
        P = float(np.dot(seg.values, w) / w.sum())
        I = 0.0001 * interval_h / 8
        out[end] = P + np.clip(I - P, -0.0005, 0.0005)
    return pd.Series(out)
cmp_rows = []
for s, p in prem.items():
    f = CACHE / f"binance_funding_{s}.parquet"
    if not f.exists(): continue
    act = pd.read_parquet(f)["rate"]; act.index = act.index.floor("min").round("h")
    act = act["2026-08-01":"2026-09-30"]
    if len(act) < 10: continue
    iv = int(round(pd.Series(act.index).diff().dt.total_seconds().median() / 3600))
    rc = recon(p, iv)
    j = pd.concat([act.rename("act"), rc.rename("rec")], axis=1).dropna()
    cmp_rows.append((s, iv, len(j), j.act.corr(j.rec), (j.act - j.rec).abs().mean() * 1e4, j.act.sum() * 1e4, j.rec.sum() * 1e4))
C = pd.DataFrame(cmp_rows, columns=["sym", "interval_h", "n", "corr", "mae_bp", "sum_act_bp", "sum_rec_bp"]).set_index("sym")
print(C.round(3).sort_values("corr").to_string())
print("median corr", round(C["corr"].median(), 3), " rank corr of 2-month sums:", round(C.sum_act_bp.corr(C.sum_rec_bp, method="spearman"), 3))
