# AiP-Gores TAS: beat Teero

## Goal and rules
Make a tool-assisted run of the DDNet map **AiP-Gores** that beats Teero's **50.72 s (2536 ticks)**.
- Solo run. Grenade is the only weapon to pick up (shotgun and laser aren't worth it).
- **No double start**: you may not cross the start line again after the race starts. The simulator kills such states.
- The run must work on a real server. The simulator is DDNet's own prediction code. Final runs can be checked on real
  server code with `TasReplay` in `testrunner`, see `ddnet/docs/TAS-RENDER.md`.
- Don't start from Teero's inputs; generate the run. Using his video and the extracted track/catalog as reference is fine.
- **Current instruction from the user: do NOT do full runs until you beat Teero on the first corridor** (see "Current task").
  Lessons from corridor 1 should then carry over to the pre-grenade segment and the full run.
  **Status (Oct 2, night): corridor 1 is beaten, 296 race ticks vs Teero's 298** (`tas-work/c1_best.txt`, server-checked).
  No full run has been made yet; that is the next step.
- The original machine had a 2-core limit. On this VM, use what `nproc` gives you, but keep an eye on memory.

## Setup
```
bash setup.sh          # apt deps + rustup if needed, builds ddnet/build-sim/{tas,lab,tas2,polish,nest,mapdump,pre}
```
It finishes by replaying `best_57.08.txt`; the last line must show `rt=2854`. If the build fails, the TAS targets are
defined in `ddnet/CMakeLists.txt` (search `TAS_TOOL`). They need gtest (DOWNLOAD_GTEST=ON) and a server build.
`ddnet/` is the user's DDNet fork (branch physics-experiments, base commit in `ddnet/BASE_COMMIT.txt`) with all
uncommitted work included. TAS sources are in `ddnet/src/tas/`.

**Pitfalls (both cost hours):**
- If your shell is zsh, `A="k=v k2=v2"; ./tas ... $A` passes ONE argument, and `tas` silently uses only the first key.
  Put parameter sets in bash scripts (`run*.sh`, `queue.sh` are bash) or pass explicit args.
- Never `cp` over an executable that has already run (the original macOS got SIGKILL 137). `rm` first.

## Where things stand
- **Best full run: 57.08 s** (`tas-work/best_57.08.txt`, one input line per tick from spawn: `dir jump hook fire tx ty weapon`).
  Teero 50.72 s, so we're 318 ticks behind: ~112 before the grenade (we pick it up at race tick 1095, Teero at 983) and ~206 after.
- Earlier: 60.52 s (lost run), 58.94, 57.66. `tas-work/full/` has the segment files (`pre4.txt` = start to grenade
  at 1095; `post4_1.txt` = the full 57.08 run).
- **Corridor 1 (start line → x > 8800): 296 race ticks** (Teero 298): `tas-work/c1_best.txt`, inputs from spawn.
  Built with the new energy beam `pre` (pre-start crossing search + corridor search with a remaining-time model).
- Read **`tas-work/NOTES.md`** first: physics facts, tools, all experiments with results (positive and negative),
  test beds, and the log. The newest section ("Corridor 1 beaten") explains this session's results.

## Full-run attempt (Oct 3): goal < 50 s (< 2500 race ticks), no double start
**Not reached.** Best complete run: **2804 ticks (56.08 s)**, `tas-work/best_56.08.txt` (LNS + polish), from the tas LNS on the new pipeline
(`tas-work/runs/lnstas/t1/best.txt`, server-check with `TasReplay`; the LNS keeps improving it while it runs).
Pipeline: pre-start + corridors with `seg` chains (`rh.py`), grenade pickup + shaft exit with `gren.py` (`seg` climbs
+ `pf` stack brute force), post-grenade with the old `tas` search to the finish, then `lnstas.py` (re-search from random
cut points, keep finish improvements). Where the time goes vs Teero (2536): ~-47 at the grenade pickup, ~-20 more by
k1100 (shaft exit at ~23 px/t vs his ~31), then ~13% slower for the rest of the run (hook braking eats most of the
explosion energy). Diagnostics from Teero's own states: `seg` is ~3.5% slower than him before the grenade and about at
par in the first post-grenade corridor when the reload phase matches. Everything (tools, options, negative results) is
in tas-work/NOTES.md, section "Full run < 50 s attempt".

## Earlier task: corridor 1 (done)
Metric: race ticks from the start-line crossing until the tee's x > 8800 px (tile 275). **Teero: 298. Ours: 296**
(`tas-work/c1_best.txt`; `./srvcheck.sh c1_best.txt` replays it on real server code: start tick 73, x > 8800 at
race tick 296, no freeze, no double start, positions identical to the simulator).
What it took (details and all numbers in NOTES.md, "Corridor 1 beaten"):
- q14 grid: corridor time is driven by the crossing energy E = v² − y (~0.08–0.1 tick per unit of E), not its height.
- The pre-start is capped: the hook only accelerates below 15 px/t, so the ceiling gives E ≤ 147; Teero's route (fall, swing,
  air jump on the line) gives ≤ 291. His track seems to show more; that is most likely a few % scale error in the
  video registration (his energy appears to grow during free fall). So the old
  "Teero-like" teleport (E = 329) was too generous and the old corridor search did *not* beat him from our own crossings
  (303–311).
- New `pre` tool: energy beam for the pre-start (best low crossing `runs/pre/vl1p.txt`: y 456, v (24.6, −11.5),
  E 279, jumps spent) and for the corridor, ranking states by race tick + time-to-go along Teero's line at the speed their
  energy gives, with the next 300 px at the current speed (`trref=teero_track.txt hnow=300`). That gave 297; an iterated
  re-search (`lns.py`) gave 296.
Next steps (no full runs were made yet):
- Carry the method over to the pre-grenade segment (corridor 2, return leg, descent to the grenade). `pre` measures
  progress and the time model by x, which only works while the route goes right; later segments need progress as arc
  length along a reference line (Teero's track works as the line) and a gate at the segment end.
- Crossing: ours is E 279 vs the 291 cap (better swing at the bottom of the shaft, ~1 tick).
- Corridor 1: we lose ~1–3 ticks to Teero before block 2 (x ≈ 2240) and ~1.5 between blocks 3 and 5; more `lns.py` time.

## Tools (all in ddnet/build-sim, run from tas-work/)
- `tas AiP-Gores.map search key=value...`: the main beam search. Important keys: `beam`, `threads`, `repeat=1` (decide every
  tick; better than 2), `macro=...` (hook hold lengths), `energyshare`, `alpha`, `vref`, `survive`, `firelook=1`, `stencil16=3`,
  `stopx=` / `stopxlt=` / `stopd=` / `stopgren=1` / `stopregion=` (gates; the result is written to `out=`),
  `prefix=FILE` (replayed from spawn), `tp=x,y,vx,vy,jumped` + `tpstarted=1` + `tpgren=1` + `tpfirst=1` (teleport test beds),
  `kcredit=1 kready=10` (the post-grenade win: credit for point-blank kick potential), `gatecands=N`, `ghost=teero_track.txt`.
  Many experimental keys exist (see `SParams` in `ddnet/src/tas/tas.cpp`); NOTES lists which helped.
- `lab AiP-Gores.map "script"`: physics lab. Commands (`;`-separated): `tp x y vx vy`, `gren`, `jumped n`,
  `in dir jump hook fire tx ty [n]`, `replay FILE`, `print`, `quiet/loud`, `scanfire N K dir`, `nextexp`,
  `swing K`, `padscan ...`, `padrange a0 a1 step`, `prefire`, `stack2`, `distinit` + `scorefile IN OUT`.
- `polish`: local search on a finished input file against the true gate time (`race=1`: judge by race ticks, so the
  pre-start can be mutated too). `tas2`, `nest`: experimental searches (weaker than `tas` so far).
- `pre AiP-Gores.map key=value...` (new): energy beam with dominance pruning. Pre-start crossing search
  (`rank=cont ymin=...`), crossing polish (`polish=FILE`), and corridor search after the line (`gatex=8800 gatelambda=0
  trref=teero_track.txt hnow=300 postdir1=1`). All keys are documented at the top of `ddnet/src/tas/pre.cpp`.
- `seg AiP-Gores.map prefix=FILE gate=K|grenade|finish ...` (new): energy beam for any segment, progress along
  Teero's track. Time models `ghost=1|2|3|4`, `sinks=`, `kcredit=`, `prefire=1` + `padaims=`, `latpen=`,
  `track=` (time-indexed tracking), `angles=128 hookdedup=0`, diagnostics `tp=... tpk= tpreload=` and `SEG_DUMP=1`.
  All keys are documented at the top of `ddnet/src/tas/seg.cpp`.
- `pf AiP-Gores.map base=FILE rel=1 t1lo= t1hi= tend=0 xf=5900` (new): brute force of a pre-fired shot (0.05 deg)
  plus a point-blank follow-up on a fixed climb (the shaft-exit stack).
- `rh.py START NAME [vset=g3|g4 nvar= stopk= sel= selpost=]`: receding-horizon `seg` chain; `gren.py PREFIX NAME`:
  grenade pickup + exit pipeline; `lnstas.py BEST NAME [workers= cutmin= hours= bestticks=]`: LNS with `tas`.
- `runH.sh NAME PREFIX [extra]`: corridor 1 with `pre` from a pre-start prefix (results in `runs/H.res`).
  `lns.py BEST [minutes] [workers]`: iterated re-search of the best run from random cut points (`runs/lns/`).
  `srvcheck.sh INPUTS [STOPX]`: replay on the real server (`testrunner`, build it with `ninja testrunner`) and print the
  start tick and the race tick at x > STOPX.
- Python helpers in tas-work: `trace.py "lab-prefix" FILE [every] [from]` (first column = input index, not race tick),
  `shots.py`, `energy.py` (energy accounting per cause), `hookloss.py`, `splits.py`, `chain.py` (segment chain over
  distance gates with several configs).
- Test-bed wrappers (bash): `runA.sh` (grenade pickup → x≥9216, Teero 161, best 172–174), `runB.sh`, `runC.sh` (corridor-2
  climb), `runC1.sh` (corridor 1 from the Teero-like teleport, 294–296), `runE.sh`, `runP.sh NAME PREFIX`, `runT.sh NAME TP`.
  `queue.sh FILE` runs a list of commands `QP` at a time (default 4). `queue1.sh` runs them sequentially.

## Reference data
- `teero_track.txt`: Teero's position per race tick (−70..2539), registered from his video `teero/teero.mp4`
  (YouTube eHJJNU-hQoU; Tater in that video is only a ghost and never interacts).
  Race tick k = video time 1.434 + k/50 s. Positions are ±~10 px.
- `teero/catalog/shots.md` / `shots.tsv`: his 50 post-grenade explosions with offsets, surfaces, hook use, and pre-fired shots.
- `map.txt` (`#` solid, `f` freeze, `E` entity; row = tile y), `map.png`. Grenade at tile (170,77), start line x=29,
  finish x=276 rows 143–146.

## Key lessons so far (details in NOTES.md)
1. Wider beams don't help (10,000 ≈ 1,000 on test bed A). The score and the action set are the bottleneck.
2. What helped: `kcredit` (a ready grenade next to a surface is worth its point-blank kick), `repeat=1`,
   energyshare 0.6 and finer macro holds before the grenade.
3. Teero's precise tricks need ~0.1° aims (e.g. the pre-fired grenade + point-blank stack at the shaft exit after the pickup,
   reproduced in `runs/pf/fine_14_259.2*.txt`). The search's 64-angle shot set can't find them.
4. Energy accounting: hook braking dominates the losses (post-grenade −26k..−33k v² vs +21k from explosions). Above
   ~35 px/t a lossless hook turn needs a radius bigger than the corridors (r ≈ v²/3).
5. Pre-grenade energy carries over. Our deficit vs Teero starts at the crossing (~−100 E) and grows in corridors 1–2.
6. The crossing energy is capped (≤ 291 on Teero's route); after that, corridor time depends on using the bare blocks
   (one-tick touches refill both jumps: +174 ground, +144 air at vy ≈ 0) and on staying low where the corridor goes down.
7. The tas score's energy share discourages ground jumps (a grounded state is credited 2 × jumpenergy = 400 > 174).
   The `pre` time model (time-to-go at the speed the energy gives along a reference line, plus the current speed over the
   next 300 px) beat both the tas search and a plain λ·E + x/vref ranking.
8. Single searches vary by ±3–5 ticks; compare trends or several runs, and use re-search from cut points (`lns.py`).
