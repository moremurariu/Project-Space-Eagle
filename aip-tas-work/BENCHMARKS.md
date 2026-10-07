# Benchmarks: simulators and TAS finders across the branches (Oct 7)

Question: which simulator and which search workflow is fastest / best, and what is worth merging. Every number here
was measured, none is taken from the branches' notes.

## Method
Machine: 4 cores, 15 GB, Linux; every tree built from the same DDNet commit (470eead4a) with the same flags
(Release, server-only, gcc 13, lld). One benchmark process at a time unless stated.

Trees:
- **faraday** (`claude/wonderful-faraday-klv0ms`, `aip-tas-fast/ddnet.patch`): CFast stepper, its collision fast
  paths (per-tile solid table for TestBox / IsOnGround / hook rays, broad-phase skips), `ddsearch`.
- **cannon** (`claude/fervent-cannon-0rrvvi`, `ddnet-upstream.patch`): block / exist prefix-sum fast paths,
  TileExists cache, `ms_TasSolo`, allocation-free `CTasGame::CopyFrom`; anchored `seg`.
- **davinci** (`claude/compassionate-davinci-5q4n6j-postnade`, `ddnet-upstream-ds.patch`): cannon's collision fast
  paths without the CopyFrom change, CFastG, `segf`, `x_ds`, `x_tig`, ...
- **cray** (`claude/fervent-cray-0pser4`): cray's sources as they ran in its own tree: faraday-style collision fast
  paths + TileExists cache, CFastG with cray's changes, cray's `seg` (survx, hookw, rjrun, ...). Built on upstream with
  davinci's CMake/test patch.
- **merged**: this folder (`ddnet-upstream.patch` + `ddnet/src/tas`).

Inputs: `kog_full_2613.txt` (best full run, 2681 inputs) and `kog_pregren_962.txt` (best pre-grenade run, 1030 inputs).

## 1. Simulator speed and exactness (`simbench`, `ddnet/src/tas/simbench.cpp`)
`simbench MAP RUN [replay|beam|fuzz]`, the same source compiled into every tree.
- *copy+step*: the search inner loop. 40 states sampled along the run, each copied and stepped with a random input
  5000 times; ns per copy+step.
- *fuzz*: exactness. From random sampled states, 150 random-input ticks on every fast stepper with all fast paths
  on, compared with plain CTasGame with every fast path off (position, velocity, hook state, freeze / double start).

copy+step in ns (lower is better):

| tree | full run, CTasGame | pre-grenade, CTasGame | full run, CFastG | pre-grenade, CFastG | pre-grenade, CFast |
|---|---|---|---|---|---|
| any tree, fast paths off (plain DDNet code) | 3240-3650 | 3260-3840 | 2040-2190 | 1910-2060 | 2060-2080 |
| faraday | 2348 | 1900 | - | - | **546** |
| cannon | 929 | 1173 | - | - | - |
| davinci | 1216 | 1088 | 786 | 721 | - |
| cray | 1357 | 1227 | 663 | 630 | - |
| **merged** | **808** | **735** | **654** | **604** | 543-632 (= faraday within noise) |

Exactness: **0 mismatches** for every stepper in every tree (~187,000 random-input ticks per tree on the two runs, and
the replays of 2613 / 962 reproduce the server-checked results). One apparent mismatch class turned out to be
bookkeeping, not physics: the fast steppers kill a double start immediately, CTasGame only marks it (`m_StartTick = -2`).

What the numbers say:
- The collision fast paths are the big win (3-4x for CTasGame, ~3x for CFastG). The two branches' implementations
  are complementary: cannon's (prefix-sum broad phase, TileExists cache, `ms_TasSolo`, CopyFrom) is best for CTasGame;
  faraday's (inlined per-tile solid table in TestBox / IsOnGround / the hook ray) is best for CFast / CFastG, which
  spend their time near walls where a broad phase can't skip anything. The merged patch has both and is the fastest
  for every stepper (CTasGame 13-37% faster than the best single branch).
- CFast (pre-grenade only, 608-byte state) is still the fastest stepper; CFastG (grenades, 920 bytes) is ~10% slower
  on the same segment and is the one to use after the pickup.

## 2. S1: same search, different simulator
`seg` / `segf` with one fixed parameter set (beam 3000, threads 1, ghost=1 hnow 600 ghoste 0.02, latpen 0.1/32,
survive 30 every 2, rothook, quant, kcredit 2) on two windows; reference = the run's own track; no incumbent:
- post-grenade: 2613 cut at rt 2450 -> finish;
- pre-grenade: 962 cut at rt 850 -> pickup.

| search | post-grenade window | pre-grenade window |
|---|---|---|
| cannon `seg` (CTasGame) | 88.2 s | 28.6 s |
| davinci `seg` (CTasGame) | 79.6 s | 28.8 s |
| cray `seg` (CTasGame) | 74.4 s | 26.1 s |
| davinci `segf` (CFastG) | 69.5 s | 25.2 s |
| cray `segf` (CFastG) | 62.6 s / 64.5 s | 21.4 s / 22.3 s |
| **merged `seg`** (CTasGame) | 67.5 s | 24.4 s |
| **merged `segf`** (CFastG) | **62.0 s** | **21.7 s** |
| davinci / cray `segf` with `TAS_NOFAST=1` | 168.7 / 172.5 s | 70.3 / 70.6 s |

Every variant returns the same result on each window (finish rt 2638; pickup rt 968), so they are interchangeable
and only the wall time differs. The collision fast paths give 2.4-3.3x on a real search, the stepper class
(CTasGame -> CFastG) only another ~10% on top of the merged fast paths.

Profile of merged `segf` on the post-grenade window (60 gdb stack samples, `tas-work/bench/prof.sh`): survival rollouts
~50% of the time (they step too), action generation (`HookTargets`, `RotHookAims`, `KickPotentialAt`, `EffEnergy`)
~40%, character stepping (`CCharacterCore::Tick` + `Move`) ~30% in total. **The simulator is no longer the bottleneck of `seg`; the next speedups are in its survival checks and action
generation** (e.g. caching hook targets per position cell).

## 3. S3: the real workflow at the same budget (incumbent LNS)
Large-neighbourhood search as the branches run it: cut the current best at a random race tick, re-search to the gate
with the best run's continuation kept in the beam (so a job can only tie or improve), keep improvements. Each finder
uses the parameter variants of its own branch's driver. 4 workers, 10 minutes each, one finder at a time, same start
run, same cut distribution (pre-grenade: rt 300 .. best-3; post-grenade: rt 1100 .. best-40), and **every
improvement re-checked on the real server code** (TasReplay) before it was accepted. Driver: `tas-work/bench/lnsbench.py`.

| segment (start run) | finder (branch, variants from) | jobs | result |
|---|---|---|---|
| pre-grenade (`kog_pregren_978.txt`, pickup 978) | **`ddsearch`** (faraday, `fast/tools/lns.py`) | 33 | **978 -> 976** |
| | `segf inc=` (cray, cannon's `lns3.py` variants) | 19 | no gain |
| | `seg anchor=` (cannon, `lns3.py`) | 16 | no gain |
| post-grenade (`kog_full_2677.txt`, finish 2677) | **`x_ds incforce=1`** (davinci, `dschain.py` variants, beam 1000) | 25 | **2677 -> 2667** (5 gains: cuts 1566-2531) |
| | `segf inc=` (davinci, `lnsinc.py` variants, beam 8000-20000) | 4 | no gain |
| | `segf inc=` + `survx` (cray's late-cut recipe) | 4 | no gain |

What it says:
- Pre-grenade: `ddsearch` on CFast is the finder to use. It ran ~2x more jobs than the `seg` family in the same time
  (cheaper stepper and a time model against the best run) and was the only one to improve.
- Post-grenade: `x_ds` (deferred shots: a grenade is only "fired" once its explosion is known to help, by patching the
  shot into the history) is far more productive per CPU-minute than `segf`. `segf`'s lnsinc-style jobs (big beams to
  the finish) take ~10 minutes each from mid-run cuts, so they barely get to try anything in a short budget; that is
  how they were used in their branch too (multi-hour runs, late cuts), which is where they found 2-8 tick gains.
- 10-minute runs are a small sample (single seeds; a gain is a discrete event). The direction matches the branches'
  own histories: davinci's 2639 -> 2614 came from `x_ds`/`x_tig` pipelines, faraday's 978 -> 962 from `ddsearch`.
