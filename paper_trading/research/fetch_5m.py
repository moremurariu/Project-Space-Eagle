"""Monthly 5m spot klines (data.binance.vision) for the universe, 2024-01 → last complete month. Cached parquet."""
import sys, io, zipfile; sys.path.insert(0, ".")
from concurrent.futures import ThreadPoolExecutor
import pandas as pd
from ptrade.config import CACHE
from ptrade.data.http import get
from ptrade.universe import UNIVERSE
OUT = CACHE / "k5m"; OUT.mkdir(exist_ok=True)
months = [str(m) for m in pd.period_range("2024-01", pd.Timestamp.now().to_period("M") - 1, freq="M")]
def one(job):
    sym, ym = job
    f = OUT / f"{sym}_{ym}.parquet"
    if f.exists(): return
    try:
        r = get(f"https://data.binance.vision/data/spot/monthly/klines/{sym}/5m/{sym}-5m-{ym}.zip", retries=2)
    except Exception:
        return
    z = zipfile.ZipFile(io.BytesIO(r.content)); raw = z.read(z.namelist()[0]).decode().splitlines()
    rows = [ln.split(",")[:5] for ln in raw if ln and ln[0].isdigit()]
    df = pd.DataFrame(rows, columns=["t", "o", "h", "l", "c"])
    t = df.t.astype("int64"); unit = "us" if t.iloc[0] > 1e14 else "ms"
    df["t"] = pd.to_datetime(t, unit=unit, utc=True)
    df.set_index("t")[["c"]].astype(float).to_parquet(f)
jobs = [(s, m) for s in UNIVERSE for m in months]
with ThreadPoolExecutor(10) as ex:
    list(ex.map(one, jobs))
print("files:", len(list(OUT.glob("*.parquet"))), "of", len(jobs))
