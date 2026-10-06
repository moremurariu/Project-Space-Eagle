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
