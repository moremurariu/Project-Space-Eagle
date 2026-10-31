"""Small HTTP helper with retries."""
import time
import requests

_session = requests.Session()


def get(url, params=None, timeout=20, retries=4):
    last = None
    for i in range(retries):
        try:
            r = _session.get(url, params=params, timeout=timeout)
            if r.status_code == 429:
                time.sleep(2 ** i * 2)
                continue
            r.raise_for_status()
            return r
        except Exception as e:  # noqa: BLE001
            last = e
            time.sleep(2 ** i)
    raise RuntimeError(f"GET failed {url} {params}: {last}")


def post_json(url, payload, timeout=20, retries=4):
    last = None
    for i in range(retries):
        try:
            r = _session.post(url, json=payload, timeout=timeout)
            if r.status_code == 429:
                time.sleep(2 ** i * 2)
                continue
            r.raise_for_status()
            return r.json()
        except Exception as e:  # noqa: BLE001
            last = e
            time.sleep(2 ** i)
    raise RuntimeError(f"POST failed {url} {payload}: {last}")
