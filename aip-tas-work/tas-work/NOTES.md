# AiP-Gores TAS — working notes (session 43a18766, Oct 2)

Goal: beat Teero's 50.72 s (2536 ticks) solo, no double start, grenade only, server-valid.
Previous best (lost from /tmp, recoverable only by rerunning): 60.52 s.

## Layout of this folder (persistent; /tmp scratch got wiped on Oct 1)
- AiP-Gores.map, map.txt (mapdump grid: # solid, f freeze, E entity), map.png
- teero/ : teero.mp4 (YouTube eHJJNU-hQoU), regtrack.py + reg_common.py (frame -> map registration)
- teero_track.txt : Teero, `race_tick x y` at 50 Hz, ticks -70..2539 (start = video 1.434 s; finish 50.733 s, matches)
- recovered/ : scripts recovered from old transcripts (e05f_, 44a4_, 50ec_ prefixes)
- runs/ : outputs
- Tools (built in /Users/c29/ddnet/build-sim, sources in /Users/c29/ddnet/src/tas):
  - tas  : old beam search (copied from ddnet-fable worktree, + nofirebefore=)
  - tas2 : new search (Pareto per spatial group, rollout option) — so far WORSE than tas
  - lab  : physics lab: `lab map "gren;in 1 0 0 0 0 1 3;tp x y vx vy;scanfire 360 6 1"`, `replay FILE`
  - build: `cd build-sim && ninja -j2 tas tas2 lab`

## Physics facts (verified in code / lab)
- Hook: accel 3 toward anchor (x *0.95 if holding toward it else *0.75; downward pull *0.3), applied only if
  |v_new| < 15 or |v_new| < |v| (post-gravity). Above 15 px/t a hook can only brake/turn; near-lossless turning
  when the anchor is ~93 deg from velocity (slightly behind perpendicular).
- Air: holding the direction of motion costs nothing (SaturatedAdd only up to 5); dir 0 -> vx *= 0.95/tick; opposite -> -1.5/tick.
- Velocity ramp: displacement = v * 1.4^-((50v-550)/2000) for v > 11 px/t; displacement peaks ~48 px/t at v~119.
- Jumps set vy (ground -13.2, air -12). A one-tick ground touch on a bare (non-freeze) platform refills both jumps
  with no vx loss if dir held (vx>10 unaffected by ground accel). The small gray floating blocks are energy sources.
- Freeze is tested at the tee centre only -> centre can be 32 px from a freeze-lined solid surface.
- Grenade: projectile 20 px/t, does not inherit tee velocity; explosion force 12*(1 - clamp((r-48)/87)) away from
  explosion (needs >= 2), reload 25 ticks. Near a wall the explosion is the same tick -> ~12 px/t kick.
  Kick vs aim is smooth; 64 angles is enough resolution.

## Teero reference
- Corridor 1 (start -> x 8800): 298 ticks. Crossing ~ (900,465) v (25,-13), jumps spent.
- Grenade pickup: race tick 983 at (5410,2457), v ~ (2,14), air-jumps immediately.
- Post-pickup: climbs the 2-tile shaft at 15 px/t (hook), top at +40, then grenade-accelerates to ~43 px/t (+72),
  50-60 px/t peaks; reaches x>=9216 (tile 288, row ~56) at +161.

## Test beds (old tas, beam 1000, 2 threads)
- C1: tp=900,465,25,-13,3 stopx=8800: old tas 304 (Teero 298). tas2: 402-488 (bad).
- A : tp=5410,2457,2,14,0 tpstarted=1 stopx=9216: old tas 184 (Teero 161), 90 s.
  Ours level with Teero until +40 (we climb with a grenade), then lose ~14 tiles by +100 in acceleration.

## Findings, Oct 2 (later)
- Teero catalog (teero/catalog/shots.md, by a helper agent): 50 explosions after pickup, ~every 28-31 ticks,
  46/50 point-blank (<=48 px, full 12 kick), ~2/3 on bare gray blocks, hooked at 36/50, ~10 pre-fired grenades.
- Test bed caveat: tp at the pickup spot WITHOUT tpgren costs ~3 ticks (tee is 51 px from the pickup); use tpgren=1, repeat=1.
  A1 (pickup -> x>=5760): old tas 58 (Teero 57) but arrives at 20 px/t vs Teero ~33.
- The whole A deficit (182 vs 161) comes from the shaft exit: Teero leaves at ~26-33 px/t (vel (1.9,-11.8) -> (26.6,-8.9)
  at k1021.6, i.e. ~+25 = two kicks), we leave at ~15-20.
- Score diagnosis (lab distinit/scorefile): along Teero's line the beam score is 2-8 ticks WORSE than ours during ticks 21-37
  (he climbs slower holding a pending grenade, along=0 at the top) and better from tick 45 on. Beam prunes his line.
- Pads (lab padscan on the tracked climb runs/trk.txt): a shot at t1 in 12..26 aimed ~245-258 deg hits the underside of the
  L-arm end (x 5200-5231, y 1951) at input tick 35..43.
- Tools added: lab cmds scanfire/prefire/stack2/padscan/distinit/scorefile/nextexp/reload0; tas options nofirebefore,
  firelookmax, projkey, pendshare/pendbonus/pendwin, tpgren, gcredit, kcredit/kready, clancap/clanperiod, optval,
  commitfire, padcredit, tpfirst. polish (local search), nest (nested beam).
- Negative results on A (all 182-206): alpha 20-80, energyshare 0/0.8, beam 3000 (182), fireangles 128, firelook 30-35 +
  firerange 600, projkey, pendshare(+bonus), gcredit, kcredit, rollout 10/20, clan caps, optval, commitfire, harmonic,
  clearance speed field, dynvref, randshare; nest (inner beam lookahead 16-40) no better than plain beam.
- Full pipeline baseline (tas_full, beam 1000, 1 thread): grenade at race tick 1111 (Teero 983). full/pre1.txt.

## !! Harness bug (found Oct 2): zsh does not word-split unquoted variables
`A="beam=1000 tp=..."; ./tas ... $A` passes ONE argument -> tas parses only the first key. Many Oct-2 experiments
that used $A/$B or multi-word $v in interactive zsh were invalid (runs started from spawn etc.). Use the bash wrappers
runA.sh / runB.sh / queue.sh (bash scripts word-split correctly) or explicit args.
Valid results so far: sweepA/expB (bash), padopt=0.5+firerange 600+projkey => 179, the fine-aim stack (178), A_r1-like runs with explicit args.

## Exit stack reproduced (fine aim)
Pre-fire at tick 14 (on the tracked climb runs/trk.txt), aim 259.2 deg -> explodes on the RIGHT FACE of the L-arm end
(5247,~1930) at tick 40; point-blank left shot at 40 => vx 5 -> 28.2 (|v| 32) = Teero's exit. Window is ~0.1-0.3 deg wide:
the 64-angle shot set can never find it. Needs edge-refined (bisection) aims for long shots.
Even with it, A = 178 (Teero 161): after the exit our line climbs early (geodesic shortest path toward the arch);
Teero stays low, lands on the block (184,60) at ~33 px/t, kicks off it hooked up, then keeps 35-43 disp through the arch.
- macOS: never cp over an executable that has run (SIGKILL 137 from stale code signature); rm first.

## Progress log (Oct 2 evening)
- best_58.94.txt (pre1 + post1), best_57.66.txt (pre1 + post2 kcredit=1.5 kready=10),
  best_57.08.txt (pre4: es0.6 repeat=1 macro 1..28 -> grenade 1095; post4 kcredit=1 kready=10).
- Working levers: kcredit (point-blank kick potential credit, kready=10): A 181->174, B 1771->1763, full -64 ticks.
  repeat=1 + energyshare 0.6 + finer macro holds pre-grenade: grenade 1111 -> 1095.
- No effect / worse (valid runs): padopt, pendshare, pendroll, padaims (edge-refined), jumpbonus, rollout,
  dynvref, alpha 16/24, energyshare 0.6 post, hookpen (hook braking accumulator), bandshare, crashpen, bendcredit,
  fine hook angles (C: -6, C1: +6), commitfire (A -2, full +7).
- Energy accounting: post-grenade hook braking -26..-33k v^2 vs explosions +21k; pre-grenade hooks -2.7..-3.7k.
- Corridor 1 from a Teero-like crossing (900,465,25,-13 spent): 294-296 (Teero 298). Our pre-start crossing gives ~306.
- TileExists lookup cache in CCollision (opt-in flag, TAS only): step 1.54 -> 1.20 us, identical replays.
- chain.py: segment chain over distance gates with several configs (gains ~1-2 ticks/gate; slow).

## Corridor 1 beaten (session of Oct 2, night; Linux VM, 4 cores)
Result: **296 race ticks** from the start line to x > 8800 (Teero 298), from spawn, no teleport:
`c1_best.txt` (one input line per tick from spawn, ends just past x = 8800). Checked on real server code
(`TasReplay.Run` with `TAS_TRACE=1`, `./srvcheck.sh c1_best.txt`): start tick 73, x > 8800 at race tick 296, no freeze,
no double start; positions and velocities equal to the simulator on all 369 ticks.

### Setup fixes
- Fresh `setup.sh` failed: `generated/server_data.h` missing (TAS targets didn't depend on it) and `g_pData` undefined
  (`server_data.cpp` was in a `file(GLOB ...)`, which finds nothing before it is generated). Fixed in `CMakeLists.txt`
  (explicit sources -> order dependency). `setup.sh` now also builds `pre`.
- On this VM `/usr/local/bin/python3` (3.11) picks up the distro's 3.12 numpy/PIL and fails; plotting used a venv.

### q14: corridor time vs crossing state (teleport, runT.sh)
| tp (x,y,vx,vy,jumped) | E = v^2 - y | ticks |
|---|---|---|
| 900,250,30,0,spent | 650 | 271 |
| 900,300,28,0,spent | 484 | 287 |
| 900,465,28,0,air jump left | 319 + J | 289 |
| 900,465,27,-13,spent | 433 | 292 |
| 900,350,27,0,spent | 379 | 295 |
| 900,420,27,-5,spent | 334 | 299 |
| 900,400,26,5,air jump left | 301 + J | 302 |
| 900,465,25,0,air jump left | 160 + J | 306 |
Energy is the lever (~0.08-0.1 tick per unit of E); height matters much less than feared (the earlier NONE entries for
g350/g420 in T.res did not reproduce: 295 and 299 now). Single searches vary by +-3-5 ticks, so read trends, not single rows.

### Pre-start energy cap (why "Teero-like tp" was too optimistic)
- The hook only accelerates while |v| < 15, so hook energy is capped at 225 - y_top = 147 (ceiling, y = 78).
  Jumps add 13.2^2 = 174 (ground jump at vy = 0) and 12^2 = 144 (air jump at vy = 0).
- Teero's route (ceiling, fall, swing, air jump on the line): at most 147 + 144 = **291**. Teero's track suggests
  E rising during his free fall (147 -> ~205), which the physics can't do; most likely a few % scale error in the video
  registration.
  So `tp=900,465,25,-13,3` (E = 329) is ~40 E richer than Teero's real crossing; from our own crossings the old tas
  corridor search gets 303-311, i.e. it was *not* already beating him.
- Floor-edge route (ceiling, tangential landing at the spawn floor edge x ~ 650, ground jump): 147 + 174 = **321** plus an
  unused air jump, but it crosses high (y ~ 220, rising) and the air jump is wasted (block 2 refills it before there is a
  safe vy ~ 0 moment under the sloping freeze ceiling). Best found: E 298 + J (`runs/pre/co0p.txt`).
- Two rooms (spawn room + room below via the shaft) don't help: at E >= 147 the turn radius (~v^2/3) is larger than the rooms,
  so only one left-to-right pass is possible.

### Corridor 1 structure (energy view)
Bare blocks refill both jumps on a one-tick touch: block 1 x 1426-1614 (stand y 466, right after the line), block 2
x 2208-2272 (y 594), block 3 x 3616-3712 (y 338), x 4576-4736 (y 498), x 5952-6112 (y 562-594), x 6688-6784 (y 594).
Teero touches block 2 (k ~ 57), ground-jumps, air-jumps at k ~ 75. A ground jump off block 1 runs into the freeze ceiling
that slopes down to y 480 by x 2300 (that trap cost the early gate experiments 175 E).
The old tas search wastes energy: it slides over blocks without jumping (its energy share credits a grounded state with
2 x jumpenergy = 400 > the 174 a jump gives), lands non-tangentially and brakes with hooks. Lowering jumpenergy (100/140)
did not help (b0: 311 vs 303-311 with other configs; Teero tp: 303 vs 294-296).

### New tool: `pre` (ddnet/src/tas/pre.cpp, built by setup.sh)
Wide multithreaded beam with per-cell dedup and a global dominance table (a cell reached earlier with a better value
prunes later visitors). Modes:
- Pre-start crossings: ranked by E = v^2 - y (+ jw/gw credit for unused air/ground jumps); every start-line crossing is
  recorded (survival-checked); `rank=cont` ranks crossings by the energy left after a short surviving continuation;
  `ymin=` keeps only low crossings. Writes `out<k>.txt` prefixes.
  `pre AiP-Gores.map beam=30000 maxticks=150 threads=4 rank=cont ymin=455 out=runs/pre/vl top=20`
- `polish=FILE seconds=S [ymin=]`: hill-climb a pre-start prefix for the crossing value; `eval=FILE` prints it.
- Post-start (corridor) mode `gatex=8800 gatelambda=0`: keeps searching after the line and records gate states.
  Post-start ranking: `lambda*E_eff + x/vref - race_tick` (E_eff = E + pjc*air jump + pgc*grounded), or the time model
  `trref=teero_track.txt`: race_tick + T_rem(x, E_eff), where T_rem integrates ds / disp(sqrt(E + y_ref(x))) along
  Teero's smoothed line; `hnow=H` covers the next H px at the current horizontal speed (this was the step that worked:
  being high and slow costs time now, which the pure energy model can't see). `postdir1=1` (only hold right after the
  start; every input of our corridor runs is dir 1) makes it ~3x faster. `ghostshare`/`ghostmu`, `lamdecay`: no gain.
- `runH.sh NAME PREFIX [extra]`: corridor run with the energy beam from a prefix (results in runs/H.res).
- `lns.py BEST [minutes] [workers]`: iterated re-search: cut the best run at a random race tick, re-run `pre` from there
  with a random variant, keep verified improvements (state in runs/lns/, log runs/lns/lns.log).
- `polish ... race=1`: polish now can judge by race ticks (so pre-start inputs can be mutated too). It found nothing on
  the 300 run.

### Corridor 1 results (race ticks to x > 8800)
- old tas search from our crossings: b0 303 (lucky; same prefix 309-311 with other configs), co0 308, lo0 306, lo2 307,
  gate prefixes after block 1: 332-347.
- energy beam, lambda 0.09: co0p 301, lo2 300 (`runs/pre/h_lo20.txt`), lo0 307, full spawn-to-gate run 304.
  Wider beam (60k) 304, ghost share 302/305, time model without hnow 302.
- re-search lo2's run from race tick 95 with `trref hnow=300`: 299 (skips the block-3 ground jump, stays low, uses the air
  jump at x ~ 4500; that section went from -4 to -1.7 ticks vs Teero).
- low crossing `runs/pre/vl1p.txt` (y 456, v (24.6, -11.5), E 279, jumps spent) + `trref hnow=300`: 297 (server-checked).
- iterated re-search (lns.py, `postdir1=1`, mostly `trref hnow`) from the 297 run: **296** (cut at race tick 121,
  `postbeam=45000 angles=64 trref=teero_track.txt hnow=300`); ~35 more jobs (incl. a reference line from our own run, `own_track.txt`) gave 296-305, nothing lower.
- negative: crossing searches with more beam/angles found no better low crossing (best E 281 at y 463, `runs/pre/xl0.txt`,
  which then ran 302-303); polished lo2p (E 288, y 435) ran 307; vl1p with postbeam 45000 / angles 96: 298.
Per-section comparison with Teero (vsteero-style, by his track): we lead after the line, lose ~1-3 ticks before block 2
and ~1.5 between blocks 3 and 5, gain back late with more energy (E ~ 850 at the end).

### How to reproduce (from tas-work/)
```
../ddnet/build-sim/pre AiP-Gores.map beam=30000 maxticks=150 threads=2 rank=cont ymin=455 out=runs/pre/vl top=20
../ddnet/build-sim/pre AiP-Gores.map polish=runs/pre/vl1.txt out=runs/pre/vl1p.txt seconds=150 threads=1 ymin=455
./runH.sh vl1p_h300 runs/pre/vl1p.txt trref=teero_track.txt hnow=300 threads=1     # 297 (runs/H_vl1p_h3000.txt)
python3 lns.py runs/best297.txt 60 4                                              # 296 (runs/lns/best.txt)
./srvcheck.sh c1_best.txt        # real server: start tick, race tick at x > 8800, freeze/double start
```
The crossing searches above ran with threads=2 before the beam sort got a deterministic tie-break, so a rerun can give
a different vl1; the exact prefixes are kept in runs/pre/.

### Next steps
- Pre-start for the full run: the crossing is now `c1_best.txt`'s first 72 inputs (vl1p, low Teero-style). The energy beam
  + time model should carry over to the pre-grenade segment, but `pre` measures progress (and T_rem) by x; segments that
  go left or climb need progress as arc length along a reference line (Teero's track) and a gate at the segment end.
- Teero's crossing is ~291; ours is 279: a better swing at the bottom of the shaft is worth ~1 tick.
- Corridor: more lns.py time; a reference line built from our own best run instead of Teero's may calibrate T_rem better.

## Full run < 50 s attempt (Oct 3; goal: < 2500 race ticks, Teero 2536)
Tools added:
- `seg` (ddnet/src/tas/seg.cpp): energy beam for any segment, progress = arc position on Teero's track (forward-only
  tracking with a velocity-direction penalty), gate=K (Teero tick) | grenade | finish. Time models: `ghost=1 hnow=600
  ghoste=0.02` (Teero's remaining time + current-speed term − energy credit) or energy table + `sinks=`. `commitk=K` also
  writes OUTc.txt cut at Teero tick K. `fire=0` disables shots. Usage at the top of seg.cpp.
- `rh.py START NAME [window=250 commit=100 nvar=4 sel=0.02]`: receding-horizon chain with seg (state runs/rh/NAME/,
  resumable). Gate choice: rt − sel·energy.
- `pf` (ddnet/src/tas/pf.cpp): brute-force pre-fired shot (0.05°) + point-blank follow-up (1°) on a fixed base,
  scored by estimated time to the line dot(pos,dir)=xf. Uses NextExplosion (stage 1 needs no simulation).
Findings:
- Map is a serpentine with continuous walls: no route skip. Teero picks up the grenade at 983, then 1553 ticks.
- seg corridor 1 is ~3-4 ticks worse than pre+lns (chain from vl1p: x>8800 at rt 300 vs c1_best 296).
- Right U-turn + return leg (chain b): lead vs Teero -2.8 (k300) -> -4.5 (k340) -> -7.7 (k450); steady ~0.4 tick/10 on
  the return leg at |v| 31-33 (Teero's track displacement 27 vs ours 26 px/t). Teero's track may be biased (its end
  at 2536 is 44 px left of the finish tile).
- Post-grenade: Teero arrives at the grenade at |v|~36 diagonally, brakes to ~0 at 984 and climbs at 13 px/t (hook
  limit), exits the 3-wide gap (x 165-167, rows 60-61) at k1020-1023 with a double kick. Our old arrival (post_cut,
  pickup rt 1095 with vx +10) wastes ~10 ticks reversing; hook-only climb reaches the gap at +46 (Teero +37).
  pf on that climb: best exit vx 22 at +58 (Teero is ~10 ticks ahead); x00 (shot-boosted climb) is better (+5 ticks).
- p20 (old pre-grenade, seg after pickup): -16 vs Teero at k1144 (shaft exit ~-8, then ~-0.08 tick/tick at 37-54 px/t).
- **seg bug (fixed Oct 3):** the "can't pass the pickup without the grenade" rule used the LAST grenade entity on the
  map, which is in the finish room (gren idx 2511), so it never fired: searches starting before the pickup could
  skip the grenade (all hook-only climbs from pre1112 did, reaching k1036 "early" at rt 1122). Now the first grenade
  near the reference line (idx 984). seg's start line prints `gren idx N has 0|1`.
- **Post-grenade hairpin (k1144-1170, right end, 4-tile channel x 292-295 with freeze walls):** seg from x00 to gate
  1310 died (NOGATE) with ghost=1, ghost=1+sinks, ghost=2: the beam fills with 55-60 px/t states (Ee ~2000-2300)
  that can't brake. ghost=1's speed term divides by Teero's local speed, which is tiny where he brakes, so fast states
  get huge credit; ghost=2 (our time over the next hnow px vs Teero's own time there) didn't help either.
  ghost=3 (cap our lookahead speed at vcap x Teero's local speed, penalty brakepen x excess when |v| can't be braked
  at `brake` px/t^2 in velocity space before his slow points) with hnow=1000 survives the hairpin.
- ghost=2/3 on the pre-grenade right turn window (c2 -> 451): 459 = same as ghost=1 (458-460).
- Diagnostics added to seg: `tp=x,y,vx,vy tpk=K [tpreload=N]` (teleport after the prefix, race clock = Teero tick K;
  NOT for final runs), `SEG_DUMP=1` env (per-tick trajectory lines "D rt ... pos vel E in hook jumped gr reload proj").
- **Post-grenade corridor k1033->1144 from Teero's own state** (pos/vel from his track): 1153-1155 with our reload
  phase (5 ticks later than his), **1145-1146 with his reload phase (tpreload=13)** vs Teero 1144. So seg's local
  corridor play is about at par; real runs lose through worse states: exit speed (ours ~23-24 px/t vs his ~31) and
  reload timing (the shot clock decides whether a kick is ready at the block he kicks off).
- Teero's line there skims the freeze floor (rows 57-59) and kicks off the bare block at x~185; seg without guidance
  jumped into a high arc (rows 52-53). `latpen` keeps it near the line; `kcredit=2 kready=10` (kick potential as
  energy credit, ported from tas) is worth ~2 ticks with our reload phase, 0 with his.
- **Pre-grenade from Teero's k401 state** to k651: 659-660 vs 651 (seg ~3.5% slower than Teero here even from his
  state). From our chain state: 672-686 for ghoste 0.02/0.05/0.08, ghost=4, tracking (track=1): no setting helps;
  the beam saturates. Teero's energy profile shows jumps timed with ground touches (+130..+380 at k404, 492, 532, 616)
  and near-lossless flight in 400-530.
- Time-indexed tracking (`track=W`): follows Teero within ~10 px while physically possible, but can't keep up from a
  weaker state; no gain on the windows tried. A tracking climb reproduced his exact exit stack (pre-fire ~261 deg +
  point-blank off the L-arm) but late.
- Chain c (rh.py vset=g3 from chain b's c4): lead vs Teero -22 (k651), -27 (k751), -31 (k851).
- pf exit stack on line-hugging climbs (latpen): best score 1240.4 (est. tick at x=5900; Teero-equivalent ~1236).
- From Teero's k401 state to k651 (his 651): baseline 659-660; beam 40000: 660; angles=128 (one aim per tile): 670;
  **angles=128 hookdedup=0 (all aims, new option): 657**. Finer hook aiming helps a little.
- Chain c reached k901 at -41 (commit c9.txt). gren.py from c9: climbs reach the gap on Teero's schedule (-47 at the
  pickup, -46.6 at the gap), but the exit is ~23 px/t (Teero ~31) and the first corridor loses ~20 ticks:
  best -67 at k1100 (runs/gren/c9/pf3_2_g_0.txt). pf finds "shoot straight up, explode at the apex" stacks, not
  Teero's L-arm stack (our climb passes ~1 tile right of his line). Tracking climbs from c9: much slower (no good).
- Post chain p1 from k1065: all variants NOGATE to k1315. Beams die after the hairpin in the V dive k1260-1300
  (Teero brakes from ~64 to ~34 px/t at the bottom, k1285). rh.py now gives up after 4 failures in a row (it used
  to loop forever when the stack had one element).
- `prefire=1` (new): shots still in flight after `firelook` are judged as if not fired (the old held-input look
  penalized every pre-fire), and the cell key includes the pending explosion (pos/24 px, ticks to go) so different
  pre-fire aims stay apart. With `padaims=48 padtop=500 padrange=30` from c9 to k1100: 1165 vs 1167-1168 (small gain).
- Post chain p2 (vset=g4: survevery=2 survive=30 beam 10000): -97 at k1315, -142 at k1415 (selected variant was 8
  ticks slower at the gate than v0 because of the energy bonus sel=0.02; after the grenade energy is cheap, use a
  lower sel). k1330-1375: our path went over the island (rows 57-58) and dived to row 77 while Teero stays at rows
  63-68 (~25 ticks lost). Survives() rollouts never fire, so strict survival pruning can remove states that would
  survive with a kick (Teero's hairpin/dive moves need kicks).
- Status (Oct 3, 08:00): best complete run still 57.08 s (old). Pipeline pieces: pre-grenade chain c (-41 at k901),
  gren.py transition (-67 at k1100), post chain p2 in progress (-142 at k1415). Projected total ~2750-2800.
  < 2500 needs beating Teero by 36+; seg is ~3.5% slower than Teero pre-grenade even from his own states.
- `shotref=teero/catalog/shots.tsv` (bonus for explosions near Teero's): worse (1583 vs 1545 on k1165->1415). Off.
- p2 post chain: -173 at k1615, -183 at k1715 (seg loses ~20% after the grenade from our states). Stopped.
- **Old tas to the finish** from our new grenade state (runs/gren/c9/best_cut.txt, k1065 at -62) with the old post4
  settings (beam 1000 vref 25 energyshare 0.4 jumpenergy 200 survive 15 firelook 1 stencil16 3 repeat 1 kcredit 1
  kready 10): **2860 ticks (57.20 s), server-checked** (runs/tpost/a.txt). Old best 57.08 still better: tas lost
  ~45 ticks against its own estimate in the last third.
- lnstas.py: LNS on a complete run with tas (random cut after cutmin, randomized settings, keep finish improvements).
- **LNS (lnstas.py t1) on the 2860 run:** 2853 (cut 2542), 2834 (cut 1676), 2827 (cut 2114), **2808 (cut 1612, beam 2000
  kcredit 1.5 kready 12 energyshare 0.4 jumpenergy 150 vref 30; 56.16 s, server-checked, tas-work/best_56.16.txt)**.
  Long re-searches from early cuts vary by +-50 ticks, so they're worth drawing.
- **Teero's exit stack reproduced:** pfperturb.py nudges the hook-only climb (dir held for 1-6 ticks between pickup+10
  and +36) and runs pf on each. Nudging left at +28..+34 lets pf find his stack: pre-fire at pickup+16..17 aimed
  258.9-260 deg + point-blank off the L-arm at +42 (aim ~187 deg). pf now also requires the result to survive 15 ticks
  under simple rollouts (`survive=15`); without it the best stacks (28.5 px/t) exit into the row-60 freeze.
  With survival: exit 25-28 px/t, est. 1170.3 vs 1172.1; seg from it reaches k1100 at **1163** (was 1165-1168).
- The work is saved in the GitHub repo (branch claude/fervent-cray-0pser4, folder aip-tas-work/) via sync_repo.sh.

## Corridor 1 vs Teero, measured (session of Oct 3)
User's video measurement (teero/hpos_first7s.csv, hspeed_first7s.csv; compare with `teerox.py RUN`): Teero leads
c1_best by -17 px at race tick 20, -77 at 60 (flat to 120), -120 at 200+; his x > 8800 is ~293 in our clock.
### Two facts that explain it
- **Positions are rounded to whole pixels every tick** (CCharacterCore::Write / Quantize), and the velocity ramp only
  scales x. So free-flight progress is exactly round(vx * ramp(|v|)) px per tick: 22 for vx 24.55 (our arc after
  the line), 23 needs vx*ramp >= 22.5 (vx >= 25.4 at vy 0), 24 needs >= 23.5 (vx ~26.65), 25 covers vx ~28.3-30.1.
  Teero's data is exactly this staircase: 22.1 (rt 0-5), 23.0 (rt 5-30), 23.9 (rt 30-55), 25.00 (rt 55-95, same as
  ours: our extra vx 29.46 vs his ~28.5 buys nothing inside the 25 band).
- **Rotation pulses.** A hook fired at a solid tile within the first-tick reach (42..122 px along the aim) grabs and
  pulls in the same tick, exactly along the aim; release next tick, fire again the tick after. Above 15 px/t the
  pull only applies if |v| does not grow, so the best aim is on that boundary: a near-lossless turn of v towards +x.
  After the line (rising at -11.5) pulses at the floor below (aim ~82-89 deg down) damp the rise and add vx:
  vx 24.55 -> 25.76 in 10 ticks, peak y 394 instead of 330, 23 px/tick from rt 5 (x at rt 20: 1446; Teero 1447,
  c1_best 1430). `rotpulse.py PREFIX N [down|up|any]` does this by hand; `pre ... rothook=1` adds these aims
  (boundary aim and 1 / 3 deg inside it) to the search's hook set (the per-tile dedup of the normal hook targets
  can't express them: they need sub-tile precision).
- **Far rotation pulses** (`farrot.py PREFIX amin amax step hold`): fire up-right at a ceiling tile 200-300 px away,
  hold until the first tick the pull is accepted (the anchor reaches ~93 deg from v), release: dvx +0.5..+1.1,
  |v| -0.01..-0.1. The search had these aims but lost them: while the hook flies / waits, the state ties with free
  flight (and with ~30 useless hook variants), so it rarely survives 4-6 ticks of beam cuts.
  `pre ... hookla=10` scores a flying/held hook by a ballistic prediction of its first applied pull (time model
  value change), and a hook predicted to never pull loses `hookidle` (0.02 ticks).
- Results (x > 8800, race ticks, server-checked): rothook=1 trref hnow=300 from vl1p: **293** (`c1_rot293.txt`, old
  opening, lead -76 px at rt 60, +1 px at rt 276); pulse opening (runs/c1x/rp80.txt) + rothook: 295; + hookla=10:
  **293** (`c1_la293.txt`, lead -2 at rt 30, -33 at rt 60, -3 at rt 276). The pulse-opening runs land on block 2 with
  vy 12.5 (E 272 -> 229) instead of turning the fall first (c1_rot293 keeps E 271 -> 442 after the jump).
- lns.py: env LNS_DIR, LNS_ROT=1 (rothook in every variant), LNS_LA=10 (hookla, forces the time model),
  LNS_MINCUT=rt (never cut before that race tick).
- **x > 8800 is a bad corridor-1 gate**: c1_292 reaches it flat at 40 px/t, y 724, air jump spent, ~190 px from the
  freeze wall of the up-shaft (x 8800-8992) -> can't turn. The section really ends in the right shaft (x 9312-9472,
  falling): Teero y > 450 there at ~334.5 (track shifted to the user's clock), our 56.08 run ~341. `pre gatey=Y`
  (gate needs x > gatex and y > gatey; past gatex states are ranked by the time to fall to gatey) - but pre's beam
  dies at the up-shaft wall from the corridor (all states fast and flat). Use seg (ghost model) for the turn.
- User's checkpoint (Oct 3): no testing past the grenade pickup until we beat Teero there (~rt 982-983).
  User data: teero/teero_inputs_pre_grenade.csv (dir / jump / aim per video frame up to the pickup).
- seg ports: `rothook=1` (rotation pulses towards the reference tangent), `hookla=N` (pending-hook scoring),
  `quant=1` (speed terms use the whole-pixel displacement per tick). k401->651 from Teero's state (Teero 651),
  ghost=1 hnow=600 ghoste=0.02 angles=128 hookdedup=0: base 657; +rothook 655; +hookla 660; +both 666;
  **+rothook+quant 653**; +rothook+quant+hookla 656. -> in seg use rothook=1 quant=1, no hookla.
- `lagtrack.py RUN [from to every]`: lag behind Teero's track by nearest point (k frame). Chain c: +5 at rt 375,
  +22 at 675, +38 at 925 (~6% slower on the return legs from our own states).
- rh.py `vset=r` (rothook+quant variants); chain r1 from c1_292 cut at rt 200, stopk=880.
- **Chain r1** (rh.py vset=r from c1_292 cut at rt 200; leads in Teero-track ticks): k452 +0 (old chain b -7.7),
  k552 -1 (b: -13), k652 -3 (c: -22), k752 -3, k852 -9 (fastest variant -4, energy selection sel=0.02 picked a
  slower one), k952 -15. Commits runs/rh/r1/c1..c6 (c6: rt 808 at k802). Stopped before windows cross the pickup.
  Losses: ~1% up to k802, ~6% in k802-952 (lagtrack: +4 at rt 780 -> +14 at 960).
- Clock check: the user's input csv has s_since_start = video - 1.383; the race clock that matches our c29 replay
  is video - 1.4467 (teerox T0), i.e. 3.2 ticks less: Teero's pickup (data end, s 19.667) ~ rt 980.
- **Pickup from chain r1's c6 (k802)**, gate=grenade, 4 variants: 998 (ghost=1 hnow=600 angles=128 hookdedup=0 and
  hnow=300 ghoste=0.01), 1001, 1003. **pregren_998.txt: grenade pickup at race tick 998, server-identical replay**
  (start 73, same final position, no freeze). Teero ~980 -> 18 behind (old pipeline ~47).
- k800->950 benchmark from Teero's state (tp=1642,1393,23.15,-0.16 tpk=800, Teero 950): 955 base; beam 40000 /
  angles 256 / hnow 300 / hnow 1000 / survive=8: 955-956; cpos=8 cvel=1: 954; dirmode=tan: 955; tan+cpos8: 954.
  Loss is at the loop k896-928: we brake at the top (dir 0/-1, vx 23.7 -> 15.8), Teero keeps dir 1 and dives along
  the slanted freeze face with ~4-5 px margin (tee centre), ~25 px closer than us; both then turn the fall with
  up-right pulses (his aims -34 -> -90 deg).
- `imit=teero/teero_inputs_pre_grenade.csv` (seg: dir / jump / hook aims restricted to Teero's within +-imitw ticks,
  race tick = s_since_start*50 - 3.2): 964-981 (much worse); with the normal hook targets too: 957. Not useful.
- `lnsseg.py BEST DIR minutes= workers= cutmin= rt=`: LNS to the pickup (gate=grenade), random seg variants.
- Energy vs Teero (by nearest point): equal within +-90 up to k700; the far-left U-turn (k700-740) costs -112 and the
  deficit grows to -179 by k900 (lag +1 -> +12). Cause: we land on the corner of the bare block (x 256-288, top y 1728)
  at vy 16.8 (rt 734, v^2 -280); Teero turns the fall earlier with a long held hook (aim -59 deg, k720-733) and passes
  ~5 px over the corner at vy ~8, then skims ~26 px above the freeze floor (y 1856).
  Window rt 690 -> k790: base 797 (lands), pjc=pgc=40 797 (lands), ghost=3 796 (lands), **dirmode=tan cpos=8 cvel=1: 797
  without landing, Ee -695 vs -865**. Pickup searches from that state (cut rt 760): 996 (tan+cpos8), 997, 1001, 1002.
  **pregren_996.txt: pickup at race tick 996.**
- LNS (lnsseg.py, 4 workers, 70 min): 998 -> 997 (cut 957); early cuts (595-761) gave 1001-1009.
- Settings are section dependent: k401 benchmark with dirmode=tan cpos=8 cvel=1: 657, cpos=8 only: 659 (base 653).
- Chain r2 (vset=r2: 8 variants incl. tan+cpos8 and hookla, sel=0.01, from r1's c1 at k302): k552 553 (same as r1),
  k652 653 (r1 655), but k752 759 (r1 755): the lower-energy commit at k502 cost 4 ticks later. Stopped.
- LNS Q on pregren_996 (cutmin 690, 4 workers). dirmode=tan dies inside the left U-turn (cut rt 723: NOGATE).
- True lag vs Teero ~ track-frame lag + 3 (track labels are ~3 ticks late near the corridor end / pickup).
- k800 benchmark with latpen: latpen=0.02 latdz=16: 956; **latpen=0.05 latdz=24 cpos=8 cvel=1: 953** (best). Added to
  lnsseg.py variants (35%).
- ecmp.py RUN [k0 k1 step]: energy / speed vs Teero by nearest point. pregren_996: energy within +-60 of Teero after
  the U-turn fix, yet lag +6 (k760) -> +12 (k960): the rest is path length (Teero's tighter lines), not energy.
- k800 benchmark: survevery off 953, beam 60000 953 (identical path) -> beam is not the limit. track=1: 958-975
  (can't follow Teero from the estimated state) -> his k800 state is probably faster than the estimate; these
  from-Teero benchmarks are only good to ~+-3 ticks.
- LNS Q (cutmin 690, 13 jobs): no gain over 996. Chain r3 (vset=r3: 8 variants incl. latpen / cpos8 / tan, sel 0.02)
  from r1's c1 (k302), stopk 790.
- **Chain r3** (vset=r3, sel 0.02, from r1's c1): k552 550 (+2; r1 553), k652 647 (+5; r1 655), k752 753 (-1; r1 755),
  k852 855 (-3), k952 963 (-11; r1 967); commits c1..c5 (c5: rt 807 at k802 - same as the 996 run there).
  Pickup searches (gate=grenade) from c5 / c4: 994 (from c4, dirmode=tan cpos=8 cvel=1), 995 x3, 996, 997, 1000.
  **pregren_994.txt: pickup at race tick 994, server-identical (start 73, no freeze).** Teero ~980.
- pregren_994 vs Teero (true lag = track lag + 3): corridor+turn ~+1, k302-690 +3 (total +4), far-left U-turn +5 (+9 at
  k775), k775-pickup +5 (+14). In the U-turn we follow his exact path ~4 ticks late; he starts turning right ~1-2 ticks
  sooner (down-right hook until ~k719, then up-right -59 deg) and clears the block corner; we touch it at rt 740
  (vy 7.9). No single big mistake left - the remaining gap is spread out.
- LNS R on pregren_994 (cutmin 250, 6 h, 4 workers, variants incl. latpen 35%).
- LNS R: 994 -> **992** (job 8, cut 891). pregren_992.txt, server-identical.

## Map switch: KoG version (Oct 4)
Teero played the **KoG** version of AiP-Gores (user, from Teero). Our runs so far were on the DDNet version.
- `tas-work/AiP-Gores.map` is now the KoG file (sha256 353b27cf...); DDNet version kept as maps/AiP-Gores_ddnet.map
  (map.txt likewise; maps/kog.txt, maps/ddnet.txt, maps/diffmap.txt: + solid only in KoG, - solid only in DDNet).
- Differences (game layer): KoG has 9355 extra solid tiles (mostly sealed fills: above ceilings, inside blocks /
  pillars), 21 DDNet-only solids removed, spawns at (4,6),(8,6),(16,6),(4,18),(8,18),(16,18) instead of rows 10/24
  (Teero's start x 144 = spawn (4,6)), start line also at (29,5) and (29,15) (freeze removed at (28,5),(28,15)), finish
  2 columns, no front layer, map setting `sv_kog_map_quests Q_NO_HAMMER,Q_NO_PLATFORM,Q_TEAM_15`. No tuning commands.
- Hook rays from Teero's path differ in k348-428, k546-576, k632-662, k896-960; small blocks moved in the k896-960 loop
  (DDNet-only solids at (132,54),(133,55) under the dive exit).
- The DDNet best (pregren_992) would replay identically on KoG up to rt ~400 (first different hook anchor at rt 401),
  but the spawn differs: `match` tool (beam search for an exact target state, here the ceiling contact at t=23:
  200,78 v 11.8359375,0) found no exact match -> pipeline redone on KoG.
- **KoG start line: you can touch the solid floor at (28-29, row 16) right at the line** (freeze removed at (28,15),
  (29,15) is a start tile): land there, ground jump (-13.2) and keep the air jump. Crossing search (pre rank=cont
  ymin=455): vl0 E 274.9 jumps 1; polished vl0p E 299.5 (24.68,-13.20), vl1p E 288.2 (24.45,-12.70), both with the air
  jump left (DDNet best: E 279, jumps spent). Teero's track y ~496 at the line = standing on that floor.
- Corridor (pre gatex=7200 trref hnow=300 postdir1=1 rothook=1 hookla=10) from vl1p: **ahead of Teero** (user data):
  +6 px rt 100, +16 rt 140, +36 rt 180, +44 rt 200, +51 rt 220 (kog_c1_gate7200.txt); from vl0p -1..-20.
- **Server-check harness bug (fixed):** TasReplay killed the two extra debug tees, but killed players respawn at a free
  spawn point; on the KoG map that is (8,6)/(16,6), in the spawn room's flight path, and our hook caught a dummy at
  t=29 (sim/server divergence). They are now moved to spectators (no respawn). KoG c_vl1p0: server start 68, x>7200 at
  rt 237 = sim. (DDNet-era checks were unaffected: identical to the sim.)
- Chain k1 (rh.py vset=r3 from runs/kog/k200.txt = vl1p corridor cut at rt 200), stopk 790.
- KoG chain k1 (vset=r3 from runs/kog/k200.txt), leads in Teero-track ticks (true = track - 3): k455 453 (+2),
  k555 553 (+2), k655 651 (+4), k755 756 (-1), k855 864 (-9; fastest variant 859, energy selection again), k955 969.
  Commits c1..c6 (c6: rt 813 at k805). Loss at rt 750-810 (climb out of the U-turn).
- KoG pickup searches (gate=grenade, no energy weight) from c4 (k605) / c5 (k705): **992** (c4, base settings), 994, 997,
  997, 999, 1000, 1001, 1009. **kog_pregren_992.txt: KoG pickup at race tick 992, server-identical** (start 68).
- LNS on KoG (lnsseg.py kog_pregren_992.txt runs/lnsK cutmin=250).
- KoG LNS (lnsK, 39 jobs): no gain over 992. Lag profile of kog_pregren_992 (track frame): corridor -6, **turn over the
  pillar rt 300-325 loses 4** (we brake at the top: dir -1/0, |v| 24 -> 12 at x ~9186; Teero keeps dir 1 and ~19 px/t
  to x ~9230, aim 37 deg held k298-320), k550-600 -3, U-turn -2, k775-925 -7 (energy deficit up to -150).
- Turn window (cut rt 280, gate k362): ghost=1 base **357** (current run 359), ghost=3 358, ghost=3 brake=2 359,
  beam 40000 + latpen 362. Chain k2 from that (runs/kog/turn357.txt), sel 0.005.
- **Physics agent report: physlab/FINDINGS.md.** turn357 is within ~1 tick of the physical limit (vertical
  deceleration cap 1.4/tick while rising; the pillar-face anchor can't pull before x~8960); remaining turn gap comes
  from the corridor -> left-shaft entry (rt 276-296). rt 730-930 loss = horizontal speed (21 vs 20 px/t steps),
  the rt 736 landing (-292 E), ceiling bumps rt 816 / 898, vx turned into climb at rt 773/775. New mechanics: hover
  grounding (5 px), convex-corner full stop, rotation 2.5x faster while falling. Teleport jump-state bug in
  tas/polish/nest/tas2/lab (not seg; final runs unaffected).
- Chain k2 (from turn357, sel 0.005): k612 at rt 604 (+8 track = ~+5 true), commit rt 456 at k462.
- Chain k2 steps: k612 604 (+8 track), k712 702 (+10), k812 807 (+5), k912 908 (+4); commits c1 (rt 456 k462), c2 (554
  k562), c3 (653 k662), c4 (754 k762). Stopped before a window crosses the pickup. Pickup searches: runs/kpick2.
- **KoG pickup searches from chain k2: 979 (from c4, ghost=1 hnow=300 ghoste=0.01), 980 x3, 981, 988, 2 NOGATE.**
  **kog_pregren_979.txt: grenade pickup at race tick 979 on the KoG map** - server-identical over all 1047 ticks
  (spawn 144,208, start tick 68), no freeze / double start. Teero ~980-981 (user input csv ends at s_since_start 19.667
  "as the pickup is about to happen" = rt ~980.2 on the clock that matched our replays). Post-pickup check: reaches
  Teero's k1005 point at rt 1024 without freeze (server-checked, runs/post/kog979_0.txt).
- LNS K2 on 979: **978** (job 10, cut 851, post-pickup climb check ok). kog_pregren_978.txt, server-identical.

## Post-grenade on KoG (Oct 5)
- Proceeding past the pickup: kog_pregren_978 (978) vs Teero ~980-981 by our timing (user asked to confirm by their
  video timing; not yet confirmed). Full-run target < 2500 ticks -> post-grenade < 1522 ticks from the pickup;
  Teero's post-grenade ~1556 (2536 - 980).
- KoG vs DDNet along Teero's post-grenade path: no hook-ray differences; changed tiles only near the finish
  (k2504-2524). Old DDNet post-grenade findings apply.
- gren.py: GREN_RQ=1 env adds rothook=1 quant=1 to its seg runs. Run kog1 from kog_pregren_978 (gate k1100).
- rh.py vset=p5: 8 post-grenade variants (g4 settings + rothook/quant, kick credit, ghost=3 brake), WIDE likewise.
- Post chain p1 (vset=p5 from post1100, k1100 at rt 1121): step 1 k1350 at rt 1392 (lead -42; v3/v7 best, 27 min for
  8 variants). lagtrack: par k1100-1170 (+21 track), then +1 tick per ~12 in corridor B (k1175-1285, going left):
  Teero's displacement there is ~36 px/t (|v| ~50-60 after his hairpin double kick #5/#6 at k1158/1165), ours 24-31
  (|v| 26-42) for the first ~80 ticks. kicks.py (new) lists fire ticks and kicks: p1 fires every ~25 ticks but
  several shots give no kick (1117, 1176 at the hairpin, 1309, 1334, 1360).
- physlab2 (grenade agent, physlab2/FINDINGS.txt): pickup arrival is the transition loss; braked pickup at rt 981 +
  double-kick exit gives x>=5671 at rt 1036-1038 (Teero ~1033), label 1100 at 1111 with a crude downstream.
  Exit cuts for seg: runs/ex/e1021.txt (1092 lines), e1023 (1093), e1025 (1095). multi.py (new): parallel seg jobs.
- Horizontal displacement limit: D(v) = v * 1.4^(-(50v-550)/2000) peaks at |v| ~119 with 48 px/t (D(36)=29,
  D(50)=36, D(60)=39.7, D(80)=44.8). Teero's corridors run at D ~36; the TAS ceiling is ~45.
- **Exits -> k1200** (runs/ex, 4 p5 variants each, beam 10000): e1021 1210-1213, e1023 1215-1217, **e1025 1208 (v1, v7)**
  (Teero true 1197). e1025v1 lag (track frame): +2 at the exit, +4 at k1100, +5 to the hairpin, +7 at k1200.
- **Equal-state test (tp at Teero's k1220 state, vx -57, his reload phase) -> k1285:** seg 1286-1291 (b7 best, beam
  40000 pending) vs Teero 1285. tpreplay.py (new; lab has a new `reload N` command) shows b7 tracking him at lag 0/-1
  and kicking to |v| 67 at k1244, but the dive (sink at k1285) eats the extra speed. survevery=0 dies; survive=15,
  vcap=2, ghost=1 no better. => seg is at par with Teero locally; gains must come from turns / slow sections.
- Teero's slow stretches (track displacement < 30 px/t): exit 1020-1040, hairpin 1140-1180, left U-turn 1420-1520,
  1600, 1780-1800, 1940-2000 (16-29), 2060-2080, 2240-2260, final maze 2420-2539 (~20 px/t for ~120 ticks).
- Chain q1 (vset=p5 vidx=1,7,0,3 from e1025v1): k1300 at 1336 (-36), k1400 at 1444 (-44), k1500 at 1554 (-54).
  Step 1 lost 20 ticks at rt 1240-1290: from our (slower, +10) state the beam took a high path over the block at
  x ~6600 (78-118 px off Teero's line, |v| 25). Stopped.
- **Strong line following helps from weaker states:** from q1's rt-1225 state to k1290: latpen=0.1 latdz=32 -> 1310,
  without -> 1330 (q1's own path ~1321). New vset=p6 in rh.py (latpen 0.05-0.2 variants). Chain q2 (p6, window 150,
  commit 75) from e1025v1.
- seg `padref=R` (pre-fire aims exploding near the reference line ~T ticks ahead, for turns): hairpin window
  k1125 -> k1215: 1228 vs 1224 without. Off.
- seg `gate=box:x0,x1,y0,y1[,vymax[,K]]` (new): pickup turnaround (cut 960, box x 5150-5376 y<=2290 vy<=-10): 1013 vs
  the grenade agent's hill climbing 994-995 -> seg can't find the braked pickup; keep physlab2's exits.
- Chain q2 (p6): k1275 1293 (-18), k1350 1377 (-27), k1425 1454 (-29), k1500 1536 (-36), k1575 1617 (-42),
  k1650 1707 (-57). Stopped (losses everywhere; shots every ~25-43 ticks).
- **Hairpin agent (physlab3/FINDINGS.txt):** no new structure; best e1025v1_hairpin_rt1223 (x<=7700 over the column at
  rt 1223, |v| 43; +0.5 tick). The hairpin itself costs nothing; the loss is exit speed (28.6 vs ~38.5) because our
  shot schedule (1100 shot + 1125 floor kick) locks the reload out of Teero's corner slot (point-blank on the L-block
  top at ~1169-1171, +10) and the rt-1098 bump under block (235-236,43) wastes energy (v^2-y 513 -> 14). Teero's
  schedule would be ~3-4 ticks better at the gate. seg suggestions: plan the 25-tick shot schedule against kick slots,
  rendezvous value for pre-fires, full-sim survival, gate on y too. Tools: hp, hcx, gates.py, alive.sh.
- **seg `shotplan=teero/catalog/shots.tsv`** (new; planpre=8 planpost=6 plangap=25 planr=96): a shot is only allowed
  near one of Teero's kick slots (our reference tick within [k-8, k+6]), when his next slot is >= 25 reference ticks
  away, or as a pre-fire exploding within 96 px of his next explosion point -> keeps the reload for his slots.
- **Final-maze agent (physlab4/FINDINGS.txt):** from Teero-like teleported entries (k2390 / k2380) best finishes
  2552 / 2554 vs his 2539 (+13 / +15); from his own track states the search is level with him (k2419 -> top passage
  2447 vs 2445, k2452 -> under the island 2477-2478 vs 2478). Single route; turning radius v^2/3 limits speed to
  18-30 px/t; the up-turn at k2410-2420 throws away the entry speed (|v| 64 -> 30); reload plan matters (previous
  shot at k<=2385, column shot pre-fired at ~k2409-2411, ceiling kick ready ~k2435). Teero's track end is ~3-4 ticks
  inconsistent with his official finish.
  **seg bugs/fixes ported:** tp clock (m_StartTick < 0 -> a finish was never recorded: every tp + gate=finish job was
  NOGATE; now the world clock is advanced by 4000) and `quota=Q [qcell qvel]` beam diversity (keeps the beam alive
  through the maze up-turn; quota=6 finished at 2581 where plain seg died). Tools: fbeam (geodesic distance +
  rollouts with a virtual kick + quota), fhc (exact hill climber), segx.
- Both agents (hairpin, final maze) conclude: local play is level with Teero from his states; our losses come from
  long-horizon state differences (energy, reload phase). No section found where a TAS beats him by a large margin.
- Chain q2 continued to the finish: k1725 1787 (-62), k1800 1872, k1875 1956 (-81), k2025 2130 (-105), k2100 2205,
  k2175 2293, k2250 2375 (-125), k2325 2456, k2400 2535 (-135), k2475 2622 (-147), **FINISH rt 2700 (54.00 s)**.
  **kog_full_2700.txt: first complete KoG run, server-identical (start 68, finish tick 2768 -> 2700 ticks = 54.00 s,
  no freeze, no double start).** (DDNet-map best was 56.08 s.)
- lnspost.py (new): LNS on the complete run (random cut >= cutmin, seg to gate=finish with a random p6-like variant
  incl. quota/prefire/angles, keep earlier finishes). Run runs/lnsP1 from kog_full_2700.
- **Why we lose post-grenade (kog_full_2700 vs Teero at the same track point, every 25 ticks):** our |v| is 5-10
  px/t below his almost everywhere (energy v^2-y ~500 lower), so the lag grows steadily (+5 at k1120, +30 at k1395,
  +64 at k1736, +102 at k1998, +135 at k2390, +164 at the end). eacct.py (new, energy accounting per tick):
  rt 1030-2700: kicks +31,000 (61 kicks), **hook -30,700 over 1138 hooked ticks** (a held hook at |v|>15 only brakes
  or rotates; seg steers along the line with long hook holds), walls -3,600 (4 ticks), dir0 -500, ground -500.
  => steering efficiency (rotation pulses instead of long holds, kicks for steering) is the lever.
- **Kick quality:** Teero's 50 kicks add +725 to v^2 on average (36,000 total; catalog displacements converted to
  velocity); ours +475 (54 fires, ~17,000 total from explosion ticks). Ours are nearly all full-strength point-blank
  (|k| ~12, explode in 0-5 ticks) but often misaligned with the motion (cos 0.2-0.6: kicks used for steering) or fired
  at low speed (|v| 14-30), where 2 v.k is small. Teero is fast and next to a surface when the reload is back.
- Energy weighting test (q2 c4 -> k1650): ghoste 0.05 / ghostsink 3000 / ghost=1 ghoste 0.04 -> 1720-1723 vs 1707
  (base). Not the lever.
- seg `firealign=c` (new: only shots whose kick has cos >= c with the line direction): c4 -> k1650 1711 (c 0.3) / 1712
  (c 0.6) vs 1707. Off. Also tried today without gain: padref, shotplan, box gate, tracking (track=1), bigger beams
  (40000) and 128 aims on the hairpin approach, higher energy weight. Stronger latpen was the only clear gain.
- LNS runs/lnsP1 on kog_full_2700 (restarted after a container restart): first iterations 2705, 2726 (no gain yet).
- LNS runs/lnsP2 (cutmin 2250): **2699 (53.98 s)**, cut 2570, server-identical (start 68, finish 2767, no freeze, no double start) -> kog_full_best.txt.
- LNS lnsP2: **2693 (53.86 s)** (cut 2550), server-identical (start 68, finish 2761, no freeze, no double start) -> kog_full_best.txt. 13 iterations: 2693, 2699, 2704 x2, 2708 x2, 2718-2739, 3 NOGATE.
- **Teero input extraction (user, teero/teero_inputs_0-3131.csv):** per 60-fps frame A/D/dir, jump, aim, weapon, and
  53 fire events (50 audio+recoil, 3 audio), s_since_start -> race tick rt = s*50 - 3.2. He fires on the reload
  (gaps mostly 25-27 ticks); fire aims are unreliable for some shots (exit point-blank read -8 deg vs ~187 deg).
  Pairing with the catalog explosions: fires 1+2 = exit double kick (pre-fire 992.7 + point-blank 1018.5 -> 1018.6),
  then fire n -> catalog explosion n-1; point-blanks match within ~1-2 ticks. teero_shots.py simulates each fire from
  his track position + aim (teero/shots_sim.tsv). Direction usage is like ours (3% dir 0, ~5% against motion).
- **seg plan=FILE** (new): "t0 t1 ex ey r [te tol]" target shots (exact aims by scan + golden section, pre-fires
  included, optional explosion tick) and "t0 t1 free" point-blank windows; no other shots. planforce, planrv (rendezvous
  credit for grenades in flight; no gain: 1230-1232 vs 1228 free pre-fire, 1225-1226 vs 1223 with the plan).
  sim.h: CTasGame caches the next explosion (m_PendE/T). teero_plan.py builds plans from Teero's fires + explosions
  shifted by our lag (per shot from a reference run, or lag=L).
- **Teero's schedule as a plan, exit -> k1215: 1223 vs 1228 without (same settings)** - pre-fire at 1146 exploding
  next to the tee at 1163 + corner point-blank at 1171 (lag +3/+4 through the hairpin). Catalog-only plans (guessed
  fire windows) were much worse (1245-1276). rh.py teeroplan=1 [planwin=3]: per-window plan with the current lag.
  Chain t1 (vset=p7 + teeroplan) from runs/plan1/pc0 cut at rt 1215.
- Plan-mode follow-ups on exit -> k1215 (each a single run, run-to-run noise +-3-5): no plan 1226/1228/1230; full Teero
  schedule (time-indexed, per-shot lag) 1223/1226/1232 -> not systematically better (the 1223 was a good draw);
  hybrid (his pre-fires only + free point-blanks, planfree/planlock) 1242-1244 (worse); position-indexed (planref) full
  1230-1232, next window k1208 -> 1358: plan 1392 vs no plan 1375 (seg's own kick at 1250, +1226, beats his slot).
  Chain t1 with time-indexed plans broke when our lag drifted inside a window (only 3 shots fired). Stopped.
  => Teero's shot timing alone is not what we lack.
- seg crashw=W [crashn=8] (new): penalty for v^2 about to be lost against solids (CTasGame::CrashLoss) - testing.
- crashw (v^2-loss penalty) on exit -> k1215: 1228/1228/1229 (n8), 1239 (n15) vs 1226-1230. No gain.
- seg jitter=J seed=N (new, stochastic beam): exit -> k1215 with 8 seeds: 1227-1231. Window distribution over ~20 runs
  ~1226-1230 (rare 1223) -> best-of-N sampling worth ~1-3 ticks per window. Chain b12 (vset=p8: 4 setups x 3 seeds,
  best of 12 per window) from runs/ex/e1025.txt to the finish.
- Chain b12 (best of 12, from e1025): k1175 1181 (-6), k1250 1263 (-13), k1325 1345, k1400 1421 (-21), k1475 1498 (-23),
  k1550 1583 (-33), k1625 1676 (-51). Stopped (no better than the 53.86 run by k1625).
- **Other agent's fast pre-grenade (branch claude/wonderful-faraday-klv0ms, aip-tas-fast/: exact fast stepper + beam,
  pickup 975)**: identical to kog_pregren_978 up to rt ~789, 2-3 ticks ahead from rt ~900. Copied to
  runs/kog_pregren_975_fast.txt.
- **Transition re-fit agent (physlab5/FINDINGS.txt):** on the 975 prefix (cut rt 955; the prefix has no air jump left,
  so it re-lands on the block top at rt 968 and slides off the corner), braked pickup rt 978, y<=2290 at ~990.8,
  pre-fire step 992, point-blank 1017: **exit_E1017_to1031 (x>=5671 at 1031, exit vx 30.0)** and E1018B (same-step
  double kick, vx 31.45, x>=5671 at 1031.04) - 7 ticks better than exit_T1025 (1038) and ahead of Teero (~1033).
  Server-verified. Reload back at 1042 (block at x 5888 reached ~1039).
- Chains n1 (from E1017 cut rt 1019) and n2 (from E1018B cut rt 1019), vset=p8 vidx 0-5 (6 samples per window).
- **Other agent's 968 pre-grenade run** (user upload, runs/kog_pregren_968_invalid.txt, freezes after the pickup): diverges
  from kog_pregren_978 at line 406 (rt ~338), **9 ticks ahead at the same positions from rt ~890** (978-run rt 900 ->
  891, 950 -> 941, 960 -> 951); air jump spent at ~951. Uses large aim vectors (x10000) for finer angles. Transition
  agent re-fitting the braked pickup/exit on it (cut ~925-955).
- **Transition re-fit on the 968 prefix (physlab5 F lines): exit_F1010_to1024 (cut rt 948, block-top landing rt 961,
  braked pickup rt 971, y<=2290 ~983.8, pre-fire 985, point-blank 1010 -> x>=5671 at rt 1024, exit vx 30.2)** and
  F1011B (same-step double kick, vx 30.73, x>=5671 at 1025). Server-verified. Teero ~1033 -> we are ~9 ticks ahead of
  him at the exit. Chains f1 (F1010 cut rt 1012) and f2 (F1011B cut rt 1012), p8 vidx 0-5. (n1/n2 from E stopped.)
- **segf (new build target): seg on the exact grenade fast stepper.** fastg.{h,cpp} (CFastG: CFast + grenade fire /
  reload / weapon switch / projectiles / explosions, checked tick by tick against CTasGame by fastgcheck: identical
  on the 53.86 run, 0 mismatches in 1000 fuzz trials) + tasfast.h (CTasFast: CFastG with the CTasGame interface seg
  uses). seg.cpp uses `CGameT` (CTasFast with -DSEG_FAST, else CTasGame); prefix replay, SEG_DUMP and commitk stay on
  CTasGame. Also: the collision fast paths were never active (CCollision::ms_FastPaths defaults to false; only the
  other agent's tools set it) -> now set in sim.cpp after InitFast (TAS_NOFAST=1 disables); exact broad-phase skips in
  KickPotential / RotHookAims / HookTargets (no solid tile in reach -> nothing to compute) and trig tables.
  Window test (F1011B -> k1120, beam 2000): seg 84 s -> segf 28 s (~3x), output byte-identical. rh.py / multi.py /
  lnsinc.py use segf (SEGBIN overrides).
- seg `inc=FILE` (incumbent, new): the given run's own continuation is kept in the beam every step (so gate=finish
  can only match or beat it); `SEG_TRACK=FILE` dumps a run as a reference track (k = rt + 3). mapk.py maps Teero's sink
  ticks onto another track. Test cut 2450 -> finish (beam 5000): own-run reference and Teero reference both 2693
  (= incumbent; that stretch was already LNS-optimized). lnsinc.py: incumbent LNS (random cut, ref own/teero mix).
- seg `kfut=W [kfutn]` (new, off): credit for the kick available where a ballistic flight puts the tee when the
  reload is back. f1 window (F1011B -> k1174, latpen 0.2 setup): kfut 1/2 -> 1173-1177 vs 1167-1168 without. Worse.
- Where the 2693 run loses to Teero (vcmp.py / dcmp.py, same place): steadily ~1 tick per 10 in the corridors
  (lead -10 at rt 1200 -> -33 at 1460 -> -85 at 2000 -> -153 at the end); U-turn apexes cost little; S-bend
  (rt 2000-2100, up the shaft at x~4100) -20. In corridor 3 Teero's displacement speed is 3-5 px/t higher throughout.
  Our post-kick peaks are high (55-68) but decay fast; Teero accelerates out of U-turns with frequent well-aligned kicks
  (k1158-1217: 4 kicks, 17 -> ~55 |v|; ours 3 kicks, 19 -> 47).
- f1 step 1 (F1010 -> k1174), p8: non-quota variants 1167-1168, quota variants 1176-1182 -> vset p9 (5 seeds each of
  latpen 0.2/24 and 0.1/32, + 2 quota setups). Chain g1 = f1 resumed after step 1 with p9 on segf. f2 stopped.
- Window tests on segf (single seeds; same-setting baselines in brackets; window-to-window noise is +-3-5):
  firemax=4/10 (new: drop states holding a loaded grenade > N ticks) 1170/1170, 1172/1169 [exit window F1011B -> k1174:
  1172/1169]; erel=1 (new: ghost energy credit relative to the reference's energy) 1171/1173; prefire on the U-turn-1
  window (g1 c1 -> k1249) 1253/1252 [1251/1250]; extended sinks (Teero's speed minima 1976, 2095, 2266, ...) on the
  S-bend window (cut 1960 -> k2080) 2208/2208 [2188/2189]; time-shifted Teero tracking (track=1 trackoff 8/10/9)
  1190/1175/1179 [1172/1169]. None helps.
- **vcap** (ghost=3 cap on the credited speed, x Teero's local speed; default 1.25): U-turn-4 window (cut 1780 ->
  k1830, where the 2693 run kicks twice to |v| 66 and slams into the end wall 54 -> 0): vcap 1.1 1903, vcap 1.0 +
  brakepen 10 1903/1904 vs 1915 (old run 1906). Exit / U-turn-1 windows: vcap 1.1 = base, vcap 1.0 +2-3. S-bend: vcap
  1.1 2194/2195 (worse), vcap 1.0 + brakepen 10 2186 (better). => window-dependent; rh.py vset p10 mixes 1.25 / 1.1 /
  1.0+brakepen10 (x seeds) + quota setups. Chain g2 = g1 resumed after step 2 with p10.
- Loss anatomy of the 2693 run (bigloss.py, hookeff.py): ~18k v^2 is lost braking (hook) or slamming walls within
  ~25 ticks after a kick, right before a bend (kicks at 1125, 1256, 1422, 1836, 2000 -> braking/impacts at 1147-1155,
  1275-1282, 1453-1457, 1844-1849 (54 -> 0 into the wall), 2016-2028 (68 -> 15)). Hook pulls at |v| >= 15: 594 ticks,
  2882 deg of rotation for -21.7k v^2; 325 near-boundary ticks rotate 1338 deg for only -2.6k, 21 ticks lose 10k.
  Teero instead turns with kicks at bends (e.g. S-bend k1945: a braking/turning kick) and arrives slower.
- Chain g1 step 2 (k1174 -> k1249, first right U-turn): best 1250 = 4 behind Teero (was 4 ahead at k1174): Teero's
  pre-fire + point-blank pair at the apex (k1158/k1165) and kicks at k1192/k1217 vs our 1158/1183 then a 45-tick gap.
- More window tests: gcred (loaded-grenade reservation value) 200/500: U-turn-1 1251/1249 [1251], U-turn-4 1917/1946
  [1915] -> off. Taut reference line (tautref.py: Teero's track pulled tight with >= 40/60 px clearance, k labels kept):
  exit window 1185/1170 [1172], corridor 3 (cut 1300 -> k1400) 1437/1433 [1430] -> off. beam 40000: 1171 [1172],
  corridor 3 identical path to beam 10000 -> beam width is not the limit; cpos=8 cvel=1: 1428 [1430]. latpen 0.05/48
  with the apex plan: NOGATE / 1260 [1248-1251] -> strong line following stays.
- **Kick spots match Teero's.** In corridor 2 / U-turn 1 our kicks land on the same blocks as his (5990 vs 5937,
  6857 vs 6931, 7583 vs 7567, 9279 vs 9324, 8463 vs 8504, 7578 vs 7629) with the same strength: the catalog's
  velocities are per 60-fps video frame (x1.2 = per tick: his 41.3 vs our 41.4 after the first block). Teero's U-turn-1
  structure: no kick on the approach after k1096, a pre-fire at rt ~1137 at the far right wall (extracted aim reads
  -177, 180 deg off) exploding at the apex k1158, point-blank k1165. Plan with that structure (apex.plan: shots only
  1126-1142 at the apex wall, then free): 1251/1248/1251 [chain best 1250]. Box gate with an energy objective (new
  seg `boxe=L`): best |v| 48 at x 7685 rt 1227; Teero ~56 there ~12 ticks earlier. The time goes between kicks: e.g.
  after the column kick (1087) the descent to the U-turn should add +330 v^2 but hook line-following eats it.
- Recurring loss: a kick that accelerates right before a bend, then hook braking (step 4: kick 1242 to |v| 62, then
  -3200 v^2 of braking into the dip at x~5800; Teero's kick there (k1256) points him down into the dip instead).
  ghostsink 3000 (energy devalued from further before sinks) on that window: 1407/1409 [1410/1409] (small).
- Chain g2: step 3 (k1174 -> k1324) 1331, step 4 (-> k1399) 1409, step 5 (-> k1474) 1487 (16 behind Teero).
  Winning setups: always latpen 0.2/24 (vcap 1.25 or 1.1); quota / brake=2 / Teero-plan variants never win in the
  corridors -> vset p11 (B1 x seeds, vcap 1.1 / 1.0+bp10, ghostsink 3000, one quota for the maze). Resumed after step 5.
- Map settings: only `sv_kog_map_quests Q_NO_HAMMER,Q_NO_PLATFORM,Q_TEAM_15` (no tune commands).
- Equal-state benchmark rerun on segf (tp at Teero's k1220 state, his reload phase, gate k1285, Teero 1285): B1
  1290/1291, vcap 1.1 1288, ghostsink 3000 1289, b7 (ghost=3 hnow=600 ghoste=0 brake=2) 1287, b7 + latpen 1287.
  seg kicks at 1241 to |v| 67 and hook-brakes to 33 into the dip; Teero kicks at the ceiling at k1256 (down into the
  dip) and is also at |v| ~34 there. Forcing his structure with a plan (no shot before 1249, point-blank 1249-1258):
  1286-1289 -> not the missing piece; the 2-5 ticks are spread over path/hook details through the valley.
- seg `loadres=F` (new: reserve F x beam for the best states holding a loaded grenade, against the horizon effect):
  equal-state 1291-1293 [1287-1290], step-4 window 1410/1409 [1410/1409] -> off.
- Own-run reference on the step-4 window (ref = 2693 track, gate own k1432 = Teero k1399): ghost=1 erel=1 latpen 0.2
  or 0.05: 1417/1417; ghost=3 vcap 1.5: 1409 [Teero ref 1409-1410] -> no gain.
- Chain g2 step 6 (k1399 -> k1549, left U-turn): 1569 (23 behind Teero). Projection at ~6% loss over the rest:
  finish ~2615-2630.
- fireangles 96/144: step-4 window 1409/1407 [1410], equal-state 1289/1292 [1290] (noise level).
- Chain g2 steps 7-9: k1624 1658 (-14 in one window), k1699 1741, k1774 1820 (49 behind Teero true). All 12
  variants within ~5 ticks of each other per window (systematic loss); ghostsink 3000 variants identical to base there.
- **Step-7 loss = Teero's double-kick launch up the shaft at x~3300 (k1583):** a grenade lobbed ~25 ticks earlier lands
  on the small 2x2 block just as he fires point-blank down at it -> two explosions under him, ~+22 px/t upward, he
  flies the 1000 px shaft at ~50; we fire one kick there (27.6 -> 37.1) and lose 12 ticks climbing. seg cannot find
  it: prefire variants 1656/1659 [1658] (they find a pre-fire + point-blank pair at the U-turn exit instead), Teero's
  schedule as a plan 1668, hand lob plans (fire 1576-1594 -> explode at the block 1603-1610): the lob is never fired
  (no aim reaches the block from our lower/later line; his extracted lob aim collides early in simulation) 1663-1682,
  planrv (rendezvous credit) 1661/1663, padtop 1500 1656. Double kicks need a planner that picks the lob and the
  approach line together.
- Chain g2 resumed after step 9 with pre-fire variants (rh.py p11 pf11=1).
- Teero's catalog: ~10 long pre-fires (18-23 t of flight, "rode beside him", "lob") + several short ones; ours 2-3.
- **rdv (new tool, build target `rdv`):** rendezvous opportunities along a run - from every tick, N aims flown
  exactly (CFastG), reported when the grenade explodes 'minflight..maxflight' ticks later next to where the run
  itself is (kick strength / d(v^2) with the run's velocity); `loaded=1` only ticks where the run holds a loaded
  grenade, and the run's next shot is printed. Chain run c9 (rt 1012-1760): ~10 clusters worth +500..+860 (about as
  many as Teero's pre-fires), but every one collides with one of our own nearby shots (taking it means dropping a
  kick, which changes the path and the rendezvous). Forced lob plans from it (planforce=1, with/without planrv):
  step-7 window 1668-1678 [1656-1658], step-8 window NOGATE. Unforced plans: the beam never fires the lob.
  padref 80/120 with pre-fire aims: 1656/1656/1657. => double kicks need the line and the shot schedule planned
  together (as Teero does); not reachable with seg + plans.
- Chain g2 step 10: k1849 at 1900 (54 behind Teero). Projected finish ~2630.
- **Chain g2 finished: kog_full_best.txt = 2677 race ticks (53.54 s), server-identical (TasReplay: start 68, finish
  2745, no freeze, no double start)** - F1010 exit (968-based prefix) + 20 windows (p8 step 1, p9 step 2, p10 steps
  3-5, p11 steps 6-9, p11 + pre-fire variants 10-20). Lead vs Teero by window end (true ticks): +4 k1174, -4 k1249,
  -10 k1324, -13 k1399, -16 k1474, -23 k1549, -37 k1624 (shaft double kick), -45 k1699, -49 k1774, -54 k1849,
  -60 k1924, -80 k1999 (S-bend), -83, -87 k2149, -97, -106, -111, -120, -127 k2524, finish 2677 vs Teero 2536.
  (old best kog_full_2693.txt). Post-grenade from the exit: 2677-1012 = 1665 ticks vs Teero's ~1503.
- **Incumbent LNS (lnsinc.py, runs/lnsinc2, 2 workers, cuts >= 1900) on 2677: 2669 (53.38 s)** at cut 2304 (Teero
  ref, beam 8000), server-identical (start 68, finish 2737, no freeze, no double start) -> kog_full_best.txt.
  Other iterations: 2677 (cut 2379 own, cut 2068 own), 2673 (cut 2035 own, on the 2677 incumbent), 2669 (cuts 2305, 2279).
- physlab6 (agent): Teero's shaft double kick (2x2 block at x~3251, k1583) from cuts >= 1450 of the best run.
- LNS lnsinc2: **2668 (53.36 s)** at cut 2060 (Teero ref, beam 8000, survive 20, kcredit 0), server-identical (start 68, finish 2736, no freeze, no double start) -> kog_full_best.txt.
- LNS lnsinc2: **2666 (53.32 s)** at cut 2598 (Teero ref, beam 20000), server-identical -> kog_full_best.txt.
- LNS lnsinc2: **2664 (53.28 s)** at cut 2454 (own ref, beam 15000), server-identical -> kog_full_best.txt.
- auto-accepted lnsinc2/best_2663.txt: **2663 (53.26 s)**, server-identical (start tick 68 finish tick 2731 -> 2663 ticks = 53.26 s; no freeze, no double start) -> kog_full_best.txt.
- **segf** (seg on CFastG via CTasFast, -DSEG_FAST) + CCollision::ms_FastPaths on (TAS_NOFAST disables): same
  results as seg, several times faster. New seg options: inc=FILE (incumbent kept in the beam), kfut/kfutn, firemax,
  erel, boxe, loadres, kickmin (drop states whose explosion kick < K, via CFastG::ms_pLog), tpline=N.
- Teero's grenade techniques (video): (a) catch-up pre-fire before a U-turn (fire forward, outrun the grenade,
  hook-swing round the inner wall tip, it explodes at the outer wall beside him after the turn); (b) 25-tick lob +
  point-blank double kick (shaft 2x2 block); (c) shot schedules planned backwards (each shot is a pre-fire so the
  reload frees in time for the next kick slot). Our beam picks shots greedily per tick.
- Kick-slot comparison vs Teero: our lag jumps exactly where we miss his slots #5, #9, #14/15/18, #20, #27, #31/33,
  #41, #44-45, #48.
- From Teero's own state at k1100 the free search loses 11-12 ticks by k1262 (tracking matches until the apex, then
  loses). With his structure as a plan (wall pre-fire + free): 1266 vs Teero 1262, reproducing his 5 kick spots. From
  our real state c1 -> k1249: 1248 vs chain 1250. Catalog-based slot plans (time- or position-indexed) are worse; plans
  built from exact rdv flights work.
- **rdv RDV_SCHED (planner):** per fire tick x aims, exact flight incl. same-step explosions (CFastG log), fresh hook
  press ticks skipped (shared aim); value = dv/max(|v|,10) * min(ticks to next speed minimum, 60); DP over shots with
  >= 25 spacing and the initial reload -> SCHED_OUT. schedplan.py turns it into a loose plan (forced windows +-3 for
  shots with >= 5 flight, free windows elsewhere). On a good path the planner picks Teero's structure; iterating
  plan -> search -> plan diverges (1276 -> 1291-1299), so it is used as an extra candidate per window (pchain.py).
- pchain p1 (planner-guided windows, beam 6000): k1249 1252 [g2 1250], k1324 1338 [g2 1331]; the plan variants lost
  every window -> stopped.
- Why the free search misses Teero's U-turn pairs: FireTargets only aims at solid tiles within 120 px (point-blank),
  so long pre-fires exist only with padaims; and a pre-fire is judged as not fired (prefire=1) while holding states get
  the loaded-grenade credit. On the c1 window (k1099 -> k1249): prefire+padaims 1254/1252, + loadres 1254/1254,
  firelook=30 look-judged 1252/1252, pfcred (new: credit from a held-input look with/without the grenade) 1254 /
  1253/1253 [free 1252/1252] -> none find the pair.
- **seg retro=K (segf, new): retro shots.** A grenade's flight is independent of the tee, so at every state the
  search looks back along the state's own history for ticks where it held a loaded grenade (no shot since, no hook
  start in the patched input) and solves the aim whose grenade first hits a solid point next to the tee in the next
  tick (exact flight check, no earlier collision); the state with that grenade in flight (reload 24-d, fire counters
  +2) is expanded too and the shot is patched into the ancestor's input at output. Exact (patched runs replay to the
  same gate tick). It finds Teero's shaft double kick by itself (lob fired 1582 + point-blank 1608, two explosions at
  the 2x2 block in the same tick, |v| 22 -> 43.5) when the line goes there (latpen 1/16).
  Window tests (beam 6000, 2 seeds): c1 window 1252/1252 [1252/1252]; k1324->1474 retro=1 1484/1491 [1488/1487];
  k1399->1549 retro=1 1571/1569 [1577/1576]; shaft retro=3 latpen 1 1662/1656 [1661/1659].
- Equal-state (tp at Teero's k1100 state, gate k1262, Teero 1262): free 1271/1273; retro=1/3 1270-1275; kickmin
  10/11 1272-1274; track=1 1275-1283. Our free path is within 10-35 px of Teero's until the U-turn exit (7 px at
  k1171) and then falls behind: one kick out of the turn (|v| 30) vs his pre-fire + point-blank pair (~38).
  retro=3 + loadres 0.4 (seed 2): 1268 with his structure (no ceiling shot, pre-fire 1142 -> apex 1161 dist 68,
  point-blank 1167, |v| 34.9); other seeds crash (1286-1291). k1220 equal state (Teero 1285): 1291-1292 for all.
- retroafter=1 (new, off by default): retro shots also for states that fired since (retro shot 25+ ticks before that
  shot, exploding after it). k1100 equal state: identical results (1270/1273, 1268) -> rarely matters.
- retro + loadres variance = beam collapse: e.g. seed 3 (1288) kicks to |v| 52 at 1224, keeps rising along y~2000
  while Teero drops into the dip, hits the ceiling at ~1248 (lag +8 -> +17) and the whole beam is in that lineage.
  quota=30 fixes it: retro=3 loadres=0.4 quota=30 1269/1270, crashw=0.002 crashn=12 1272/1270 [free 1271/1273,
  Teero 1262]. Remaining gap to Teero from his state: U-turn exit ~35 px/t vs his ~40 (our double: d54 cos 0.60 +
  d55 cos 0.70; his d57 + d14).
- Validation chain r1 (rchain.py, beam 6000, best of free1 / rt1 / rlr2 / rlr4): k1249 1252 [g2 best-of-12 at beam
  10000: 1250], k1324 1331 [1331, won by retro + loadres 0.2]. Resumed from window 3 with quota variants
  (free1, rt1, rlr2q, rlr4q).
- Nade lab sub-agent (tas-work/nadelab/): best possible grenade boosts in isolated scenarios, first: max height from
  one unhookable block.
- **Chain r1 finished (rchain.py, beam 6000, best of free / retro=1 / retro=3 + loadres 0.2|0.4 + quota 30): 2664
  (53.28 s), server-identical (start 68, finish 2732, no freeze, no double start) -> kog_full_r1_2664.txt**, vs the
  same-scheme chain g2 2677 (best of 12 at beam 10000). Per window vs g2: k1399 1405 [1409], k1474 1485 [1487],
  k1549 1562 [1569], k1624 1652 [1658], k1699 1734 [1741], k1774 1809 [1820], k1849 1895 [1900], k1924 1973 [1981],
  k1999 2059 [2076], k2074 2143 [2154], k2149 2227 [2233], k2224 2306 [2318], k2299 2390 [2402], k2374 2468 [2482],
  k2449 2551 [2566], k2524 2644 [2648], finish 2664 [2677]. Retro variants won 12 of 19 windows (often by 5-15 over
  free). Not yet below the LNS-polished 2663; re-running the last two windows with 10 variants (runs/rc/r1b).
- r1b (last two windows of r1 re-run with 10 variants: free x3, retro=1 x3, retro=3+loadres+quota x3, retro=3+quota):
  k2524 still 2644 (2644-2662); finish **2663 (53.26 s), server-identical -> kog_full_r1b_2663.txt** = ties the best
  (the LNS-polished g2 run) without any LNS. Wait loops of the form `until ! pgrep -f X` match their own command line.
- **nadelab (sub-agent), scenario 1 = max height from one unhookable block: 2414 px (75.4 tiles), server-identical**
  (nadelab/best_height.txt, FINDINGS.md). Technique: grenade stacks exploding on the block in the same tick as a
  ground jump + point-blank shot: a 2-stack (79-tick lob + point-blank, vy -36.7), then a 3-stack (a 101-tick
  lifetime grenade fired from 853 px up exploding in mid-air right under the launch spot + a 73-tick lob + point-blank,
  vy -48.7). Proven max without mid-air kicks toward the block (relaxed gate model); a kick-assisted 4-stack (3734 px)
  is not ruled out.
- **Prediction vs server bug found by nadelab:** a grenade hitting a tile on its last lifetime tick explodes twice in
  the client prediction world (CTasGame, CFastG) but once on the server. CFastG fixed (lifetime branch now `else if`;
  backup fastg.cpp.bak_dblexpl); CTasGame still has it. Accepted runs were all server-checked, so they are unaffected.
- Prediction world fixed too (game/client/prediction/entities/projectile.cpp: return after a destroying hit, like the
  server; backup .bak_dblexpl): nadelab demo now 2165 px on CTasGame (was 3185; server 2166 from the standing y),
  best_height 2413 unchanged, kog_full_best still replays to 2663. Patch included in changes.patch.
- LNS lnsr1 (retro variants, on the r1b 2663 run): **2661 (53.22 s)** at cut 2403 (retro=3 loadres=0.2 quota=30, beam 8000), server-identical (start 68, finish 2729, no freeze, no double start) -> kog_full_best.txt. (autoaccept's first check got an empty testrunner output - transient; re-checked by hand.)
- auto-accepted lnsr1/best_2653.txt: **2653 (53.06 s)**, server-identical (start tick 68 finish tick 2721 -> 2653 ticks = 53.06 s; no freeze, no double start) -> kog_full_best.txt.

## Post-nade vs Teero, tracker-seeded search (Oct 6)

Start: kog_full_2639 (52.78 s). Teero data (local only, gitignored): teero/teero_inputs_0-3131.csv (per video frame:
dir, jump, cursor aim, shots with aim, certainties; best effort, not exact) and teero_track.txt (position per label,
+-10 px). Label == csv race time (s_since_start * 50).

### Facts measured
- Our pickup (rt 971) is at Teero's label ~982.5: we start the post-nade ~11.5 ticks ahead. His finish is label ~2542,
  so his post-nade is ~108 ticks faster than ours (from our pickup it would finish ~2530).
- Lag profile (2639 run vs Teero, monotone projection): 971-1160 +5, U-turn 1160-1240 +10, 1240-1520 +7, shaft
  1520-1640 +18, channel 1640-1680 +6, 1680-1800 ~0, 1800-2240 +43, 2240-2600 +16.
- 1800-2240: same route as ours (overlay: viz.py with TREF=teero_track.txt:L0:L1), he is 5-10% faster per tick
  everywhere. Our kicks average |f| 10.9 of 12 (28 of 55 at full force); the solid walls sit behind a freeze tile, so
  a full kick needs the tee skimming the freeze.
- Velocity ramp: factor from |v| (total), applied to the horizontal movement only (vertical never ramped). v^2 - y
  is the right stored energy but over-credits speed as progress (x movement saturates at ~48 px/t, |v| ~119).
- Shaft (rt ~1600): Teero does a double kick on the single tile at (3264, 3328): a lob fired ~25 ticks earlier lands
  in the same step as his point-blank (vy -47 after). From our path lobs reach that tile only at rt 1601-1602
  (x_lob); x_dbl builds our version (vy -40) but nothing downstream turned it into time.
- Hook pull: applied only if |v| stays < 15 or does not grow; the downward part is scaled by 0.3, so hooking
  below-right while climbing builds vx cheaply (Teero's shaft climb).

### Tools (src/tas)
- x_lob: exact lob scan from a run's own positions into a box/tick range.
- x_dbl: double-kick rendezvous (approach search so a point-blank lands in the same step as a lob in flight).
- x_win: windowed beam with a linear or geodesic objective (probe what a section allows).
- x_tig: tracker of a reference run (Teero's video, or one of our runs via run2ref.py). Position-indexed hints
  (each state looks up the reference's jump/shot ticks and aims at its own place on the reference line), lateral
  cost with dead zone, signed lag reward (lagw), reserved shot slots resolved as retro lobs (also stacked with a
  point-blank), survival check (simple hook/dir/jump/point-blank policies, with an all-doomed fallback), diversity
  quota, seeded jitter (results vary a lot with tie-breaking: run several seeds). Speed/energy credits (sw, ew) did
  not help on 1800-2300.
- run2ref.py: one of our runs as an x_tig reference (track + inputs csv, all certain).
- tigloop.py: the pipeline below, looped over start points with server-checked promotion.

### Pipeline that produced 2633 (52.66 s, server-checked)
1. x_tig follows Teero from the 2639 run's state at rt 1800 (off = Teero label - rt = -35.9): 8.2 ticks ahead of
   the 2639 run by rt 2050 (it then falls behind after ~2150 and does not finish well on its own).
2. x_tig from that rt-2050 state following the 2639 run itself (run2ref.py reference, off 8.2): full runs 2647-2660.
3. dschain from rt 2050 with prefix0 = the 2647 run's prefix, anc0 = the 2647 run (forced lineage), inc = 2639:
   2633. Without a forced lineage, chains from any novel prefix lost 25-34 ticks in the first window - that was the
   blocker for every earlier rejoin attempt (double kick, channel, tracker cut points).

### Tracker loop results (Oct 6-7) and what limits it
- Best so far **2614 (52.28 s)**, server-checked: 2639 -> 2633 -> 2630 -> 2629 -> 2627 -> 2622 -> 2616 -> 2615 -> 2614,
  every gain from tigloop starts in rt 1700-2050 (velocity-matched Teero tracker, vw 1-2, judged cuts, follower +
  chain with the follower as forced lineage).
- Follower fixes that mattered: (1) a judge follower that dies before its horizon fails the cut (it used to measure
  the lead at the death point); (2) children made by the shadow run's own inputs are kept past the cell quota and the
  survival check (shkeep), but the survival exemption only applies within 3 px / 1.5 px/t of the shadow's own state
  (far from it those inputs steer into freeze). From the incumbent's own state at rt 2140 the follower now loses 2.4
  ticks by rt 2300 (was 8.6).
- Dead ends: a 10000-wide tracker beam (worse than 3000: the ranking, not the width, limits it); polishing chains
  (dsloop) on the 2614 run: all ties or worse; the Teero tracker after rt 2200 finds no ahead cuts.
- Lag vs Teero of the 2614 run: rt 1600-1650 +16.4 (his double kick + climb), 1650-2000 only +5, 2000-2050 +6,
  2400-2600 +14.
- Shaft: tracker cuts ahead by up to +5 at rt 1650 have velocities ~10 px/t off ours and collapse when following our
  line. hybridref.py (Teero's reference up to label 1667, then the 2614 run relabelled -33) gave a tracker run 9.4
  ticks ahead at rt 1640 and 3.7 at 1700, but it loses ~14 ticks over 1700-1850 (finish 2640); chains with it as
  forced lineage ended 7-10 behind. The states it reaches after the top turn / channel are not Teero's (it lags him
  by ~8 more between rt 1640 and 1700, where his ceiling kick and channel kick are).

### Oct 7: why new branches lose their lead (shaft double kick, polish chains)
- Chain dk2 (shaft double kick, prefix L0_c1700 + F1700_0 lineage): ties 2614. Lead over 2614: +12.4 at rt 1640,
  +8.4 at 1700, +2.6 at 1800, 0 at 1840; from ~1960 on it is the 2614 run itself (merged).
- Same-x comparison through the channel (x 4300-8800): Teero's and 2614's displacement speeds match within ~1 px/t
  (2614's channel came from Teero tracking); dk2 is 2-6 px/t slower everywhere. Energy at x=4900: 2614 -1080, dk2
  -1750 (2614 kicks at 1679 on the descent, dk2 spent that slot at 1656 and loses ~360 to hook steering there).
- Every attempt to keep the shaft lead loses it in the channel: Teero tracker from dk2@1700 (4 seeds) +1-3 at 1800;
  incumbent follower from dk2@1645 -26/-36 at 1870; tigloop D1 on the dk2 run (starts 1630-1710) no cuts; x_ds
  windows from dk2 cut 1630/1645/1658 with dk2 as lineage +3.2..+4.3 at 1790.
- Polish chains P2_006..P2_008 (x_ds windows from cuts 1155-1507 of 2614) reach +5..+9.7 at rt 1647, then the
  window through the channel (gate 1825) loses 27-29 ticks (and P2_006/P2_008 got no gate at all).
- x_ds free search (no incumbent lineage, no shadow) is far below the polished incumbent everywhere: channel 1640 ->
  1790: -5.8 (beam 3000), -9.9 (beam 10000), -8.8 with energy weights x5-x10; final 2440 -> finish 2618, 2300 ->
  finish 2632. (Exception: U-turn 1, cut 1095 -> gate 1262: beam 10000 free 1258.5 vs 1262 with +1100 energy
  (one more kick: 1146, 1177, 1196, 1215, 1242); beam 3000 1261-1262.) The channel loss is kick-spot timing: the
  free search kicks off the valley floor at 1707 (|f| 7) instead of the small block at x~5450 (1712, |f| 12).
- **The polished run is pixel-brittle** (x_pert, new: replay to tick K, perturb the state, continue with the run's
  own inputs): +-1/256 px/t is absorbed by quantization (positions are integers, velocities 1/256 after every tick),
  but +-4..16 quanta or +-1 px at rt 1640/1700/1900/2200 usually ends frozen. An approximate rejoin onto the run
  cannot reuse its inputs; a branch needs its own pixel-level polish downstream.
- Energy accounting of 2614 (eacct.py, rt 1030-2614): kicks +34,100, hook -32,000 (964 hooked ticks), jumps -1,200,
  braking dir -2,000, dir0 -1,300, walls -600. About 19,000 of the hook loss is braking right before turns
  (1125-1150, 1250, 1425-1450, 1800-1825, 1950-2000, 2175, 2300, 2450), mostly 20-30 ticks after a kick that
  accelerated into the turn. Example, the hop at rt 2300 (peak at x~3800): kick 2299 to |v| 47, hook-braked to 13.5
  at 2325; Teero crosses it at ~25 without braking and is 2-4 px/t faster in the following corridor (x 4300-7600).
  An x_ds window that forbids the 2299 kick (nokick=2286,2318) found nothing better than the incumbent.
- Lag of 2614 vs Teero by section: 971-1150 +3.5, U-turn 1 (1150-1250) +11.8, 1250-1500 +4.3, 1500-1600 +7.9,
  shaft 1600-1650 +16.4, channel 1650-1825 +2.6, 1825-2000 +3.4, S-bend 2000-2050 +5.9, 2050-2300 +9.5,
  2300-2450 +7.6, final 2450-2614 +11.4 (83 in total; we start 12.3 ahead).
- Wide-beam polish chain W10a (dschain from 1825, beam 10000, 4 variants): +0.11 at 2135, +1.67 at 2202, 0.00 at 2320,
  +1.18 at 2505, +0.45 at 2574, finish 2614 (tie) - the same profile as the beam-1000 chains P1/P2: leads gained before
  the hop (2202) and before the final maze (2505) vanish in the next window.
- dk2's double kick vs an ideal one: explosions at 1601 (lob, |f| 12, cos 0.59 with the motion, pushes up-left) and
  1602 (point-blank, |f| 11.85, cos 0.91): vy -21 -> -43. Both kicks straight up would give about -44.5, so the launch
  itself is only ~1.5 px/t short of the best possible here; Teero's -47 is within his track's noise.

## Branch claude/fervent-cray-0pser4 (Oct 6-7, in parallel with the section above)
- autoaccept.sh: DIR is now made absolute (a relative DIR broke the testrunner input path after its cd -> the empty
  "start tick" rejections of 2661/2654); empty testrunner output is retried.
- Shaft window from the best run (cut 1550 -> k1699; own run 1731): free 1735, retro + loadres + latpen 1/16
  1733-1739; forced hold before the block (plan free windows around 1576-1599) 1738-1742 - one of them gets the
  double kick at the block (two explosions at 1601, |v| 22 -> 44) but still 1738. The shaft loss (~15 ticks vs Teero)
  is not just the missing double.
- **seg bug (incumbent lost in LNS):** a state with a grenade in flight is judged by a held-input look; if that look
  froze the child was dropped - the incumbent's own continuation too (runs with long pre-fires / retro shots hit it a
  lot), so lnsinc iterations returned worse than their incumbent (2669/2676/2701/2662/2682). Fixed: the incumbent's
  child is never dropped by the look (evaluated without it). Beam-1 incumbent replays now reach 2653/2663 from cuts
  1100/1984 (before: NOGATE). New tool fgcheck (CTasGame vs CTasFast tick by tick from a cut, freeze flags): no
  mismatch on kog_full_best / kog_full_2663.
- auto-accepted lnsr1/best_2652.txt: **2652 (53.04 s)**, server-identical (start tick 68 finish tick 2720 -> 2652 ticks = 53.04 s; no freeze, no double start) -> kog_full_best.txt.
- Shaft (best run cut 1550): Teero reaches the 2x2 block already rising at ~25 px/t (hook swing through the V bottom,
  no kick) and the double kick sends him up at ~53; we reach it slower and get one kick (~36). Box gate at the shaft
  top (3400-3700 x 2300-2520): search 1625 vs own 1627 (Teero ~1604, mostly lost before 1550). Box past the top turn
  (3950-4150 x 2350-2750): search 1652-1653 vs own 1657, but at |v| 24 vs our 30-32 -> no real gain.
- LNS lnsr1: 2653 -> **2652 (53.04 s)** at cut 2088 (retro=3 loadres=0.2 quota=30, beam 15000), auto-accepted.
- auto-accepted lnsr1/best_2650.txt: **2650 (53.00 s)**, server-identical (start tick 68 finish tick 2718 -> 2650 ticks = 53.00 s; no freeze, no double start) -> kog_full_best.txt.
- **2639 (52.78 s), uploaded by the user** (kog_full_2639.txt): server-identical (start 68, finish 2707, no freeze, no double start); same inputs as our 2650 up to race tick 1088, different post-grenade after. Now kog_full_best.txt and the LNS incumbent (runs/lnsr1/best.txt).
- **2630 (52.60 s), uploaded by the user** (kog_full_2630.txt): server-identical (start 68, finish 2698, no freeze, no double start); = 2639 up to race tick 1801. Now kog_full_best.txt and the LNS incumbent. Also uploaded: kog_pregren_963 (other agent's pre-grenade, pickup at race tick 963) to merge with it.
- Merge 963 + 2630: the 963 pre-grenade is +5 ticks ahead of 2630's (968-based) pre-grenade over rt 820-945 at the
  same positions (<= ~20 px), then dives at ~37 px/t and freezes 2 ticks after its end (no grenade kick can save it).
  Direct splices "963 up to rt c + 2630 from c+5" (c = 700..945) all freeze (checked with fgcheck). Sub-agent
  (merge963/) re-fits the transition aiming at an exact state match with 2630 (DDNet quantizes the core state), so
  2630's inputs can be appended unchanged (target ~2625).
- **Strategy change (user): no single operation > 1 min until +50 ticks; creativity over compute.** LNS stopped.
- New tools: `eaudit MAP INPUTS FROM_RT [TO_RT]` (per-tick energy audit: hook / explosion / tick / move deltas of |v|,
  EA_TICKS=1 for per-tick lines), `perturb MAP INPUTS CUT_RT [n dpos dvel seed]` (basin test: perturb the state at a
  cut and replay the rest), `rejoin MAP RUN A B DELTA` (beam toward RUN's state at B+DELTA), seg options `rjrun=FILE
  rjd=D` (rank by distance to FILE's state D ticks ahead, write out+"rj.txt" on an exact rejoin), `follow=FILE fh fw
  ffin` (follower lookahead toward a reference run), `hookmaxv=V`, `hookhold=N hookholdv=V`.
- Energy audit of 2630 post-nade: braking dominates the losses; long hook holds are ~2/3 of the hook loss; taps cost
  ~4 px/t each. Teero takes the same path with the same vertical share but is 1-4 px/t faster horizontally, with the
  same air-brake frequency.
- **The polished run is a knife-edge:** perturbing the state by +-1 px kills 73% of continuations within 30-100 ticks;
  +-0.02 vel is absorbed. Rejoin works only for delta=0. rjrun rjd=1 scan of 2630 at A=1100..2525 step 75: no exact
  rejoin anywhere. Follower lookahead: followers die, no gain. Window searches with brake=4.35 / vcap / hookmaxv /
  hookhold knobs stay within +-5 ticks of 2630's own window times (owngate.py).
- **Teero comparison, post-nade (2630):** the same path, and about the same number of explosions (ours 56: mean kick 11.0,
  78% of the ideal aligned energy). Teero is faster on the straights (ramp-inverted |v| 10-17 px/t higher at the peaks).
  Lag growth per section (vcmp/wp.py): the shaft climb + top turn (block k1584 -> k1624) costs 17 ticks; 2110-2200
  costs ~12; the right U-turn exit 1150-1250 ~10; the rest ~1 tick per 10-20.
- **Shaft (rt 1564-1730):** Teero double-kicks off tile (102,104), from a lob fired ~25 t earlier plus a point-blank,
  to vy -45. During the climb he drifts right (vx 7 -> 16), exits the shaft diagonally where it opens (row 83), and
  turns at the freeze ceiling with a point-blank (#21). We single-kicked to -34.5, killed vx in the shaft, and crept
  across the top at 16 px/t.
  - **seg's survival test was the blocker:** from Teero's own state (tp at k1587), seg reaches k1699 at rt 1737
    (Teero 1696): it air-jumps at once (sets vy = -12), because every fast climb fails the held-input 30-tick
    rollouts. With survevery=0: 1705.
  - tp tests at our block (survevery=0): vy -46.5 (double) -> k1699 at 1721-1722 vs single 1734 (own 1731).
  - New tool **lobscan** (`lobscan MAP RUN STRIP_RT T0 T1 EX EY R TE0 TE1 [N]`): exact lob aims from the run's own
    states (shots stripped from STRIP_RT) that explode within R px of (EX,EY) at race ticks TE0..TE1. From 2630:
    lobs fired at rt 1575-1579 land on the tile top (3270,3328) at 1601-1604.
  - **Forced lob in the prefix (runs/shaft/mklob.py) + segf survevery=0:** lob at 1576 (aim 9837,-1797) -> double
    kick 24 px/t at 1602 (|v| 23.6 -> 44.6), Teero-like climb, air-jump brake, ceiling point-blank at 1631
    -> **k1699 at rt 1724 vs own 1731/1732** (L1576_0_0.txt). Lob 1577: 1726; lob 1578: 1729. With survevery=2:
    1730-1733 or NOGATE.
- Window comparison, survival on (sv2) vs off (sv0), from 2630 cuts: k1249->1399 1405 / NOGATE (own 1407);
  k1399->1549 1574 / 1571 (own 1564); shaft 1735 / 1733 (own 1732); k1699->1849 1898 / 1894 (own 1890);
  k1849->1999 2063 / NOGATE (own 2051). New seg option `survrisk=F`: up to F x beam doomed states still breed (capped
  in the selection). 0.3: shaft 1733, k1249 window 1420 (worse). Not a general fix.
- **Carrying a local gain into the polished run (the knife edge):** a fresh continuation from the double-kick state
  loses the lead within ~1-2 windows (chain sh1/sh2: +8 at k1699, +5 at k1817, ~0 at k1892; with sv0 commits a
  later window can fail completely). The other agent's branch (compassionate-davinci, read only) hit the same wall
  and solved it with a tracker plus a forced lineage (tigloop: x_tig follows the incumbent with its inputs as hints,
  then dschain with the tracker run as forced lineage; 2639 -> 2633 -> 2630 -> 2629). Their x_tig / x_trace / x_ds /
  x_dbl / x_lob build unchanged in our tree (sources copied, CMake targets added). By x_ds root progress, our
  double-kick line (L1576) leads 2629 by 10.3 ticks at rt 1690. Following 2629 with x_tig (beam 800-1000, in
  < 60 s segments, tigseg.py) keeps 4.5 of it by rt 1840, then loses it: +1.7 at 1890, -8 at 2090, -30 at 2290.
  Their pipeline uses beam 3000 and several seeds per stage; it doesn't fit the 1-minute budget.
- **Energy along the path (ecomp.py; E = v^2/2 - 0.5 y, Teero's from his track with the ramp undone):** at every
  turn our energy equals Teero's (within ~100). On every straight Teero gains 300-650 more and then brakes later and
  harder to the same turn energy (e.g. right U-turn: Teero 727 -> -1054, us 135 -> -1122).
- Kick surfaces (eaudit EA_SURF=1, from the explosion log): of our 56 post-nade kicks, 37 are off bare gray blocks
  (28 at full force, mean cos with v 0.80, push along v 9.1 px/t) and 19 off freeze-lined surfaces (3 full, cos
  0.59, 5.9 along v). A full kick off a freeze-lined surface is >= 42 deg off the surface's tangent (the tee center
  can't come closer than the 32 px freeze tile), so it pushes <= ~9 px/t along a corridor. Teero also uses bare
  blocks for ~2/3 of his kicks; our bare kicks are mostly well aligned (0.88-0.92). The weak ones are turning kicks.
- Hook audit (eaudit EA_HOOK=1): 197 of 582 fast hook ticks turn almost for free (876 deg for 17 px/t of speed);
  385 brake (273 px/t for 1290 deg). Above 15 px/t a hook pull is applied only if |v| doesn't grow, so a rope just
  behind perpendicular turns at up to |h|/|v| rad/tick at near-zero cost; a backward rope just brakes.
- **Shaft lead transfer with x_tig at beam 3000** (tigseg.py seg=80 back=20, ~35 s per segment, runs/tx/g3): lead vs
  2629 +10.3 (rt 1690) -> +6.8 (1750) -> +6.7 (1810) -> +6.1 (1870) -> +7.7 (1930) -> +6.7 (1990), then **lost in the
  left U-turn at rt 1995-2015** (5 -> -0.7). Other x_tig seeds and segx variants (survx / retro + loadres) from the
  rt-1990 state also end at -3..+0.7 after that U-turn. 2629's U-turn there is very precise, and an early-arriving
  state probably has a different reload phase for its kicks. Hand-off for a bigger budget: runs/shaft/L1576_0_0.txt
  (lob at 1576 + point-blank double kick) and runs/shaft/shaft_lead_rt1990.txt (lead +6.7 by x_ds root progress).
- Box gate at the end of the right straight (from 2629 cut rt 1740, box x 8400-8520): segx survx reaches x 8418 at
  rt 1797 with E = v^2/2 - 0.5y = 484 (2629: about rt 1798.5 there with E ~140). Better states exist on the
  straights, but the beam ranking doesn't keep them. New seg option `eres=F [ewin=15]`: F x beam places for the
  highest effective-energy states within ewin reference ticks of the front.
- eres tests (windows from 2629 cuts): k1249 window 1409 / eres 0.2: 1411 / eres 0.3 ewin 30: 1410 (own 1407);
  k1699 window 1895 / 1898 / 1912 (own 1890). No help.
- Per-shot re-aim check (lobscan self mode with DX=DY=0 = along the tee's own velocity; same-tick point-blank kicks
  are not seen by the scan): several 2629 shots have a much stronger delayed kick on the unchanged path (1158 7.5 ->
  10.2, 1236 6.7 -> 10.8, 1348 5.8 -> 12.0, 1574 6.4 -> 10.3, 1807 8.5 -> 11.4, 2261 4.6 -> 11.4). Applied
  (kickfix.py RUN T0 T1 GATEK): the 1348 re-aim (full kick at 1356) -> k1449 at 1491-1496 vs own 1458. The weak
  kick was deliberate: it sets up the left U-turn. Kick strength along v is the wrong objective near turns.
- **Seed spread dominates window results:** the right U-turn window (2629 cut rt 1740 -> box past the turn, own 1850)
  gives 1845-1882 over 6 seeds (survival on) and 1848-1872 over 5 (survevery=0). Best-of-many beats the polished
  run locally (1845).
- **Best-of-10 chain from the shaft line** (rchain.py variants ss*/sz*, then bo10b with a mixed set qs*/sz*/sxs*/
  rlrxs*/ss1; window 3 redone, where quota=30 seed 56 gave 2008 vs own 2015). Lead vs 2629 per gate: k1817 +7,
  k1892 +9 (sz = survevery=0 seeds 20 ticks better than survival-on seeds there), k1967 +7, k2042 +1, k2117 +3,
  k2192 0, k2267 -6, k2342 -3, k2417 +1, k2492 -4. **Finish 2633** (kog_full_2633_shaft.txt). Finish searches with
  inc from its commits at rt 2493 / 2414 (10 + 6 variants): best **2631** (retro=1 seed 79 / survx seed 84;
  kog_full_2631_shaft.txt). 2629 not beaten: the shaft gain is lost at the left U-turn (k1967-2042) and the
  bottom-left turn (k2117-2267), where 2629 is very tight.
- **Why the lead dies at the hard turns: shot timing.** 2631 (+7 ahead of 2629) follows 2629's path, but its shots are
  out of step with 2629's kick spots. From the same spot as 2629's rt-1868 shot (2631: rt 1861) it waited 50 ticks
  and missed 2629's kicks at (7684,3206), (6732,3188) and the bare block (5272,3726) (2629: full aligned kick at
  1965, then a perpendicular turning kick at the corner, 1995). 2629's gaps (28-39 ticks) leave enough reload
  slack to fire at all its kick spots 7 ticks earlier.
- **Shot plan from the incumbent (mkplan.py RUN FROM SHIFT TOL > plan):** 2629's shots as seg plan entries (fire
  window shifted -SHIFT +-TOL, explosion target = 2629's explosion point). planfree=1 prefire=1 padtop=600 from
  2631 at rt 1850 -> k2042 at **2090** (2629 2097, 2631 2095; planforce=1: 2104). Lead along it: +6.9 -> +2.6 in
  the U-turn -> +6.5 at rt 2090. As rchain windows (pl7) it fails (gates/commits inside the U-turn: 2026 at k1968,
  2106 at k2043). From that run at rt 2080 (+6.1) to k2192 (2629 2259): plan variants 2271-2282, survx 2263,
  survevery=0 **2262**, retro + survx 2263. The lead holds at +4..+5 to rt 2230 and then dies at the bottom-left corner
  (2629: a 23-tick lob straight down landing at 2221, then a point-blank at 2233; ours: one strong kick at 2214 and a
  weak freeze-lined one at 2242; no lob from the free reload slot reaches the corner on our path).
- seg plan entries by position (new: `P fx fy D ex ey r`, fire while within D px of 2629's fire position; planlockpx
  holds ordinary shots while approaching; mkplan.py with PLANPOS=D): U-turn window from 2631 rt 1850 -> k2042:
  D=48 NOGATE, D=96 2099 / 2107 (time-shifted plan: 2090). Bottom-left window (rt 2080 -> k2192) with the time plan
  shifted 5 / 6 (+-3): 2263-2273 (best without a plan 2262; 2629 2259). Not solved.
- More attempts at the bottom-left corner from the rt-2080 state (+6): 26 variants in all give 2259-2271 (2629:
  2259). The cause is shot timing again: our shots are 5-6 ticks ahead of 2629's at the same spots until the
  top-left turn (2629 shot 2149 at (1370,2777); ours 2159 at another spot), then only 2-5 ahead at the corner.
  Teero-tracking (seg track=W from 2629 rt 1740 -> k1849): 1895-1906 (own 1890). Finer shot aims (fireangles 64/96):
  1893-1898 (32: 1895). U-turn 1 pre-fire construction (kickfix KDIR=-1,0): 1266-1267 at k1249 (own 1253).
- **Other agent: new best 2622 (52.44 s)** (compassionate-davinci, tigloop start 1700): = 2629 up to rt ~1700, then
  +6 on the right straight (k1730-1880, where the box test showed +350 energy available), kept to the finish. It
  has the single kick at the shaft (1600). Our double-kick line (+10 at rt 1690) is complementary: hand-off
  candidate for their pipeline (start ~1690, prefix runs/shaft/L1576_0_0.txt cut at 1690).
- gchain.py (gate-list chain: gates after the turns, 10 variants per window, half of them with a plan from the
  incumbent's shots shifted -8): shaft line from rt 1690 vs 2622 -> k1849 1885 (= 2622), k1999 2051 (2622 2044, -7;
  the left U-turn again). Stopped. 2622's better straight (k1730-1880) uses up the shaft gain by k1849.
- **Collision / hook-loss penalties as search variants.** 2622's straight gain over 2629 comes from no block-landing
  collision (coll -28 vs -215 over rt 1750-1799) and fewer hook taps (hook -521 vs -662 over rt 1700-1799); its
  kick spots are the same. seg `crashw=W` (existing: penalty for v^2 about to be lost against walls) and the new
  `hookw=W` (segf: penalty per unit of v^2/2 the hook has taken along the path; a counterfactual core tick
  without the hook, via SStepAudit). Windows from 2629 cuts:
  right straight k1699->1849 (own 1890): plain 1893-1898, crashw 0.005-0.02 **1886-1888**, hookw 0.002 1896 /
  0.005 1918, both 1888; left U-turn k1849->1999 (own 2051): plain 2071, crashw 0.02 **2055** (0.04 NOGATE),
  hookw 0.002 2062 / 0.005 **2055**; k1249 window (1407): plain 1409, crashw 1420; k1399 window (1564): plain 1569,
  crashw 1573-1576. Strong in some sections, harmful in others: variants, not defaults.
- gchain cw22 (shaft line, 8 variants incl. crashw/hookw, vs 2622): k1849 1884 (+1), k1999 2048 (-4), k2192 2255
  (-3), k2342 2417 (-10). Stopped.
- **2621 (52.42 s), server-checked** (testrunner TasReplay: start tick 68, finish tick 2689 -> 2621 ticks; srvcheck:
  frozen ticks 0, double start False, died False): 2622 (other agent) with a new ending from a finish search on its
  rt-2500 cut (segx survx=300 jitter=1 seed=406, inc=2622) -> kog_full_2621.txt = kog_full_best.txt.
- lateloop l1: **2620 (52.40 s), server-checked** (start 68, no freeze, no double start): cut 2530 of the previous best, segx [survx=300] seed 565 -> kog_full_2620.txt = kog_full_best.txt.
- lateloop.py (late-cut finish searches on the best run, auto server-check): 2622 -> 2621 -> **2620** (both from survx
  seeds at cuts 2500 / 2530). Then 3 more rounds x 8 cuts (2580..2365) x 7 survx-heavy variants: no further gain.
  The ending is saturated for this search; current best kog_full_2620.txt = kog_full_best.txt (52.40 s).
- lateloop l4 (earlier cuts 2330..2150, beam 3500 so each search fits in 60 s; 7 variants): no gain on 2620. From
  cut 2180 and earlier some variants exceed 60 s. Finish searches from any cut 2150-2580 now reproduce 2620: the
  second half of the run is saturated for this search.
- Large-beam finish searches on 2620 (cuts 2600..2540; beam 20000 survx / beam 40000 / beam 20000 retro=3 loadres):
  all 2620 (beam 40000 from 2555/2540: > 60 s). The ending is optimal for seg's action set.
- lateloop m1: **2613 (52.26 s), server-checked** (start 68, no freeze, no double start): cut 2470 of the previous best, segx [survx=300] seed 1334 -> kog_full_2613.txt = kog_full_best.txt.
- Other agent: 2615 -> **2614** (tigloop L7 start 1900). lateloop m1 on 2614 (cuts 2590..2350, 8 survx-heavy
  variants, 2 rounds): **2613** (cut 2470, survx seed 1334, server-checked); nothing more in round 1. Pattern: a
  late-cut survx finish search squeezes 1-2 ticks from each new tigloop best.

## Merge session (Oct 7): the four branches benchmarked and merged; 962 grafted -> 2608
- Branches merged into this folder: davinci (base), cray (seg options + tools, 2613), cannon (collision fast paths,
  CopyFrom), faraday (`aip-tas-fast`: CFast, ddsearch, splice drivers, 962). Details: ../README.md, ../BENCHMARKS.md.
- Simulator: both branches' exact collision fast paths merged into one patch (cannon's prefix-sum broad phase,
  TileExists cache, ms_TasSolo, CopyFrom + faraday's inlined solid table in TestBox / IsOnGround / hook ray). copy+step:
  CTasGame 0.74-0.81 us (best branch 0.93-1.09), CFastG 0.60-0.65 (best branch 0.63-0.66), CFast unchanged ~0.55.
  `simbench ... fuzz`: 0 mismatches vs plain DDNet code for every stepper (~187k random-input ticks per tree).
- Same search on different simulators (S1): identical results, wall time 88 s (cannon seg) -> 62 s (merged segf);
  without fast paths 170 s. Profile of segf: survival rollouts ~50%, action generation ~40%, stepping ~30% (overlap).
- Same-budget LNS (S3, 10 min x 4 workers, server-checked gains): pre-grenade ddsearch 978 -> 976, segf / seg none;
  post-grenade x_ds 2677 -> 2667, segf lnsinc variants none (4 jobs: their big-beam finish searches take ~10 min each
  from mid-run cuts).
- **2608 (52.16 s), server-checked** (TasReplay + TasServer: start 68, finish 2676, pickup 966, no freeze, no double
  start): faraday's 962 dive grafted onto 2613 with the new `x_graft` (D=5, cut 930, beam 20000, 23 s). The 962 line
  is the 968 line ~5.6-5.9 ticks earlier (lead measured along the line: +5.0 at rt 850, +5.5 at 930, +5.6 at 940,
  +5.9 at 950), so D=6 is out of reach in the air. The graft grazes the block corner at rt 956 (jump refill), brakes
  with the hook on the block face and the double jump at the pickup, and becomes identical to 2613 (+5) at rt 969 when
  the air-control clamp sets vx to -5.00. `segf rjrun=` (exact rejoin) did not find it: its distance ranking never got
  below ~28 (D 2-6, cuts 930-950).
- **2607 (52.14 s), server-checked** (TasReplay + TasServer + trace: start 68, finish 2675, pickup 966, no freeze, no
  double start): 20 min of x_ds incumbent LNS (bench/lnsbench.py xds-dav, 4 workers, dschain variants, cuts >= 1000)
  on the first 2608 graft (kog_full_2608_b.txt): one gain at cut 1182, beam 1000 retro=3 shadow=2; differs from
  2608 from rt 2432 on. kog_full_best.txt.
- x_graft sorts with a total order now (distance, parent, hash, input), so a run is reproducible; the README command
  regenerates kog_full_2608.txt exactly.
- Pitfall again: `pkill -f PATTERN` / `pgrep -f PATTERN` match the calling shell's own command line.

## Techniques session (Oct 7): Teero's shaft double kick in a full run -> 2606
- Lag of 2607 vs Teero (teero_track.txt, lag = rt - label) by section: pickup -> 1150 +4, right U-turn 1 (1150-1275)
  +13, 1275-1450 +3, left U-turn (1450-1575) +8, shaft + top turn (1575-1675) +18, channel (1675-1825) 0,
  1825-2050 +10, 2050-2450 +17, final maze (2450-finish) +15.
- Shot spots: from the channel on (rt 1702, 1744, 1769) ours equal Teero's #25-27 (Teero-tracked). In the shaft
  Teero lobs from (2707, 3181) and double-kicks at the block; we kicked once. davinci's dk2 uses exactly his spots
  (lob, block, top (3596, 2439), descent (4189, 2716), channel (5298, 3225)).
- Splice: 2607's post-grenade = 2614's shifted -5 (the 962 graft), so dk2 (= 2614 up to rt 1573) splices onto 2607 at
  a 5-tick offset: sdk2 finishes 2609 (dk2 2614 - 5), sdk3 = sdk2 + 2607's ending from rt 2400 ties 2607 (they are
  state-identical from rt ~1955 on).
- Lead of the double-kick line over 2607 (lead.py-style projection): +12.4 at rt 1635, +11 to 1665, +8.4 at 1695,
  +6 at 1725, +4.3 at 1755, +2.6 at 1795, +0.6 at 1835, 0 at 1975. Block -> top: Teero 29 ticks, line 30, 2607 37.
- eaudit rt 1640-1689: line expl +270 (one kick, cos 0.53), hook -352, |v| 37.1 at 1689; 2607 expl +629 (kick cos 0.83
  at 1674), hook -109, |v| 47.1. Channel spot (~5300, 3220): line |v| 30, 2607 33-34, Teero ~31-33 (track).
- Grafting the line back onto 2607 does not work: x_graft (no shots) best distance 68-175 from cuts 1615-1635, D 6-10;
  segf rjrun (shots, retro) stuck at ~33 from cuts 1600/1610, D 4-8. The states are too different (no wall /
  floor contact or clamp collapses them; the channel-end U-turn is hook-turned, vx passes 0 smoothly).
- x_ds incumbent LNS (bench/lnsbench.py xds-dav, cuts 1590-1950, 4 workers, 20 min) on sdk3: 49 jobs tie, one gain:
  **2606 (52.12 s)**, cut 1777 (beam 1000, kickmin 11, retro 4, sinkh 40, shadow 2), differs from sdk3 from rt 1798.
  Server-checked (TasReplay + TasServer). kog_full_best.txt.
- Energy audit of 2607 (rt 966-2607): explosions +18,851, hook -9,729, dir -9,280, collisions -1,315. The big braking
  sections are turns we reach too fast: 1416-1465 (left U-turn) -2,790, 1766-1815 -1,759, 2416-2465 -1,696,
  1966-2015 -1,691, 2266-2315 -1,462, 1216-1265 -1,239, 1116-1165 -1,196.
- Correction to the lag table: Teero's track end is ~3-4 ticks inconsistent with his official finish (physlab4), so
  ~5 of the final-maze "15" is an artifact; the real final-maze loss is ~9-10 ticks spread over rt 2450-2600.
- Descent after the double kick is a local optimum for x_ds: 16 windows from 2606 cut 1627 (shaft top) to 2606's
  progress at rt 1700 (incforce=1, beam 5000, egain 0.004/0.01/0.02, kickmin 11, 4 seeds) all reach the gate at
  1699.98-1700.03 with the same energy. x_ds incumbent LNS on 2606 (cuts 1590-2050, 54 jobs, 30 min): no gain.
  Teero has both the double kick and a fast channel entry (~2607's); the difference is a joint change of the top
  turn / descent / reload phase that neither local windows nor LNS reach.
- Free searches from cuts far before a gate are much weaker than the polished runs and cannot judge a structural
  change: x_ds from 2606 cut 1627 to the channel gate (rt 1800) 1814-1838 with or without nokick=1640,1662; from
  cut 2400 to the finish 2640/2646 (2606: 2606). Short windows are competitive: cut 2435 -> 2606's progress at rt
  2497 (the final-maze up-turn): 2498.1-2498.6 free, the same with nokick=2436,2455 / 2436,2462 (dropping the kick
  before the up-turn changes nothing), 2495.9-2496.1 with incforce=1.
- x_tig (Teero tracker, tigloop's settings, off -51.5) from 2606 at rt 2380: drops to |v| ~13 within 10 ticks and
  ends ~100 ticks behind (4 seeds). As davinci noted, the tracker does not help after rt 2200.
- Pre-grenade, Teero's new run (fast/runs/teero_new_xy_60fps.csv, frame f -> rt = (f - 194.51) * 50/60, px = tile *
  32): leads 962 by +0.9 at rt 100, +2.6 at 225-250, +4.5 at 325 (right U-turn), +3.8..+5 at 350-450, then 962 is
  faster (+0.7 at 575, -4.6 at 700). ddsearch ref=teero_new (c1 settings, cut 100 -> dumpat 360): only the lp=0.05
  latdz=24 variant follows his line (+2.6 at rt 300) and it loses 6-8 in the U-turn (-6 at 360); the others end
  23-59 behind. x_graft from those dumps onto 2606 (cuts 280-300, D 1-2): closest distance 12, no graft.
  Collapse points on 962 after rt 280: vx clamped to exactly 5.0 at rt 337 (U-turn), ground jumps at 374, 398, 489
  (vy set to exactly -13.2).
- When x_graft works: two lines that are the same line shifted in time, with small state differences that a
  saturation (air-control clamp |vx| 5, a jump setting vy, a wall or floor) erases. Different lines (double kick vs
  single kick; Teero's U-turn vs ours) stay 12-175 apart in its distance and never merge.
- segf window from 2606 cut 1627 to 2606's progress at rt 1800 (own track k 1803, beam 10000, no inc): 1798, a line
  +0.5..+0.9 ahead of 2606 through the channel (lead.py) - a little room in the descent, under one tick, so it cannot
  be grafted. Forcing the descent kick lower (plan "1652 1668 4410 2943 48" + free, planforce, prefire; 2607's spot):
  1802 (worse), also with retro=3.
- Right U-turn 1 windows, 2606 cut 1095 -> 2606's progress at rt 1270 (x_ds beam 5000, 3 seeds): free 1288-1293 (one
  died), free + nokick=1100,1140 (no approach kick, reload free for Teero's far-wall pre-fire) 1325 or died;
  incforce=1 with or without nokick: 1270.006-1270.022 (= incumbent).
- Saturation: x_ds incumbent LNS on 2606 over all cuts >= 1000 (30 min, 36 jobs) and over 1590-2050 (30 min, 54
  jobs): no gain. ddsearch LNS on 962 (fast/tools/lns.py, cuts >= 600, 2 workers, 30 min, 47 jobs): no gain.
- Conclusion of this session: Teero's techniques can only be brought in as complete seeded lines (like dk2), which
  incumbent LNS then polishes. Free searches started far before a gate are 15-40 ticks weaker than the polished runs,
  and incumbent-kept windows fall back onto the incumbent, so neither can show whether a different structure (shot
  schedule + line) is better. The missing tool is a planner that builds the line and the shot schedule together for a
  section (davinci's notes reached the same conclusion for double kicks).

## Section planner v1 (tas-work/plan/xplan.py): staged x_ds windows + (time, energy) Pareto front
- Design: split a section of the incumbent at waypoints (every `stage` ticks); each stage = short x_ds windows
  (incforce=0, prefix = a kept state, gate = the incumbent's progress at the next waypoint) from each of K kept
  states x V variants; the arrivals non-dominated in (gate time, energy) are kept (then the earliest).
- Why x_ds alone fails at turns (U-turn 1, 2606 cut 1095): a free window races into the turn (+2.8 at rt 1144) and
  loses 15 in the 56 ticks after the apex, on an exit 126-188 px off the good line at |v| ~24. Its rollout lookaheads
  make it worse: rollh=30 1297/1304, rollh=60 / shh=40 / both: the beam dies. From the incumbent's own apex state
  (cut 1165) a free x_ds exit window is fine (1201.0 vs 1200).
- Planner, U-turn 1 (cut 1095 -> 1270, stage 35, K 6), plain variants (seeds / egain / kickmin): apex gate 1166.3
  (+1.3) but every arrival jumped at the apex (vy -11.5, up to y ~2040) instead of the incumbent's point-blank apex kick
  (vx -18.8 -> -29.9, staying low) -> exit gate 1213 (+13), 1256 at rt 1235. With dschain's tracking variants
  (trackfrac / shadow / shh, /tmp list in xplan docs): apex 1163.7 (-1.3, ahead), exit gate 1201.1 (+1.1), then
  1256 (+21) at rt 1235: every line runs into the single block in the left-going corridor (x 7680-7711, y 2112-2175)
  at |v| 37 -> 6, where the incumbent fires forward at rt 1206 (aim -174, along its motion) so the grenade explodes on
  that block as it passes (kick to |v| 48): a catch-up pre-fire none of the stage windows finds.
- Planner, descent after the double kick (cut 1627 -> 1800, dschain variants): stage gates -0.8, +0.9, +4.1, +3.7,
  +2.4 vs the incumbent (a single segf window from the same cut: 1798 / +0.6..0.9).
- Result: staging + a Pareto front fixes the apex collapse (with tracking variants) but not precision maneuvers
  (catch-up pre-fire on a block), so the planned lines end behind the polished incumbent. A technique still needs a
  complete line that is then polished (the double kick: dk2 + 50 LNS jobs). xplan.py is kept as an experimental tool.

## Section planner v2 (Oct 7): overlapping stages -> 2605
- The "precision maneuver" of v1 was not missing from the search: x_ds's retro shots are the block-targeted shot
  generator (each step: solid points within 100 px of the tee on 32 rays x every free fire tick in the last maxf ticks,
  exact aim and flight check). From 2606's own apex state (prefix to rt 1164, gate rt 1235, 4 dschain variants) x_ds
  finds the incumbent's corridor-block shot itself (explodes at (7679, 2112), |f| 12, cos 0.90; 1235.0-1236.7).
- v1 lost 17 of its 21 ticks at its own stage boundary: windows ending at the rt-1200 gate spent the grenade just
  before it (kicks at 1197-1199) and arrived low, so the line ran into the block 10 ticks later. One 70-tick window
  from the same stage-1 state to rt 1235: 1237.6-1241.3 (staged: 1256). The other ~3.7: v1's "1.3 ahead" apex state
  was slower (|v| 34.6 vs 39.3 after the apex kick).
- Fix (xplan.py look=35, now the default): each stage's windows run 35 incumbent ticks past the stage's gate and are
  ranked there; only their part up to the gate is kept (x_ds commitk=). U-turn 1 (2606, cut 1095 -> 1270, K 6, dschain
  variants, ~7 min on 4 cores): stage arrivals 1162.8 @1165, 1198.9 @1200, 1232.9 @1235, 1268.6 @1270, best 1268.46.
  lead.py: +0.3..0.9 to the apex, +1.0..1.5 from rt 1185 on, on 2606's own line (within 4 px from rt 1205) at the
  same speed - 2606's line ~1.5 ticks earlier.
- x_graft D=1 (beam 20000) from cut 1250 puts 2606's remaining inputs on it: **2605 (52.10 s)**, server-checked
  (TasReplay finish tick 2673, no freeze; TasServer no death / freeze / double start). Cut 1262: no graft (best
  distance 0.67). kog_full_2605.txt = kog_full_best.txt.
- xsweep.py (new): xplan over a list of sections + x_graft of the best arrivals back onto the run (cuts where the
  planned line is on the run's line, D from the measured lead) + server check, carrying gains into later sections.
- Left U-turn (2605, cut 1420 -> 1595, xsweep): the planned line is on 2605's line (within 4 px) and ahead from rt
  1495: +3.1 at 1510, +3.2 at 1525, +2.9 at 1540, +4.4 at 1555, +4.3 at 1570. It then skips the shaft lob: a kick
  off the shaft wall at 1569 (|v| 40) reaches 2605's rt-1595 progress at 1580.9 (the gate reading "14 ahead"), but
  moving sideways; planned upward from there (cut 1595 -> 1735) it is 10 behind at 1665 and 1700. x_graft of the
  U-turn part onto 2605 (cuts 1558 / 1568, D 3 / 4): no graft (no wall / floor / clamp contact to collapse the states
  before the lob). Planning the shaft from the lead state (start = the line at rt 1550, gates 1590 / 1625): the lead
  holds at +4.3..4.6 to rt 1570, then no lineage finds the lob + double kick (shaft climb at |v| 14-24 vs 43); level at
  1625, -2 at 1660. x_ds windows from rt 1558 with nokick=1559,1586/1588 (grenade kept for a stack), retro 4,
  retro2 2: 1647.5-1652.9 at the rt-1640 gate (2605: 1640). Naive splices (the line to rt c + 2605 shifted by 4 / 5)
  die in the shaft. Open: a ~4-tick left U-turn lead that needs the double kick rebuilt 4 ticks earlier.
- Sections where the planner reproduces the incumbent (lead 0.0 at every 15-tick checkpoint): 2000-2175. 1825-2000:
  equal to rt 1975, +1.5 at 1990 only in the last stage, which had no look-ahead then (fixed: the last stage now also
  looks 35 past END); no graft.
- 1270-1445 (2605): the planned line fires 1341 / 1366 / 1395 (point-blank) / 1424 / 1450 vs 2605's 1342 / 1367 /
  1394 (pre-fire, explodes 1399) / 1422 (27-tick pre-fire onto the left U-turn wall, explodes 1449) / 1453. At rt 1435
  it is in 2605's rt-1439 state (1 px, 0.15 px/t, same reload): +4.0. No graft: with D 4, 2605's 1422 pre-fire would
  have to be fired at 1418, but the planned line's grenade (1395) reloads at 1420; x_graft holds 5-7 px to rt 1439,
  then 92 px (2605's wall kick). Its own pre-fire (1424 -> 1440) costs the lead: +0.9 at rt 1480. Same pattern as the
  left U-turn lead: the planner gains ticks by spending a grenade at another time, and the incumbent's next technique
  (a pre-fire, the shaft lob) then needs a slot the new line does not have.
- xsweep on 2605 (dschain variants, K 6, look 35; sections 2000-2175, 2175-2350, 2350-2525, 1270-1445, 975-1150,
  2430-2570, 1095-1270; ~9 min per section on 4 cores): 2000-2525 reproduce the run (0 ahead); 975-1150 is 8.7 behind
  at rt 1185 (a free plan from the pickup falls into the apex failure again); 2430-2570 +1.46 at rt 2600 -> graft D=1
  from cut 2555: **2604**; 1095-1270 on 2604 +1.0..1.6 from rt 1215 -> graft D=1 from cut 1255: **2603 (52.06 s)**,
  server-checked (TasReplay finish tick 2671, no freeze; TasServer no death / freeze / double start).
- Shot schedules (Teero's video extraction vs 2605, rt 1300-1700): Teero fires at every reload, 25-28 ticks apart
  (1404 1429 1454 1479 1504 1532 1559 1584 1610 1636); 2605 leaves 31-41 tick gaps (1422 -> 1453, 1533 -> 1570 (the
  shaft lob), 1650 -> 1691). Teero gets ~1 extra kick per 300 ticks. The planner's +4 line in 1270-1445 had moved
  to a 25-tick rhythm (1341 1366 1395 1424 1450), which is what makes 2605's later pre-fires impossible on it.
- xsweep on 2603 (dschain variants with seeds 21-24): U-turn 1 (1095-1270) +1.0 from rt 1230 -> graft D=1 from cut
  1255: **2602 (52.04 s)**, server-checked (TasReplay finish tick 2670; TasServer). U-turn 1 has given a tick on
  every re-plan so far (2606 -> 2605, 2604 -> 2603, 2603 -> 2602).

## Why Teero is faster after the pickup (2602 vs teero_track.txt, analysis/*.py)
- analysis/vsteero.py (per section): we fire as many shots as he does in every section (5/5, 6/5, 6/6, 5/5, 4/4,
  3/3, 6/6, 6/6, 5/5, 4/3, 5/5). Direction key against vx (|vx| > 5): similar tick counts (ours 0-36 per section, his
  1-25; he holds a direction in 98% of the video frames). His path speed is 1.1-2.3 px/t higher in every section
  but 2175-2350 (31.8 vs 31.9). Our horizontal ramp factor (x moves vx * 1.4^-((50|v| - 550) / 2000), y unramped)
  is 0.72-0.89 at |v| 25-50.
- analysis/speedgap.py (speed at the same place, 10-tick displacement): he is faster on the straights by 1.5-4 px/t
  and equal or slower at the turns (U-turn 1 apex -0.7 / -2.7, left U-turn -0.8 .. -1.3). A steady ~5% (~1.5 px/t at
  ~33) over the ~1600 post-grenade ticks is the whole deficit (lag -17.6 at rt 1002 -> +63.6 at 2600).
- analysis/kickvs.py (our kick windows rt e-3..e+4 vs the stretches between): at our kick spots we gain more speed
  than he does over the same path (+4.69 vs +3.36 px/t per kick, 54 kicks); between kicks we lose speed faster
  (-0.189 vs -0.126 px/t per tick over 1224 ticks). The stretches where we lose >= 3 px/t more than he does are mostly
  hook-heavy for us (rt 1459-1497: hook grabbed 22/38 ticks, ours -11.3 his -4.6; 1530-1548 13/18, -6.5 / -1.8;
  1799-1827 21/28, -14.8 / -10.6; 2294-2323 19/29, -19.9 / -1.9).
- eaudit 2602 (rt 966-2602, d(v^2/2)): explosions +18,258, hook -9,842, direction -8,731, collisions -1,291,
  jump -69, gravity/rest +1,701. Over half of the kick energy goes into hook and direction braking. Above 15 px/t
  (hook_drag_speed) a hook pull is applied only if it does not raise |v|, so a high-speed hook can only turn or brake.
- So the missing techniques are not stronger rockets but carrying the speed: turning with less braking (rope geometry
  that rotates v at near-zero loss, wider lines, kicks that turn), and keeping |vy| low on horizontal straights
  (vertical speed lowers the ramp on vx). The searches cannot see this: their windows rank by time to a gate 35-70
  ticks ahead with a v^2 - y energy credit, and a line that brakes less pays off over hundreds of ticks.
- xsweep rounds on 2602 (dschain_variants_b, rounds 3): U-turn 1 round 0 / 1 +0.46 / +0.49 (no graft), final maze
  round 0 +0.36; final maze round 1 (seeds +100): +2.0 at rt 2565 on 2602's line -> x_graft D=2 from cut 2555:
  **2600 (52.00 s)**, server-checked (TasReplay finish tick 2668; TasServer).

## Braking cost and far low-loss hooks in x_ds (brakew=, rotfar=)
- Our worst stretch (2602 rt 1458-1477, left U-turn exit): two held hooks (anchors (223, 2670), (798, 2944)) turn
  2-4 deg/tick but cost 0.3-0.76 px/t each tick, |v| 37.5 -> 24.9 for ~50 deg: the anchor drifts behind the tee and
  the pull turns into braking. A pull just past perpendicular turns as fast at ~0 loss (above 15 px/t a pull only
  applies if |v| does not grow); RotAims only aims such pulses at anchors 47-122 px away (same-tick grabs).
- New in x_ds: rotfar=1 adds, per side, the aim whose pull at the grab (anchor up to 362 px, flight 80 px/t, idle
  prediction of the tee) turns most without raising |v|, plus the farthest anchor turning >= 80% of that (slower
  drift). brakew=W adds W ticks per px/t of the lineage's speed lost to the hook pull and to the direction key (vs
  pressing along vx) to the ranking (not to the reported gate time).
- Test on 2600 (x_ds windows, incforce=0, beam 3000, dschain variants v0 / v1, gate = 2600's progress):
  1440 -> 1510 (2600: 1510): base 1508.77 / 1507.05; brakew 0.3 1507.65 / 1507.10; brakew 1 1507.94 / 1508.98;
  rotfar 1507.39 / 1506.38; rotfar + brakew 0.3 1507.42 / 1506.16. 2275 -> 2345 (freeze-lined): v0 dies in all
  configs (also from cuts 2260 / 2266 / 2290), v1 2345.03-2345.27 in all configs. -> rotfar ~ -1 tick on the turn
  exit, a light braking cost helps a little, a heavy one hurts. dschain_variants_rf.txt = variants_b + rotfar=1
  brakew=0.3.
- More windows on 2600 (raw gate tick, |v| at the gate; base -> rotfar=1 brakew=0.3; dschain variants v0 / v1):
  1520 -> 1590: 1588.07 -> 1587.57 (|v| 26.7 -> 33.0) / 1583.33 -> 1582.85; 1390 -> 1460: 1460.00 = 1460.00 /
  1458.96 -> 1458.58; 1785 -> 1855: base dies (both) -> 1864.71 / 1856.29; 1165 -> 1235: 1235.01 -> 1235.54
  (|v| 53.4 -> 57.9) / 1235.35 -> 1236.16 (53.5 -> 56.0); 2120 -> 2190: 2189.00 = 2189.00 / 2189.84 -> 2190.00
  (40.0 -> 42.2). With 1440 -> 1510: earlier in 5 of 12, equal in 2, alive where base dies in 2, later but 2-4.5 px/t
  faster in 3. (x_ds's printed GATE t is the raw arrival; the energy credit only ranks.)
- A/B xsweep on 2600 (dschain_variants_b vs _rf, same seeds, cores 2 each; ranked arrival vs the run): left U-turn
  1583.73 vs 1587.54 (both skip the shaft lob; no graft), U-turn 1 1304.54 vs 1304.93 (1305), final maze 2594.63 vs
  2594.84 (2595), 1825-2000 2034.68 (plain; 2035). No gain either way: with incumbent-tracking variants the planner
  reproduces the run's own hooks.
- ... the A/B's last section: 1825-2000 plain 2034.68 (0 ahead), with rotfar + brakew 2033.63 (+1.0 from rt 1975 on
  2600's line) -> x_graft D=1 from cut 1985: **2599 (51.98 s)**, server-checked (TasReplay finish tick 2667;
  TasServer). Its line differs from rt 1828 (other hook aims, several off the 32-angle grid); the section's energy
  budget is the same as 2600's (rt 1825-1989: expl +2421 / +2387, hook -1123 / -1087, dir -1519 / -1542), so it is a
  better line found by the search with the new options, not a large braking saving.
- Free variants (plan/free_variants.txt, no incumbent tracking) on 2600's 2000-2175, xplan stages: with rotfar +
  brakew 2071.64 @2070 / 2108.00 @2105; without 2071.06 / 2110.91 (run 2070 / 2105). The new hook aims help free
  search by ~3 ticks over two stages, but free search stays behind the polished run; stopped there. The productive
  setup is incumbent-tracking variants + rotfar + brakew (dschain_variants_rf.txt), now the sweep's default choice.

## Teero's hooks from his video (Oct 7; user uploaded the video, teero/hooks/)
- The video is "Aip-Gores TAS in 50.720": Teero's run is a TAS (user: no human can play this fast). The camera is
  centered on Teero; Tater is a solo ghost. Registration: 1.75 px per world unit; video time t shows the track's tick
  (t - 1.383) * 50 - 2.6 (the input CSV's s_since_start is 2.6 ticks ahead of the track; median error 7 px).
- Hook timeline (teero/hooks/teero_hooks.csv, k 978-2537), vs 2599 (rt 966-2595), pull angle against v at |v| >= 15:
  Teero hooked 492/1551 ticks (32%) in 134 grabs (median 3 ticks); 2599 721/1629 (44%) in 312 grabs (median 1 tick).
  Teero: forward 22%, low-loss (90-100 deg) 24%, braking (> 100) 47%; 2599: 7%, 14%, 73%. So ~213 braking hook ticks
  for him vs ~495 for us: he hooks less, longer, and brakes with the hook less than half as often.
- Rebuilding his run from his inputs: x_tig hooks=teero/hooks/teero_hooks.csv tolh=N (new) holds / presses the hook
  only where he hooks (+-N ticks), releases where he does not, and aims new hooks at his anchor. From 2599 at rt 1100
  (off 13.25): tolh 1 kills the beam at rt 1114 (the extracted timing is not that exact), tolh 3 survives but drifts
  (d 9 -> 51 px by rt 1119, 1.2 ticks behind): at the same place our state is slower than his. From rt 990 (right after
  the pickup, off 17.55, both ~14 px/t) to rt 1160: no hooks 12.7 behind, tolh 3 16, tolh 2 25. The tracker matches
  him to rt 1003 (d 6-19 px), our pickup double kick gives his speed (|v| 28.5 at rt 1009), then over rt 1012-1030 he
  moves 27-32 px/tick and we 23-26 (|v| 25.5-27.8): the gap opens in the first 40 ticks after the pickup. Hook timing
  from the video does not make his run reproducible (+-1-3 ticks, anchors +-10 px).
- CSV vs track: CSV jump rising edges match the track's velocity jumps within 0.3 ticks (23 jumps), so the CSV and
  the track share a clock; the video renders the tee 2.6 ticks later (the hook timeline uses the track's clock).

## Right after the pickup (Oct 7): 2598
- 2599 vs Teero at the same places (analysis/speedgap.py, 3-tick steps): equal ~14 px/t to rt 1002; after our pickup
  double kick (lob fired 980 -> 1005 |f| 11.6 cos 0.57, point-blank fired 1005 -> 1006 |f| 9.0 at 70 px) he is
  +0.7..+1.3 px/t faster, +1.9..2.3 before our 1031 kick, +0.3..1.2 to 1059, +3.4..3.7 at 1065-1074 (after our weak
  1058 kick, |f| 9 cos 0.71); lag 1002 -17.8 -> 1100 -13.2. Our hooks here are 1-tick taps, his 12-13-tick holds.
- xsweep from cut 966 (the pickup): stage 0 (-> 1001, ranked at 1036) 1040.1-1041.3 vs 1036: the plans kick at once
  (fire 979 -> 980, +9 at rt 998) and never rebuild the lob + point-blank double kick (|v| ~21 instead of 31 after
  1008). Kept the double kick: cut 1007.
- x_dbl xt=X (new: horizontal objective, ticks of free flight holding right with the speed ramp to reach x = X)
  on 2599's lob (fired 980, explodes (5247, 1946) at 1005), zone at the wall, xt 5900: a same-step double kick gives
  vx 30.56 at 1005 vs 2599's 30.23 at 1006: ~0.3 tick; 2599's double kick is near this lob's best.
- xsweep 1007:1095 on 2599 (dschain_variants_rf: tracking + rotfar + brakew 0.3): +0.3 at 1037, +0.8 at 1082,
  +1.0..1.9 at 1112-1129 -> x_graft D=1 from cut 1109: **2598 (51.96 s)**, server-checked (TasReplay finish tick
  2666; TasServer). eaudit 1007-1109: expl +1139 vs +1005, hook -303 vs -350, |v| 44.1 vs 42.3 at 1109.
- Round 1 (seeds +100) of the same sweep on 2598: +0.7 at 1097, +1.3 at 1112, +2.2 at 1127 -> x_graft D=1 from cut
  1111: **2597 (51.94 s)**, server-checked (TasReplay finish tick 2665; TasServer).
- xsweep on 2597 (dschain_variants_rf2: seeds 31-34 + rotfar + brakew): post-pickup again +2.0 at 1127 but off 2597's
  line (26-32 px), no graft; U-turn 1 (1095-1270) +0.8 at 1125, +1.0 1155-1245, +1.6 at 1275 -> x_graft D=1 from cut
  1282: **2596 (51.92 s)**, server-checked (TasReplay finish tick 2664; TasServer).
- Same sweep, 1270-1445: +1.0 at 1315, +2.5 at 1345, +3.2 at 1375, +4.0 from 1405 to 1465 on 2596's line -> x_graft
  D=4 from cut 1458 (the window's look-ahead part, past the left U-turn's pre-fire that blocked the 2605-era lead):
  **2592 (51.84 s)**, server-checked (TasReplay finish tick 2660; TasServer).
- hookref A/B on 2596 (x_ds windows, rf variants v0 / v1, raw gate tick): 1007->1077 rf 1077.00 / 1076.90, hr0.5 =,
  hr1.5 =, hard 1079.4; 1165->1235 rf 1235.51 / 1235.99, hr0.5 1235.82 / 1235.47; 1440->1510 rf 1507.89 / 1506.67,
  hr0.5 1507.58 / 1506.36, hr1.5 1507.63 / 1510.84, hard 1510.3 / 1510.5; 1520->1590 rf 1582.71 / 1581.57, hr1.5
  1581.92 / 1590.03; 1785->1855 rf 1877.82 / 1855.00, hr0.5 1862.05 / 1855.00, hr1.5 1854.41 / 1855.00; 2120->2190
  rf 2189.71 / 2189.64, hr0.5 2189.79 / 2189.56; 2430->2500 rf 2500 / 2500, hr0.5 2499.63 / 2499.77. Soft prior:
  a different search, sometimes 0.3-0.8 better, sometimes worse; hard prior worse. Kept as an extra variant set.
- Left U-turn + shaft (2592, xsweep 1420:1640, look 60, K 8, rf variants): stages +4.4 @1515, +4.2 @1550, then -1.1
  @1620. The +4 line is on 2592's line (1-2 px) to the shaft floor (rt 1568) but fired a point-blank at 1540, so its
  grenade is not back for 2592's lob (fired 1561, explodes at the block 1586 with the point-blank 1586 -> 1587; floor
  jump 1575). With nofire=1533,1556 (dschain_variants_rf_nf.txt): +1.6 at 1556-1572, and every plan fires short
  shots (~1560 -> 1562-1567, ~1588 -> 1589-1592) instead of the lob + point-blank stack: -5 at 1596, -17 at 1620.
  The shaft double kick is the wall for any faster left U-turn; x_ds does not rebuild it from a different state.
- autosweep.py (endless xsweep passes; variant sets rf / rf_hr / rf2 with new seeds) on 2592, pass 0 (rf): sections
  1007-1270 +0.7..1.1 (no graft), 1420-1560 0/12 grafts (the shaft), 1590-1875 0 ahead, 1825-2000 failed (stage 2 had
  no arrival: xplan now retries such a stage without look-ahead), 2000-2175 +0.8 at 2120, +1.0 at 2195 -> x_graft D=1
  from cut 2188: **2591 (51.82 s)**, server-checked (TasReplay finish tick 2659; TasServer).
- ... same pass, 2350-2525 -> **2590 (51.80 s)**, server-checked (TasReplay finish tick 2658; TasServer).
- autosweep pass 1 (dschain_variants_rf_hr: + Teero hook prior hrefw 0.5) on 2590: 1700-1875 +1.0 -> x_graft D=1 from cut 1905: **2589 (51.78 s)**, server-checked (TasReplay finish tick 2657; TasServer).

## Where Teero gains on 2589, turn by turn (Oct 7, night)
- The post-pickup map is one winding corridor (solid walls lined with freeze, no tele/speedup tiles): no route
  choice, and our line is his within 0-16 px everywhere. The 64-tick deficit (lag -17.6 at rt 982 -> +46.8 at 2572,
  in his labels) is speed only: U-turn 1 and its exit 1142-1262 +11, left U-turn 1472-1572 +10, shaft top + channel
  1592-1802 +13, then 4-7 per stretch. He leaves the turns faster and keeps it on the straights.
- Pickup double kick: his displacement over 1005-1046 is 26.8 px/t vs our 26.0 (vx ~31.5-32 vs 30.23, 0.9 ticks by
  1046). His shots (CSV) are ~1.5 ticks earlier than ours for the lob and ~1 for the point-blank, i.e. the same
  pattern. A lob fired at 979 lands on the block in the step to 1006, the same step as a point-blank fired at 1005,
  but x_dbl (xt 5900, zone at the block) gets only vx 30.44: the approach has to be a tick slower. Lobs fired at 978
  or earlier cannot reach the block top (max lob height 357 px). Not reproduced.
- U-turn 1 (his labels L = our rt + ~17): he holds one hook from L1144 to L1166 on the dividing wall's tip
  (anchors detected on its right face x ~9311). Between L1156 and L1166 his |v| grows 18 -> 25 while the velocity
  turns ~90 deg, which a hook alone cannot do (above 15 px/t it only turns or brakes), so he kicks in the turn as
  well: his CSV shots at L1140 (aim 20 deg, a lob from our ~1123 toward the outer wall) and L1166.7 (our ~1150), then
  he leaves at vx ~-37 (displacement 30-31) while we fly at -25..-28 until our 1169 kick. We kick weakly at 1113
  (|f| 7, braked away in the turn), at the apex 1145 (cos 0.24) and at 1169.
- New tool x_pend (src/tas/x_pend.cpp): brute force of one held hook through a turn (press tick, aim 0.5-1 deg,
  dir +1/0 then -1 from a switch tick, release tick; optional anchor box), end states scored by projection onto the
  incumbent (lead + spw x speed difference), best ones written as prefixes. 0.6-3.6M steps in ~1 s. U-turn 1: best
  swings -0.1 (anchor on the wall top 400 px back, |v| 23 vs 28) and -2..-4 (anchor at the tip) at rt 1150-1156;
  x_ds from them reaches the incumbent's rt-1200 progress at 1223+ (the swing states drop onto the floor slowly).
  We arrive at 52 px/t; a hook (3 px/t^2) cannot hold a tight swing at that speed.
- x_ds windows from cut 1100 with nokick=1105,1128 (no 1113 kick): 1230.9-1231.6 at the rt-1230 gate (incumbent
  1230, same windows with the kick 1230.2-1230.5); the search still kicks at the apex (1146). Apex lobs (x_lob on
  that line): fired 1129-1131 they land on the outer wall at 1145 with |f| 12 but cos ~0 (pure redirect); x_ds from
  them 1229.74 (+0.26, lower energy) to 1233. Lobs fired 1120-1125 land at 1145-1149 ~100 px from the tee.
- Teero tracker (x_tig, vw=1.5) from 2589 at rt 1612 / 1640, 2 seeds each: best +0.9 at 1752, others -2..-5.
- autosweep pass 2 (dschain_variants_rf2, seeds 31-34) on 2589: 1700-1875 again +1.0 at rt 1880 -> x_graft D=1 from
  cut 1892: **2588 (51.76 s)**, server-checked (TasReplay finish tick 2656; TasServer).
- Full post-pickup plan (chain1: xplan on 2590 from rt 1007 to 2575, stage 35, look 35, K 6, rf variants, cores 2,
  ~4.5 h): stage leads +0.1..+6.4 (left U-turn and shaft), +1..+4 through the channel, ranked +1.15 at rt 2585; its
  line finishes at 2589 (x_ds gate=finish). Against 2588 it is +1.0 on the line (0 px) at 1360-1455, +4..+6 in the
  left U-turn (1493-1558, 0-6 px), +12 at 1576 (off the line: no shaft stack), +1.4..+3.0 at 1614-1799, and back to
  -1 after the rt-1800 U-turn. xsweep's automatic graft only tried its two latest cuts (1802 / 1812: no finish);
  multigraft.py (scratch: every on-line cut with lead >= 1, largest D first) found x_graft D=4 from cut 1503:
  **2584 (51.68 s)**, server-checked (TasReplay finish tick 2652; TasServer). Lesson: try grafts at every on-line
  cut of a long plan, not only the latest ones.
- Two more full plans on 2584 from rt 1007 (chain2: rf2 variants, seedadd 500; chain3: rf_hr, seedadd 700): leads
  +0.2..+3.5 through rt 1532 (chain2) / +1.8 at 1392 (chain3), then both collapse (chain3 -5.5 at the left U-turn
  1462, chain2 -2.7 at the shaft 1602 and -8.5 at the rt-1800 U-turn); stopped at stages 28 / 24. The full plans
  keep losing at the same three places (left U-turn, shaft, rt-1800 U-turn); their value is the stretches before.
  multigraft of chain2's stage-14 line (s14_k1_v1, +1.5..+3.3 on 2584's line at 1382-1534) onto 2584: x_graft D=1
  from cut 1395: **2583 (51.66 s)**, server-checked (TasReplay finish tick 2651; TasServer). 56 other cuts / D: no
  finish.
- Plans on 2583 starting after the trouble spots: chain4 from rt 1640 (rf, seedadd 900) +0.3..+2.2 to 1780, then
  -3.3 at the rt-1800 U-turn and -5..-7 after (stopped); chain5 from rt 1830 (rf2, seedadd 1100) +0..+2.1 all the way,
  ranked +1.2 at rt 2578 (a steady 1-tick lead from rt 2220 on, 0-2 px on 2583's line). multigraft of chain5's best
  line: x_graft D=1 from cut 2573: **2582 (51.64 s)**, server-checked (TasReplay finish tick 2650; TasServer).
- xsweep allcuts=1 (new: grafts at every on-line cut, spacing 4, largest D first, batches of cores/2, stops at the
  first batch that beats the run; a plan that ends behind still offers its top 3 arrivals) + autosweep with long
  sections (1007:1450, 1270:1800, 1640:1830, 1830:2200, 2100:2575; vsets/xs passthrough), on 2582: section
  1007-1450 (rf, seed0 3000, 23 min) -> x_graft D=1 from cut 1423: **2581 (51.62 s)**, server-checked (TasReplay
  finish tick 2649; TasServer).
- Same pass: 1270-1800 3 ahead, 0/12 grafts; 1640-1830 (9 min plan) 3 ahead, 48 graft jobs -> x_graft D=1 from cut
  1750: **2580 (51.60 s)**, server-checked (TasReplay finish tick 2648; TasServer).

## Speed session (Oct 8): where the post-grenade speed goes, and a velocity-aware time-to-go field
Goal of the session: < 2500 (50.00 s) from 2580, i.e. -81 ticks; the user's hint: no new tricks, the post-grenade
part needs higher speeds and keeping them. Tools: `sp/` (analysis scripts: trk.py, prof.py, gap.py, bound.py,
lead.py), `x_tfield` (new), x_ds `tfield=` / `tfmode=` / `nokickall=` (new).
- teero_track.txt is not in git; rebuilt from teero/hooks/hooks_raw.csv (Teero's position per video frame, k 978-2538,
  interpolated per label) - same registration as the old track, post-grenade only.
- Speed budget of 2580 (rt 1000-2580, |v| changes): kicks +453 px/t (54 kicks, sum |f| cos = 418 of a possible
  12 x 64 = 768), hooked ticks -365, everything else -85. On straights the 351 hooked ticks cost -130 (the 82 ticks
  pulling 105-135 deg behind the velocity cost -104 for 336 deg of turning; the 200 ticks at 90-105 deg only -27 for
  693 deg). eaudit: direction braking -9,155 (v^2/2), as large as the hook's -8,713, almost all in the 50 ticks
  before a turn.
- 24 of the 54 kicks lose their speed gain within 30 ticks (kicked right before a turn; e.g. 1113, 1251, 1781, 1930,
  1961, 2061, 2097, 2271, 2419, 2448). Teero fires as often as we do (53 vs 54); the difference is where/when.
- Time below 30 px/t: 406 ticks (25%). Value of speed at turn exits (bound-style estimate on our own line: +12 px/t
  carried from each sink to the next speed peak): ~190 ticks in total, 18-26 each after U-turn 1, the shaft top, the
  S-bend, rt 2112 and the hop. That is where "higher speeds, kept" pays; approach kicks are worth little.
- Hook physics at speed: above 15 px/t a pull applies only if |v| does not grow, so a held hook turns losslessly only
  while the anchor stays ~90 deg off v; a circular swing at radius r needs v^2/r <= ~3 -> ~34 px/t at the hook length
  (380). U-turn 1's apex (21.7 px/t) is exactly sqrt(3 x 135) for its 135 px swing around the divider tip.
- Free x_ds (no incumbent tracking) is far below the polished incumbent: U-turn 1 (cut 1095 -> rt-1250 gate) 1263-1265,
  1275 without the approach kick; dip + climb + left corridor (1250 -> rt 1420) 1475 (incumbent 1250 / 1420). Its
  ranking (geodesic lag + energy credit) races into turns.
- **x_tfield** (src/tas/x_tfield.cpp, tfield.h): T(x, y, heading, speed) = ticks to the finish under a reduced
  point-mass model (gravity, the horizontal ramp, lossless hook turning up to lat x 3 px/t^2 scaled like the hook,
  hook + direction braking, an average kick acceleration, freeze kills, solid slides like MoveBox), by value
  iteration on 16-px cells x 24 headings x 16 speeds (20.5M states, ~15-25 min on 3-4 cores). Along 2580 the model's
  T is 1.06-1.13 x the real remaining time. x_ds tfield=FILE ranks by race tick + T (tfmode=1).
  - U-turn 1 window, free: 1252.4-1252.7 at beam 3000 (old ranking 1263-1265), **1248.63 at beam 12000 (1.4 ahead of
    the incumbent, E 1169 vs 386)**: every kick 3-7 ticks earlier (1139, 1166, 1216, 1245 vs 1145, 1169, 1223, 1251).
  - Climb window (1250 -> 1420): 1446 at beam 3000 and 12000 (old free 1475; incumbent 1420): the field rates the
    incumbent's states better (t + T 2683.5 vs 2712.6 at rt 1340) but the free beam never contains its dip line
    (turning kick 1251 at the top of the descent, block kick 1285, climb kick 1308).
  - Full free run from rt 1007 with the field (beam 3000): 113 behind at rt 1927 (most of it in 1287-1530). Not a
    replacement for the incumbent-tracking variants; with them (rf variant + field) windows are within +-0.3 of
    the variant without the field, except U-turn 1 (+1.25 vs +0.66).
  - tfmode 2 (field value of v vs the incumbent's v at the same place) and 3 (penalty only): NOGATE (rewards braking).
- Teero's U-turn 1 (his kicks vs ours, our rt): pre-fire exploding at the end wall at the apex, point-blank exit
  kick ~1150 (ours 1169), then every kick ~18 ticks earlier than ours to rt 1250 (his gain there: 4 ticks in the exit,
  5 more by 1250). Forced reproduction: our 1108 shot stripped, lobscan pre-fires (fired 1118-1132, exploding at
  x 9504-9536, y 1966-2140 at 1143-1150), x_ds rf + field from each: best fired 1127 -> 1145 + point-blank 1152:
  **1247.78 at the rt-1250 gate (2.2 ahead, E 1585)**, but it is 3.5 behind at the apex (no approach kick) and its
  lead comes from a 1233 kick that takes the slot of the incumbent's 1251 turning kick into the dip: continued to rt
  1420 it is 10-15 behind (7 lost in the dip at 1252-1290); with that slot kept free (nokick 1222-1245) it ties the
  incumbent at 1250 and ends 4.8 behind at 1420. Every local re-timing of the kick phase meets the next section's
  critical kick (the 25-tick reload couples all sections) - the same wall as the shaft double kick and the planner.
- **Value of speed by place (x_ds boost=12 at a cut, rf tracking variant, gate ~150 ticks on):** at turn exits into a
  straight +12 px/t saves 3.6 (U-turn 1, 1150), 3.7 (shaft top 1615), 4.4 (1812), 5.4 (S-bend 2003), 4.0 (corner 2116),
  6.4 (hop 2308) ticks; on approaches it saves nothing (1114: -1.0, 1931: -0.4, 1782: 0) while taking 6 px/t away costs
  1.5-4 (26 at 1782: crash); at the start of vertical sections it costs 12-33 (1418, 1582, 1999, 2449: the V-turn at
  their end overshoots). Speed only pays where the next stretch can carry it: right after turns.
- Teero vs 2580 by place: both fire at the same spots (his 53 fires map onto ours within +-3 ticks of position except
  U-turn 1 and the top-left corner). At the same places in 1640-1790 his displacement is a steady 1-4 px/t higher
  even where we fly hook-free; the offset is set at the shaft top (rt 1640: 40.6 vs 34.4): his top-turn shot is aimed
  -138 deg (ours -113), he descends steeper (vy 33 vs 29 at 1638) and the same upward-aimed descent kick is better
  aligned for him (ours cos 0.53). A grid of top-turn aims (-105..-150 deg x fire 1608-1610) + x_ds gives at best +0.47
  at 1700; tracking his descent positions (tref + ttrack) reproduces the better kick (cos 0.76, |v| 41 at 1640) but our
  search then cannot follow his channel line (-5.3 at 1700).
- Top-left corner (rt ~2112): Teero fires his corner shot where we fire our approach shot (rt ~2097 positions) and
  gets a descent kick right after the corner; he takes a line 60-80 px lower through the corner. Forced versions
  (lobscan pre-fires 2096-2106 onto the corner ceiling + x_ds; nokick re-searches from 2062/2084; tref to his line)
  end 3-19 behind at rt 2200: the descent kick gains ~3, the approach without our kick loses 5.5.
- Searches with field ranking + survival check off (surv=0), finer hook / fire angles (128 / 256), shadow inputs for
  the free search, the trap-penalty field mode: no gain. Free field search at U-turn 1: beam 3000 1252.4, 12000
  1248.63, 48000 1248.28 (diminishing); past the dip (gate 1300) 1302.0.
- **x_sem** (new): semantic replay of a run - its keys, hook presses re-aimed at the anchors its hook grabbed (rebuilt
  on the original ray, the stored hook position is quantized) and shots re-aimed at its explosion points, optionally
  time-shifted. Exact from the run's own state (2580 from cuts 1640 / 1900). From a perturbed state (+-1 px or
  +-0.1 px/t at rt 1640) it dies 40-70 ticks later, raw replay 40-43: the run's freeze margins, not its aims, make it
  knife-edge; a plan does not transfer without search.

## Shot structure from Teero, turn by turn (Oct 8): U-turn 1 -> 2576
- Teero's shots mapped onto our path by position (teero/teero_inputs_0-3131.csv fire rows, label = s_since_start x 50
  - 2.6, projected on 2580): the same spots as ours almost everywhere, except: U-turn 1 (he skips our approach shot
  1108 and pre-fires ~1122 so it lands at the exit with his point-blank), an extra shot at ~1490 (left U-turn exit; our
  gun idles 1493-1517), the top-left corner (2095 / 2119 vs our 2086 / 2113), no shot at 2418 (our approach kick before
  the final maze) and the final maze (2519 vs our 2509).
- Implied |v| (sp/teerov.py: his smoothed displacement solved through the horizontal ramp): 3-10 px/t above ours on
  most straights, e.g. 40 vs 30 at the U-turn 1 exit, 57 vs 46-53 at 1210-1250, 55-65 vs 50-53 at 1750-1780.
- Energy (eacct, 2580 1000-2580): kicks +34390, hooked ticks -30287 (865 of 1580 ticks hooked; Teero ~32%).
  sp/hookloss.py lists the braking hook episodes (e.g. 1971-1984 -10.9 px/t, 2433-2438 -8.9, 1558-1568 -8.4).
- **x_pfscan** (new, src/tas/x_pfscan.cpp): for fire ticks F0..F1 on a run's own states and every aim (0.1 deg), the
  grenade's exact flight and the kick the run's tee would get where it explodes in E0..E1, scored along the run's
  velocity or dir=dx,dy. Finds pre-fires that can land at a turn exit. U-turn 1 from a line without the approach shot:
  fired 1126 aim 30 -> 1147 at (9490, 2080); fired <= 1125 every lob hits the divider top (y 1888) first. The rt-1800
  U-turn, the S-bend and the final-maze entrance have no pre-fire that reaches the exit in time (grenade too slow).
- **U-turn 1 restructure:** (1) x_ds from cut 1082 with nofire=1082,1118 nokick=1095,1132 nokickall=1 (no approach
  kick; nofire alone is not enough, retro shots still fire then) -> runs/opt/u1k_r1 (0.9 behind at 1200 with single
  kicks). (2) its inputs to rt 1125 + a forced shot at 1126 aim 30 deg (runs/u1s/p_1126_30.txt). (3) x_ds rf variants
  from there to gate 1200: +2.4 (kicks 1146 pre-fire 8.0/cos 0.5, 1151 point-blank 12/0.83, 1184): only -0.9 at the
  apex. (4) continued from the same prefix to 1320 all variants die in the dip (-2.5..-45); re-searched from its cut
  1165 instead: variant 3 (seed 23) **+4.0 at rt 1320, on 2580's line (0 px) from 1240**, kicks 1146, 1151, 1184,
  1205, 1247 (dip turn), 1281, 1304 = Teero's schedule (his 1147, 1176, 1204, 1244, 1271, 1302). (5) multigraft D=4
  from cut 1315: **2576 (51.52 s)**, server-checked (TasReplay finish tick 2644; no freeze). All 19 D=4 cuts 1243-1315
  finished.
- Lesson: our searches never find these structures (the approach without the kick ranks worse until the stacked exit
  pays off), and a forced structure only holds through the next turn when the re-search starts a little after the
  change (cut 1165, not at the end of the 1200 window), so the kicks after it can be re-timed.
- **x_opt** (new, src/tas/x_opt.cpp): simulated annealing on a semantic plan of a window (hook holds with anchors,
  shots with explosion points or angles, direction / jump keys; mutations shift / re-aim / merge / split / drop / add,
  shift the shot schedule), exact CFastG evaluation (~8000 evals/s/thread for 200 ticks), score lead + kv x speed along
  the run's direction - lateral penalty at the window end, survival tail (the run's own plan), off-route = dead,
  prefix= / shift= / ref= / shots= / field=. On 1636-1850 it finds +0.76 and +1.7 px/t, but x_ds continuations from it
  are 1-2 behind those from the run itself by rt 2060 (the end state was ahead by being faster before the S-bend):
  a window-end score is misleading unless the window ends after a turn. Finish-time windows (cut 2300 -> finish):
  no gain. SA cannot repair a dying plan (degenerate survivors), so it is a polisher, not a structure finder.
- Other Teero-structure attempts on 2576 (all the same recipe; none beat the run):
  - Left U-turn exit, extra shot (our gun idles 1490-1514 while Teero fires at ~1486): x_pfscan from 2576's states
    gives 1491-1497 shots exploding 1500-1508 (+6..+9 along v); forced at 1492 (aim 27.6) the kick throws the tee into
    the small solid block at (1824, 2591) at 1506 (12-13 behind at 1550); forced at 1497 / 1495 x_ds never rebuilds
    the shaft (-22..-60 at 1640 or no gate). Even from 2576's own state at 1490, x_ds rf variants reach the rt-1640
    gate 6-31 ticks late: the shaft double kick (1551 lob + 1576 point-blank) is not reproducible by x_ds from any
    other cut. Moving the 1466 shot to 1462 (needed for a 25-tick slot at ~1487): every scanned aim either kills the
    tee within 6 ticks or explodes 110-140 px from it (forward-up aims like Teero's hit the ceiling behind the tee).
  - Top-left corner without our approach shot (nokick 2071-2096): x_ds finds Teero's stack by itself (lob 2095 +
    point-blank 2120, both exploding at 2120), 1.6-2.4 behind at 2160 with 8-10 px/t more speed, but every stage-2
    re-search (tracking variants from cuts 2125 / 2135, free + field variants) ends 2-4 (tracking) / 13-21 (free)
    behind at rt 2230 after the bottom-left turn. (nokick blocks shots FIRED in the window, not explosions in it.)
  - rt-1800 U-turn without the 1777 approach shot: 6-11 behind at 1880 (no pre-fire reaches its exit in time).
  - Hop without the 2266 turning kick (cos 0.18): 16-19 behind at 2330. x_opt on 2250-2335: +0.14 / +1.5 px/t only.
  - Final maze without the 2414 approach shot (to the finish): 2583-2588 (run 2576); no pre-fire reaches 2430-2455.
- plan/structsweep.py (new): the recipe automated per shot (stage 1 with the shot forbidden around its time, stage 2
  re-searched from a later cut, multigraft, server check).
- **x_opt on the restructured U-turn 1 -> 2575:** x_opt cut 1083 end 1200 (kv 0.3, tail 20, 4M iters, 2 threads) on
  2576: point-blank 1151 -> 1148 and new hooks, +1.42 at 1200 and +2.8 px/t (its tail dies). Stage 2 x_ds rf variants
  from its cut 1175 to 1320: v2 1319.67, +1.0 on 2576's line (0 px) from 1180 to 1250; multigraft D=1 from cut 1203:
  **2575 (51.50 s)**, server-checked (TasReplay finish tick 2643; no freeze). x_opt is useful as the stage-1 polisher
  of a new structure (its window end must be followed by an x_ds re-search through the next turn).
- Polish sweep (xsweep rf, allcuts) on 2576: 1095-1270 +0.8 (no graft cuts), 1200-1380 +1.3 at 1414 (graft 0/2; the
  line continued through the left U-turn ends 4-15 behind at 1500).
- **optsweep (plan/optsweep.py) on 2575**, two sweeps in parallel on copies: left U-turn window (x_opt 1330-1460,
  stage 2 1450 -> 1540: 1537.93) -> graft D=1 at 1486: **2574**; shaft top (x_opt 1560-1660: +1.28 lead, |v| 44.4
  vs 38.6; stage 2 1645 -> 1760: 1757.05) -> graft D=2 at 1700: 2573; top-left corner (x_opt 2030-2140, stage 2 to
  2230) -> graft D=1 at 2149: 2573 (on 2574). Splices: the left U-turn graft only rejoins 2575 (+1) at its rt 1574,
  the shaft-top line leaves 2575 at 1566 (overlap: that splice froze); shaft top + corner (os1 run to rt 1900, then
  the os2 run's inputs from its rt 1901, both being 2575's state at 1902) -> **2572 (51.44 s)**, server-checked
  (TasReplay finish tick 2640). The left U-turn window is re-run on 2572.
- Final maze window (x_opt 2380-2480, stage 2 from 2470 to the finish: three variants finish 2571 on the os2 base);
  spliced onto 2572 (2572's rt 2300 state = the maze line's rt 2301): **2570 (51.40 s)**, server-checked (TasReplay
  finish tick 2638).
- optsweep round 2 on 2570 (runs/os3, 4 cores sequential): 1800 U-turn window (x_opt 1700-1840, stage 2 to 1960: 1958.47) -> graft D=1 at 1907: **2569 (51.38 s)**, server-checked.
- Same sweep: S-bend window (x_opt 1880-2020, stage 2 to 2100: 2097.56) -> graft D=2 at 2080: **2567 (51.34 s)**, server-checked.
- optsweep round 3 on 2567 (runs/os4, x_opt 6M iters): dip window (x_opt 1200-1300, stage 2 to 1420: 1419.08) -> graft D=1 at 1349: **2566 (51.32 s)**, server-checked.
- Same round: shaft-top window (x_opt 1550-1650, stage 2 to 1760: 1755.58) -> graft D=3 at 1739: **2563 (51.26 s)**, server-checked.
- Same round: 1800 U-turn window (x_opt 1720-1840, stage 2 to 1960: 1957.92) -> graft D=1 at 1914: **2562 (51.24 s)**, server-checked.
- optsweep round 4 on 2562 (runs/os5, kv 0.4): left U-turn window (x_opt 1330-1450, stage 2 to 1540: 1535.10) -> graft D=3 at 1477: **2559 (51.18 s)**, server-checked.
- Same round: shaft exit / channel window (x_opt 1600-1700, stage 2 to 1800: 1795.40) -> graft D=2 at 1746: **2557 (51.14 s)**, server-checked.
- optloop round 0 (runs/loop/r0): window 1984-2112 (S-bend exit), stage 2 to 2171: 2169.61 -> graft D=1 at 2157: **2556 (51.12 s)**, server-checked.
- optloop round 0: window 1093-1227 (U-turn 1 exit) -> graft D=1 at 1278: **2555 (51.10 s)**, server-checked.
- optloop round 0: window 1374-1492 (left U-turn) -> graft D=1 at 1489: **2554 (51.08 s)**, server-checked.
- optloop round 0: window 2385-2503 (maze entrance) -> graft D=1 at 2512: **2553 (51.06 s)**, server-checked.
- optloop window 2118-2238 (runs/loop/r1/w2118_2238_v1_mg/g_c2255_D1.txt): **2552 (51.04 s)**, server-checked.
- optloop (plan/optloop.py, rounds of optsweep windows placed on the run's turns, auto commit / push via optsweep's
  publish): round 0 2557 -> 2553, round 1 -> 2552, rounds 2-4 (incl. two-turn windows, t0 0.6) no gain: converged.
- Lag vs Teero on 2552 (his labels): -17.7 at rt 1000 -> +10 at 2540, i.e. ~34 ticks behind him after the pickup
  (61 on 2580). Remaining by section: shaft 1480-1640 ~12, 2300-2440 ~7, 1960-2060 ~5, 2100-2160 ~3, 1000-1100 ~3.
- What blocks the rest: every faster state we make (x_opt window ends +1..+3 px/t, the shaft windows 1440-1630 /
  1460-1660 with 16M iters, a beam-12000 stage 2 that reaches 1740 level with the run with E +200, the top-left
  corner with Teero's stack: no approach shot, lob + point-blank both at 2099, x_opt-polished to +11 px/t at 2130)
  is lost at the NEXT turn (rt-1800 U-turn: 10-33 behind at 1860; bottom-left turn: 10-27 behind at 2220, also when
  stage 2 tracks Teero's line, tref=teero_track.txt latpen 0.05: 13-33). Teero carries his extra speed through
  those turns; our searches (tracking our own slower line, or free) cannot. A high-speed turn solver is the missing
  piece for < 2500.
- optloop window 1210-1360 (runs/loop/r5/w1210_1360_v3_mg/g_c1396_D1.txt): **2551 (51.02 s)**, server-checked.

## Making x_ds keep Teero-type plans (Oct 9)
U-turn 1 benchmark: x_ds from 2580's rt 1082 to the rt-1320 gate (rf variants, beam 6000) only ever rebuilds 2580's
own structure (1319.97); the forced Teero structure (no approach shot, pre-fire 1126 -> 1146, point-blank 1151) is at
1316. Three biases found, each now an option:
- cellreload=1: the dedup cell only had "reload > 0". At the apex a lineage whose kick came from a lob pre-fired at
  1126 (gun free at 1151) and one whose kick was a point-blank at 1145 (gun busy to 1170) share position / velocity, so
  one replaced the other. The cell now has the reload bucket (5 ticks), grenades in flight and saved retro slots. From
  a no-approach-shot line at 1120 the search then finds the pre-fire stack by itself (1146 + 1152, 1199.13 at 1200;
  without: 1200.5-1201.7, apex point-blank).
- freefrac=F [freemin=20]: a beam share for lineages that have not fired for >= freemin ticks (grenade savers), ranked
  by the raw score (m_Lag0 / m_E0). The rollout / shadow pre-scoring replays the run's own shots, so a saver looks
  ~5 ticks worse than it is and never reached the selection (log: savers best lag 0.46 at 1107 -> 5-7 at 1111).
- shlate=N: shadow rollouts fire a run shot that the lineage's reload blocked up to N ticks late, at the run's
  explosion point (a lineage on a shifted kick schedule lost every kick in its rollouts).
Result on the benchmark: still 2580's structure from 1082 (1319.97), and from the 1120 no-approach line no variant
reaches 1316 (1322-1323 new, 1321-1331 old): the stack forms, the dip after it is not found in one search (the forced
version also needed a re-search from 1165). Next: a structure layer that generates the alternatives (skip the
approach shot, x_pfscan pre-fires, point-blank) and evaluates each with the two-stage search.
- **structsearch (plan/structsearch.py) on 2551 (runs/ss3): no gain.** Turns (speed minima with a >= 60 deg heading
  change) with an approach shot: 1275 / 1417 / 2266 / 2446 lose; 1469: x_opt +1.96 but stage 2 1601.86 vs 1597;
  top-left corner 2089 (pre-fire 2080 aim 224.5 -> 2090): stage 2 2167.12 vs gate 2167 (level); 2513: 2553. No
  pre-fire reaches the exit at 1597 / 1783 / 1965 / 2473. Pre-fires on a hook-press tick are now kept (the press is
  dropped; it used to skip 1267 / 1409 / 2250 / 2502); re-run on those turns (runs/ss4): 1275 stage 1 18 behind,
  1417 65 behind (stopped there, see below).

## Why x_ds cannot use speed: the survival check (Oct 9)
- Our 2551 vs Teero, same places: the lines are the same (rendered 1800 U-turn, S-bend, corner, hop), his kicks are
  not better (our 53 kicks average dv2 687, his ~725, same count and spacing), but we pull a braking hook (> 100 deg
  behind v, |v| >= 15) on ~476 ticks after the pickup and he on ~227 (teero/hooks timeline, per section 2-3x).
- Benchmark (x_ds from 2551's own state at rt 1570, rf variant seed 21, gate rt 1720 = where 2551 is at 1720.0):
  surv=12 (the variant's setting) 1735.9, with a free +12 px/t boost 1758.2; surv=6 1730.0 / 1733.0; **surv=0
  1719.8 (0.2 ahead) / 1716.5 (3.5 ahead)**. At rt 1584 the survival check kept 1 of 1083 selected states (and the
  run's own continuation failed it a tick later): in the shaft a state only survives with a hook sequence plus a kick,
  and the check only tries constant inputs without shots. So the search could not follow its own incumbent there
  (NOTES above: "the shaft double kick is not reproducible by x_ds from any other cut") and a faster state was
  rated doomed (the "value of speed" boost tests that cost 12-33 ticks at the vertical sections).
- x_ds survsoft=F (new): the best failing states fill the beam up to F x beam when too few pass.
- plan/xdsbench.py (new): search-quality benchmark, x_ds from a run's own states at several cuts to cut + 150, every
  variant, per setting; a search that cannot follow its incumbent ends behind it.
- Benchmark on 2551 (plan/xdsbench.py, runs/survb, 4 rf variants, gate = cut + 150, ticks behind the run, best of 4):
  cut 1151 +0.02 / +0.02, 1260 +0.02 / +0.02 (most variants NOGATE either way), 1410 -4.17 / -1.16 (the rt-1560 gate
  is mid-shaft: the -4.2 line spent the double-kick grenade at 1550 and is 11-13 behind at 1720), **1490 +6.84 /
  -0.89, 1763 +1.46 / -0.44, 1978 +5.24 / -0.55**, 2092 -0.12 / 0.00, 2282 +1.29 / +1.12 (surv as in the variants /
  surv=0); sum +10.58 / -1.87. Single variants with the check were often 17-22 behind, with surv=0 within ~1.
  survsoft=1 behaves like surv=0 where the check fails (1570: 1719.785 / boost 1716.507, 69 s instead of 109 s).
- U-turn-1 structure benchmark (2580 from rt 1082 to the rt-1320 gate) with surv=0 (+ cellreload freefrac shlate):
  still 2580's own structure (1319.97, 6 of 8 runs). The survival fix does not make one search find the pre-fire
  stack; it makes the re-searches after a plan change follow the run (shaft, 1800 U-turn, S-bend).
- plan/dschain_variants_rf_s0.txt (rf with surv=0); optsweep / optloop / structsearch take vars=FILE. optloop on
  2551 with it: runs/loop/s0.
- Speeds at the same place (sp/teerov.py on 2551, Teero's implied |v|): he is faster at the left U-turn exit (rt
  1490-1550, +4..+5), before the 1800 U-turn (1750-1760, +12..+14), before the S-bend (1910-1920, +10), at the corner
  exit (2100-2130, +6.5), at the hop exit (2270-2300, +10) and before the final maze (2330-2390, +7..+10).
- Teero's fire spots on our path (rt 1385-1660) all match ours within a few ticks except one at our rt-1479 place
  (our gun idles 1462 -> 1506; reload frees it at 1487). extrashot.py (new) 1487-1500 -> 1488-1520, s0 variants:
  the kicks at the small block (1824, 2591) kill the tee within 3 ticks; fired 1500 -> 1512: 19 behind at 1640.
- Teero time-tracking (tref=teero_track.txt ttrack=1 toff=-22.8, s0) from 1490: 45-70 behind at 1720 (he gains ~12
  ticks over 1490-1640, so the offset targets are 150-400 px ahead of anything reachable).
- **Late braking before the 1800 U-turn:** 2551 drops dir at 1744-1746 (air friction x0.95) and counter-steers at
  1750: -10.6 px/t at 61 px/t, 25 ticks before the turn (Teero is at 64 there). Forced dir=1 over 1744-1752 + x_ds
  (s0) to the rt-1846 gate: 1872.5-1874 vs 1845.97 from the run's own state. The late line stays ~1 tick ahead at the
  same speed until 1800, then hits the wall at (8910, 3326) at 1803: 2551 redirects at the turn bottom with a hook into
  the floor + its AIR JUMP at 1801 (vy +14.6 -> -11.1, vx kept); the jump came back at 1733 and is saved for it. The
  late line's search spent the air jump at 1759 (it looks as good until 1803), and the dedup cell had no jump state,
  so the saver was replaced. x_ds celljump=1 (new): air-jump availability in the cell.
- **Jump saving:** the same late-braking prefix with jumps forbidden over 1752-1795 (x_ds nojump=, new): **1844.97**
  at the rt-1846 gate (1.0 ahead of the run and of x_ds from the run's own state, 1845.97). Teero's later braking is
  faster; our search lost it by spending the air jump early. x_ds jumpfrac=F (new): a beam share for lineages that
  still have their air jump, ranked among themselves (the freefrac mechanism, shared code).
