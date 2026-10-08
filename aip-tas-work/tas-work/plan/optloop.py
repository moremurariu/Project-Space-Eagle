#!/usr/bin/env python3
"""optloop.py DIR [rounds=99] [seed=1000] [iters=6000000] [kv=0.3,0.4] [threads=4]
Endless optsweep rounds on tas-work/kog_full_best.txt. Each round places its windows on the current run's turns (speed
minima from x_trace): x_opt cut = apex - 90..110, end = apex + 20..35 (after the exit), stage 2 from end - 10 to the
next apex + 30 (or the finish), with per-round jitter and a new seed; the round's run is a copy (DIR/r<k>/run.txt),
and when it improves, the server-checked run becomes kog_full_<n>.txt / kog_full_best.txt and is committed and pushed.
"""
import os, random, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
d = os.path.abspath(sys.argv[1])
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
ROUNDS, SEED = int(kw.get('rounds', 99)), int(kw.get('seed', 1000))
ITERS, TH = kw.get('iters', '6000000'), kw.get('threads', '4')
KVS = kw.get('kv', '0.3,0.4').split(',')
BEST = os.path.join(TW, 'kog_full_best.txt')
# commits go to the checked-out branch, with this session's link (session=URL overrides)
BRANCH = subprocess.run(['git', '-C', ROOT, 'rev-parse', '--abbrev-ref', 'HEAD'], capture_output=True, text=True).stdout.strip()
SESSION = kw.get('session', 'https://claude.ai/code/session_01FMtZNoArAZJ4aqsFiXN4kh')
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'loop.log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path):
    T = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '990'], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 7 and a[0] == 'T':
            T[int(a[2])] = (float(a[7]), 'FINISH' in a)
    return T


def apexes(T):
    ts = sorted(t for t in T if t > 1000)
    fin = min([t for t in ts if T[t][1]] or [max(ts)])
    out = []
    for t in ts:
        w = [T[k][0] for k in range(t - 15, t + 16) if k in T]
        if T[t][0] == min(w) and T[t][0] < 0.85 * max(w) and (not out or t - out[-1] > 20):
            out.append(t)
    return out, fin


def finish(path):
    T = trace(path)
    f = [t for t in T if T[t][1]]
    return min(f) if f else None


for r in range(ROUNDS):
    rd = os.path.join(d, 'r%d' % r)
    os.makedirs(rd, exist_ok=True)
    subprocess.run(['cp', BEST, os.path.join(rd, 'run.txt')])
    T = trace(os.path.join(rd, 'run.txt'))
    ap, fin = apexes(T)
    rng = random.Random(SEED + r)
    wins = []
    for i, a in enumerate(ap):
        cut = a - rng.randint(90, 110)
        end = a + rng.randint(20, 35)
        if cut < 1000 or end >= fin - 15:
            continue
        nxt = ap[i + 1] + 30 if i + 1 < len(ap) else fin + 20
        g2 = min(nxt, fin + 20)
        if g2 - end < 40:
            g2 = min(end + 80, fin + 20)
        wins.append('%d:%d:%d:%d' % (cut, end, end - 10, g2))
    rng.shuffle(wins)
    kv = KVS[r % len(KVS)]
    f0 = finish(os.path.join(rd, 'run.txt'))
    log('round %d on %d: apexes %s, windows %s, kv %s' % (r, f0, ap, wins, kv))
    subprocess.run(['python3', os.path.join(HERE, 'optsweep.py'), os.path.join(rd, 'run.txt'), rd] + wins +
                   ['threads=' + TH, 'par=' + TH, 'seed=%d' % (SEED + 100 * r), 'iters=' + ITERS, 'kv=' + kv])
    f1 = finish(os.path.join(rd, 'run.txt'))
    cur = finish(BEST)
    log('round %d: %s -> %s (best file %s)' % (r, f0, f1, cur))
    if f1 is not None and cur is not None and f1 < cur:
        chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), os.path.join(rd, 'run.txt')], capture_output=True, text=True).stdout
        m = re.search(r'-> (\d+) ticks', chk)
        if m and int(m.group(1)) == f1 and 'frozen ticks 0' in chk:
            dst = os.path.join(TW, 'kog_full_%d.txt' % f1)
            subprocess.run(['cp', os.path.join(rd, 'run.txt'), dst])
            subprocess.run(['cp', os.path.join(rd, 'run.txt'), BEST])
            with open(os.path.join(TW, 'NOTES.md'), 'a') as nf:
                nf.write('- optloop round %d (%s): %d -> **%d (%.2f s)**, server-checked (windows on the run\'s turns, kv %s).\n'
                         % (r, os.path.relpath(rd, TW), f0, f1, f1 / 50.0, kv))
            msg = ('Full run %d (%.2f s): optloop round %d (x_opt windows on the turns + x_ds re-search + graft)\n\n'
                   'Server-checked: TasReplay, no freeze, no double start.\n\n'
                   'Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\n'
                   'Claude-Session: ' + SESSION) % (f1, f1 / 50.0, r)
            subprocess.run(['git', '-C', ROOT, 'add', os.path.relpath(dst, ROOT), 'tas-work/kog_full_best.txt', 'tas-work/NOTES.md'])
            subprocess.run(['git', '-C', ROOT, 'commit', '-q', '-m', msg])
            for k in range(4):
                if subprocess.run(['git', '-C', ROOT, 'push', '-u', 'origin', BRANCH]).returncode == 0:
                    break
                time.sleep(2 ** (k + 1))
            log('round %d: NEW BEST %d committed and pushed' % (r, f1))
        else:
            log('round %d: %d failed the server check' % (r, f1))
