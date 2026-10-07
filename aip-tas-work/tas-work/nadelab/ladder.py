#!/usr/bin/env python3
"""ladder.py: chain nadesearch stages (beam search over stages).

Each stage starts from an input prefix (the state after it), searches for the best next stack and writes the best
continuations; the best `beam` results of a stage (by apex height) seed the next stage. Stops when a stage does not
improve the apex.

usage: ladder.py MAP PREFIX OUTDIR [stages=4] [beam=4] [key=value ...]   (extra key=value go to nadesearch)
example: ./ladder.py block.map p0.txt runs/block stages=3 beam=6 tmax=330 hop=0
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, '..', '..', 'ddnet', 'build-sim', 'nadesearch')


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 1
    mapf, prefix, outdir = sys.argv[1:4]
    opts = dict(a.split('=', 1) for a in sys.argv[4:] if '=' in a)
    stages = int(opts.pop('stages', 4))
    beam = int(opts.pop('beam', 4))
    os.makedirs(outdir, exist_ok=True)
    seeds = [(0.0, os.path.abspath(prefix))]
    best = (0.0, None, '')
    for st in range(stages):
        results = []
        for si, (_, pre) in enumerate(seeds):
            outp = os.path.join(outdir, 'st%d_%d_' % (st, si))
            args = [BIN, mapf, pre, outp, 'out=%d' % beam, 'v=0', 'show=0'] + ['%s=%s' % kv for kv in opts.items()]
            if st == 0 and 'tmax' not in opts:
                args.append('tmax=220')
            txt = subprocess.run(args, capture_output=True, text=True).stdout
            for line in txt.splitlines():
                m = re.match(r'best (\d+): .*apex (\d+) px .* -> (\S+)$', line)
                if m:
                    results.append((float(m.group(2)), m.group(3), line))
        if not results:
            print('stage %d: no plans' % st)
            break
        results.sort(key=lambda r: -r[0])
        print('stage %d: %d plans, best apex %.0f px' % (st, len(results), results[0][0]))
        print('   ', results[0][2][:300])
        if results[0][0] <= best[0]:
            print('no improvement, stopping')
            break
        best = results[0]
        seen, seeds = set(), []
        for r in results:
            key = open(r[1]).read()
            if key in seen:
                continue
            seen.add(key)
            seeds.append((r[0], r[1]))
            if len(seeds) >= beam:
                break
    print('BEST apex %.0f px: %s' % (best[0], best[1]))
    return 0


if __name__ == '__main__':
    sys.exit(main())
