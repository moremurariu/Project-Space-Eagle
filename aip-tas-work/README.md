# AiP-Gores TAS: beat Teero (merged toolkit)

## Goal and rules
Make a tool-assisted run of the KoG version of **AiP-Gores** that beats Teero's **50.72 s (2536 ticks)**.
- Solo run. Grenade is the only weapon to pick up.
- **No double start**: you may not cross the start line again after the race starts. The fast steppers kill such
  states; CTasGame marks them (`m_StartTick = -2`).
- The run must work on a real server. The simulators are DDNet's own prediction code (plus exact fast paths); every
  result is checked on real server code with `TasReplay` (`tas-work/srvfin.sh FILE`).
- Any means are allowed, Teero's own inputs included (user, Oct 7: the goal is the fastest run without a double
  start, by any means). An earlier README said not to start from his inputs; that was an agent's misreading. We have
  no exact inputs of his: `tas-work/teero/` holds what was extracted from his video
  (https://www.youtube.com/watch?v=eHJJNU-hQoU): direction keys, jump, aim, fire per 60-fps frame, no hook.

## Status (Oct 7)
- **Best full run: 2558 race ticks (51.16 s)**, `tas-work/kog_full_best.txt` (= `kog_full_2558.txt`), server-checked
  (TasReplay: start tick 68, finish tick 2626, no freeze; TasServer: no death, no freeze, no double start). Grenade
  pickup at race tick 966. 36 ticks on 2606 from the section planner (`tas-work/plan/xplan.py`, overlapping stages)
  + `x_graft`, automated by `tas-work/plan/xsweep.py`: right U-turn 1 (2605), the final maze (2604), U-turn 1 again
  (2603, 2602), the final maze again (2600), 1825-2000 with the low-loss hook aims and braking cost in `x_ds`
  (`rotfar=1 brakew=0.3`, 2599), right after the pickup's double kick (2598, 2597), U-turn 1 (2596) and 1270-1445
  (2592, a D=4 graft through the left U-turn) 2000-2175 (2591, autosweep.py), 2350-2525 (2590) and 1700-1875 (2589, with the Teero hook prior; 2588, seeds 31-34), all with those options; then the full post-pickup plan (xplan from rt 1007 to 2575) grafted through the left U-turn (2584), a second full plan grafted before it (2583) a plan from rt 1830 grafted in the final maze (2582), then autosweep with long sections and `xsweep allcuts=1` (2581, 1007-1450; 2580, 1640-1830; 2579, 2100-2575; 2576, 1640-1830 again, a D=3 graft at rt 1860; 2575, 2100-2575; 2574, 1007-1450); then `x_opt` + `optsweep.py` (borrowed from the claude/sweet-maxwell-3t11yw branch: window plan optimizer, x_ds re-search through the next turn, graft) on the left U-turn (2570); combined with the claude/sweet-maxwell-3t11yw branch's 2567 (`kog_full_2567_maxwell.txt`: its U-turn 1 rebuilt with Teero's shot plan, shaft top, S-bend, corner and final-maze gains) by two grafts: its run to rt 1400, our left U-turn to rt 1584, its run after that (2563); optsweep on it, rt-1800 U-turn window (2562); combined again with sweet-maxwell's 2562 (`kog_full_2562_maxwell.txt`: its dip and shaft-top gains) the same way, grafts D=1 at 1423 and D=4 at 1580 (2558).
  See NOTES "Section planner v2", "Braking cost and far low-loss hooks", "Teero's hooks from his video".
- 2606 (52.12 s): first full run with **Teero's shaft double kick** (see "The shaft double kick" below).
- 2607 (52.14 s): faraday's 962 pre-grenade dive grafted onto the 2613 run -> 2608 (below), then 20 minutes of
  `x_ds` incumbent LNS -> 2607.
- Previous best: 2613 (52.26 s, cray), pickup at 971.
- **Best pre-grenade run: pickup at race tick 962** (a dive, searched for the earliest pickup only),
  `pre_grenade_kog/kog_pregren_best.txt`, server-checked.
- Teero: pickup ~980, finish 2536. We are 22 ticks behind overall: ~16 ahead at the pickup, ~38 behind after it.

### The 962 graft (2613 -> 2608)
The 962 dive follows the same line as the 968 dive the 2613 run was built on, ~5.6-5.9 ticks earlier (measured along
the line, rt 930-950). The 2613 run turns that dive into a viable pickup by grazing the corner of the last solid block
(landing for one tick at x <= 5293, y in [2285, 2290) refills the double jump without losing the downward speed),
hook-braking on the block face and spending the double jump at the pickup to brake. `x_graft` (new) steers the 962
prefix from a cut into that same sequence 5 ticks earlier: a beam ranked by the distance to the 2613 run's state 5
ticks ahead; every step the closest states are continued with the 2613 run's own remaining inputs, and a continuation
that finishes counts. From cut rt 930 the graft grazes the corner at rt 956 (jumps refilled at 957), hook-brakes on
the block face (959-965) and spends the double jump at the pickup (rt 966). Its velocity is still ~0.1 px/t off the
2613 run's until rt 969, where the air-control clamp sets vx to -5.00 in both; from there the state (position,
velocity, hook, jumps) is identical to the 2613 run's 5 ticks later, all the way to the finish: 2608
(beam 20000, 4 threads, 23 s; deterministic: this command regenerates `tas-work/kog_full_2608.txt` byte for byte.
`kog_full_2608_b.txt` is the first graft found, before the tie-break fix, and the start of the 2607 polish).
```
x_graft AiP-Gores.map prefix=../pre_grenade_kog/kog_pregren_962.txt cut=930 run=kog_full_2613.txt D=5 \
        horizon=45 beam=20000 threads=4 test=300 out=graft.txt
```
A 6-tick graft is out of reach: the 962 line is ~5.7, not 6, ticks ahead, and in the air the tee can lose speed but
not gain it.

### The shaft double kick (2607 -> 2606)
Teero fires a lob ~25 ticks before reaching the single 2x2 block at the bottom of the shaft (tile 102,104) and a
point-blank down at the block, so both grenades explode under him in consecutive steps (vy ~ -45). davinci's `dk2` line
had this (lob at 1576, double kick at 1601-1602 on its clock) and climbs the shaft like Teero (block -> top: Teero 29
ticks, dk2 30, our previous runs 37), +12.4 ticks ahead at the top, but lost all of it by rt ~1960. Because the
post-grenade part of 2607 is the 2614 run's shifted 5 ticks earlier (the 962 graft), dk2's inputs splice onto 2607 with
a 5-tick offset (`tas-work/runs/shaft/sdk3_doublekick_2607.txt`: 2607 up to rt 1568, dk2 after, 2607's ending; ties
2607). `x_ds` incumbent LNS restricted to cuts 1590-1950 (where the double kick still matters) then found 2606 from cut
1777. Where the lead goes (eaudit): after a Teero-like top turn the line's descent kick at (4189, 2716) is badly
aligned (cos 0.53 with the motion) and the hook brakes -352 over rt 1640-1689 (2607: -109, with a cos 0.83 kick lower
down), so it reaches the channel at |v| 30 instead of 33 and runs the channel ~3 px/t slower. That is where the
remaining ~10 ticks are.

## Where the pieces came from
This folder merges four branches (all on top of the same DDNet commit; nothing else in this repository is involved):

| branch | contribution kept here |
|---|---|
| `claude/compassionate-davinci-5q4n6j-postnade` | base of this folder: upstream build (`ddnet-upstream-ds.patch`), CFastG/`segf`, `x_ds` (deferred-shot search), `x_tig` (Teero / run tracker), `tigloop.py`, `dschain.py`/`dsloop.py`, `x_dbl`/`x_lob`/`x_win`/`x_pert`, best run 2614 |
| `claude/fervent-cray-0pser4` | `seg` options `survx`, `hookw`, `eres`, `rjrun` (exact rejoin), `follow`, `hookmaxv`/`hookhold`, `survrisk`; tools `eaudit`, `lobscan`, `rejoin`, `perturb`, `fgcheck`; best run **2613** (late-cut `survx` finish search on 2614) |
| `claude/fervent-cannon-0rrvvi` | exact collision fast paths (prefix-sum broad phase, TileExists cache, `ms_TasSolo`) and the allocation-free `CTasGame::CopyFrom`; anchored `seg` (`anchor=` = `inc=` here) |
| `claude/wonderful-faraday-klv0ms` (`aip-tas-fast/`) | CFast (608-byte pre-grenade stepper), its solid-table collision fast paths, `ddsearch`, `fastcheck`/`replay`/`viacheck`/`brute`, the splice / guided-LNS drivers (`fast/tools/`), pre-grenade runs 962-968 |
| new in this merge | one patch with both fast-path sets (fastest for every stepper), `simbench` (speed + exactness of every stepper), `x_graft` (graft a faster prefix onto a complete run) and the 2608 run, `tas-work/bench/` (benchmark drivers) |

The merge of davinci and cray had three trivial conflicts (both appended to NOTES.md; two different 2652 runs, both
kept: `kog_full_2652.txt`, `kog_full_2652_cray.txt`; `kog_full_best.txt` = cray's 2613). All other files merged
cleanly. `kog_full_best.txt` is now the 2608 run. History: each branch's own log, and `tas-work/NOTES.md` (cray's parallel section is marked).

## Which tool for what (measured, see BENCHMARKS.md)
| task | use | evidence |
|---|---|---|
| simulator | the merged `ddnet-upstream.patch`; CFast before the pickup, CFastG after it, CTasGame for prefixes and checks | fastest for every stepper, exact (BENCHMARKS 1) |
| pre-grenade improvement | `ddsearch` LNS / splice / guided LNS (`fast/tools/lns.py`, `splice.py`, `glns.py`, `gsplice2.py`) | only finder to improve 978 in the same budget (978 -> 976); found 978 -> 962 in its branch (BENCHMARKS 3) |
| post-grenade improvement | `x_ds` incumbent LNS / chains (`tas-work/dschain.py`, `dsloop.py`, or `tas-work/bench/lnsbench.py xds-dav`) | 2677 -> 2667 in 10 min where `segf` LNS found nothing (BENCHMARKS 3) |
| carrying a new lead through a polished run | `x_tig` tracker + forced-lineage `dschain` (`tigloop.py`, needs `teero_track.txt`) | how 2639 -> 2614 was found (NOTES "Tracker loop results") |
| last ticks of a run | late-cut `segf` finish searches with `survx=300 jitter=1 seed=N` (`inc=`) | 2614 -> 2613 (NOTES, cray section) |
| merging a faster prefix | `x_graft` (new), or `segf rjrun=` for exact merges | 962 + 2613 -> 2608 (above); `rjrun` did not find it |
| re-planning a section of a polished run | `tas-work/plan/xsweep.py` (section planner `xplan.py` with overlapping stages + `x_graft D=1` + server check) | 2606 -> 2602 in four 1-tick steps where incumbent LNS was saturated (NOTES "Section planner v2") |
| single windows / experiments | merged `segf` (or `seg`); `x_win`, `x_dbl`, `x_lob`, `rdv`, `lobscan` for grenade setups | S1: `segf` 62 s vs 67-88 s for the CTasGame `seg` builds (BENCHMARKS 2) |
| checking | `simbench ... fuzz`, `fastcheck`, `fgcheck`, `tas-work/srvfin.sh`, `srvcheck.sh` | |

## Simulator
All steppers are DDNet's own code; the fast paths only skip work whose result is known, and are checked:
`simbench MAP RUN fuzz` compares every stepper with plain DDNet prediction code from random inputs (0 mismatches over
~187,000 ticks per tree), and every accepted run is replayed on server code.
- `CTasGame` (`sim.h`): the prediction world. Used by `seg`, `pre`, `lab`, prefix replays, and as the reference.
- `CFastG` / `CTasFast` (`fastg.h`, `tasfast.h`): single-tee stepper with grenades. `segf`, `x_*`, `rdv`, `eaudit`.
- `CFast` (`fast.h`): single-tee stepper without weapons (pre-grenade only). `ddsearch` and its drivers.
- `TAS_NOFAST=1` turns every collision fast path off (plain DDNet code), `TAS_NOFASTCOPY=1` the CTasGame copy shortcut.

Merged copy+step cost: CTasGame 0.74-0.81 us, CFastG 0.60-0.65 us, CFast ~0.55-0.6 us; plain DDNet code ~3.3 us.

## Setup
```
bash setup.sh          # from aip-tas-work/; details in HOW_TO_USE.md
```

## Layout
- `ddnet-upstream.patch`, `DDNET_BASE_COMMIT.txt`, `setup.sh`: the build (upstream DDNet + patch + `ddnet/src/tas`).
- `ddnet/src/tas/`: all TAS sources (the tools' keys are documented at the top of each `.cpp`).
- `tas-work/`: runs, drivers, analysis scripts, research notes (`NOTES.md`), physlab agents, `bench/` (benchmarks).
- `fast/`: the CFast / `ddsearch` drivers and runs from `aip-tas-fast` (`fast/README.md`).
- `pre_grenade_kog/`: pre-grenade runs (`README.md` there lists them).
- `BENCHMARKS.md`: simulator and search benchmarks across the branches.

## Next steps
- **Section planner (NOTES "Section planner v2"):** `xsweep.py` re-plans sections of the best run with overlapping
  stages and grafts lines that are the run's own line ~1-1.5 ticks earlier back onto it (D=1). It gained 4 ticks
  (U-turn 1 three times, the final maze once); sections 2000-2525 reproduce the run. Re-run it with new seeds
  (`rounds=`) after every gain.
- **Bigger leads that do not graft:** the planner finds +4.4 in the left U-turn and +4.0 in 1270-1445, both on the run's
  own line, but each spends a grenade at another time, and the run's next technique (the 27-tick pre-fire onto the
  left U-turn wall; the shaft lob + double kick) then has no reload slot. Turning them into finishes needs the next
  technique rebuilt in the new shot rhythm (Teero fires at every reload, 25-28 ticks apart; we leave 31-41-tick gaps).
  No search found the shaft double kick 4 ticks earlier yet.
- (before the planner) Teero's remaining techniques (U-turn 1 far-wall pre-fire + apex double, a better descent after
  the shaft double kick, the left U-turn) change the line and the shot schedule together; free searches far before a
  gate are 15-40 ticks weaker than the polished runs, and incumbent-kept searches fall back onto the incumbent.
- Plain incumbent LNS is saturated on the current runs (2606: 90 x_ds jobs without gain; 962: 47 ddsearch jobs).
- Where the remaining ~70 ticks are (lag vs Teero): shaft + top turn ~6 left after the double kick, right U-turn 1
  ~13, left U-turn ~8, 2050-2450 ~17 (spread), final maze ~9-10 (plus a ~5-tick artifact of his track's end).
- Pre-grenade: Teero's newer run is +4.5 ahead at the right U-turn; following his line gains +2.6 by rt 300 but no
  search reproduces his turn, and the line does not graft back onto ours.
- `seg` speed: the simulator is no longer its bottleneck; survival rollouts (~50%) and action generation (~40%) are
  (BENCHMARKS 2).
