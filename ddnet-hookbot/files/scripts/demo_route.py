#!/usr/bin/env python3
"""A route (waypoints in tiles) along the path a recorded run took, for the bot's `Claude route`.

usage: scripts/demo_route.py <dump.tsv> <out.txt> [--cid N[,N...]] [--spacing TILES] [--from S] [--to S]

Follows one tee (default: the lowest client id) and writes a waypoint every --spacing tiles of travel (default 12),
one "x y" line per waypoint, in tiles. With several client ids (--cid 0,1), it writes one path per tee, each after a
"# path" line: where a team splits up, the bot follows the line of the tee it is (docs/HOOKBOT.md). Times are seconds since the demo start. Put the result in
data/hookbot/routes/<map>.txt (and list it in CMakeLists.txt's EXPECTED_DATA), or in the same folder of your DDNet
save directory.
"""
import argparse
import math


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('tsv')
    ap.add_argument('out')
    ap.add_argument('--cid', default=None)
    ap.add_argument('--spacing', type=float, default=12)
    ap.add_argument('--from', dest='t0', type=float, default=0)
    ap.add_argument('--to', dest='t1', type=float, default=1e9)
    a = ap.parse_args()
    cids = [int(c) for c in a.cid.split(',')] if a.cid else [None]
    paths = []
    for c in cids:
        cid, route = make_route(open(a.tsv, encoding='utf-8', errors='replace'), c, a.spacing, a.t0, a.t1)
        paths.append((f'client {cid} of {a.tsv.split("/")[-1]}, every {a.spacing:g} tiles', route))
    write_routes(a.out, paths)
    print(f'{" + ".join(str(len(r)) for _, r in paths)} waypoints -> {a.out}')


def make_route(lines, cid=None, spacing=12, t0=0, t1=1e9):
    """(cid, [(x, y) in tiles]) from demo_dump lines; cid None = the lowest client id"""
    first = None
    path = {}
    for line in lines:
        if line[0] == 'M':
            first = int(line.split('\t')[1])
        if line[0] != 'T':
            continue
        p = line.split('\t', 5)
        path.setdefault(int(p[2]), []).append(((int(p[1]) - first) / 50, int(p[3]) / 32, int(p[4]) / 32))
    if cid is None:
        cid = min(path)
    pts = [(x, y) for (t, x, y) in path.get(cid, []) if t0 <= t <= t1]
    if not pts:
        return cid, []
    route = [pts[0]]
    run = 0.0
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        step = math.hypot(x1 - x0, y1 - y0)
        if step > 8:
            continue  # a teleport or a respawn: not something to fly along
        run += step
        if run >= spacing:
            route.append((x1, y1))
            run = 0.0
    if route[-1] != pts[-1]:
        route.append(pts[-1])
    return cid, route


def write_route(out, route, comment):
    write_routes(out, [(comment, route)])


def write_routes(out, paths):
    """paths: [(comment, route)]; more than one: each after a "# path" line"""
    with open(out, 'w') as f:
        for comment, route in paths:
            f.write(f'# path along {comment}\n' if len(paths) > 1 else f'# route along {comment}\n')
            for x, y in route:
                f.write(f'{x:.1f} {y:.1f}\n')


if __name__ == '__main__':
    main()
