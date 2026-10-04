# DDNet hookbot: Stronghold work

This is the bot's DDNet work. It sits on top of commit `24c453e2c` ("messy catch all commit number 2") in your DDNet fork, the commit the work was handed over at.

The goal is two live hookbot brains (`CHookBotBrain`, one per tee) beating Stronghold in the sim (`SimMapBots.Hammerhit`).

## What's here
- **`work-since-24c453e2c.patch`:** everything since `24c453e2c`, as one patch. It covers:
  - the work that was uncommitted at that commit;
  - its untracked files;
  - the bot work since then.

  It applies cleanly to `24c453e2c` (checked).
- **`files/`:** the same changed and new files at their DDNet paths, so you can read them without applying anything.
- **`runs/`:** the scripts I run the sim with. They expect the fork at `/home/user/ddnet` with `build-sim` built; adjust the paths for yours.
  - `fulld.sh <tag> [seconds] [seed]`: a full run from the spawn, recorded to a demo (`HH_DEMO`, `SIM_DEMO_DIR`).
  - `at.sh <tag> <route index> <x0> <x1> <y> [seconds] [seed]`: a run from any spot, both tees on one row.
  - `at2.sh`: the same with the tees at different heights.
  - `at3.sh`: the same with tee 1 frozen.
  - `sect.sh`: one spot, several seeds.
  - `corr.sh`: from the corridor after the shaft (waypoint 144).
  - `swbatch3.sh`: the swing course from 8 start positions.
  - `render.sh <demo name>`: a recorded run to video (headless, Xvfb).
  - `seeds.sh <tag> <seed>...`: full runs at several timing seeds, one after another, with each one's result.
  - `attempts.sh <log>`: for a run's log, each attempt's furthest waypoint and how it ended.
  - `sec.sh <tag> <route index> <x0> <x1> <y> <through wp> <seconds> <seed>...`: one section from 6 starts per seed (x offsets -0.3, 0, +0.3, each swapped): how many get through.
  - `sw.sh <tag> <seed>...`: the swing course from 8 starts.
  - `stopsim.sh`: stops the scripts and the sim.

`docs/HOOKBOT.md` (in the patch and in `files/docs/`) covers the bot's behaviour, the reasons for it, its failure cases, and every test and env var.

## Apply
```sh
cd <your ddnet checkout>
git stash -u                      # if you have local changes
git checkout 24c453e2c -b hookbot-stronghold
git apply <this dir>/work-since-24c453e2c.patch
git add -A && git commit -m "hookbot: Stronghold work"
cd build-sim && ninja -j2 testrunner
```

## Where Stronghold stands
Both live brains take both tees from the spawn to the finish at three of four timing seeds, each on the first attempt (`runs/seeds.sh`, 900 s of game each). The server marks both races finished. Brain time: 120-151 s per run.

| Seed (`HH_DET`) | Result |
|---|---|
| 1800 | finished, 417.5 s, no restart |
| 2200 | finished, 401.5 s, no restart |
| 2000 | finished, 409.3 s, no restart |
| 2600 | no finish (to waypoint 142, then stuck three times in the corridor after the bottom room) |

Before this round, none of these four finished. The 429 s finish at seed 2000 was one deterministic path; the bot has changed since, and that seed now takes another one.

- **Still weak:**
  - the corridor after the bottom room (waypoints 60-68): freeze above and below, one hookable block; the solo swings fall short of the ledge;
  - the drop into the unhookable room (waypoint 228): 10 of 18 section starts get through.
- **Section checks** (`runs/sec.sh`): the gap before the bottom corridor (waypoint 174) gets through 18 of 18 starts (6 offsets × 3 seeds).

The video (`runs/render.sh stronghold_f7`) is rendered from the recorded demo (`HH_DEMO`, `SIM_DEMO_DIR`). `docs/HOOKBOT.md` has a section on making one.

## Robustness round (latest)
All in `src/game/hookbot.cpp`. Details, reasons and switches are in `docs/HOOKBOT.md`, section "Robustness across timing seeds".

- **Nudge:** both standing where every search and retry came up empty: one steps a few px, to the floor's end, or over a gap. Then everything is searched again from there.
- **Fall-catch from the floor's edge:** the catcher can walk to the end of its floor first. At the gap before the bottom corridor that's the difference between found in 30k ticks and nothing in 1.8M.
- **Regroup:** apart and stuck: a joint move that brings the one ahead back to the one behind.
- **Live fly:** between bots, it doesn't start under freeze (within 6 tiles); the planned climbs go round.
- **Frozen-pair search:** every 5 ticks while the free one is falling into freeze.
- **Drop:** prefers a candidate after which a joint move gets us out of the freeze, and plays that move as part of the plan.
- **Route:** skips ahead when we've dropped past the next waypoints.
- **Fail spot:** a tee standing on the other's head jitters; that no longer reruns every search every 2 ticks.
- **Earlier in this round:**
  - a climb's coast direction avoids freeze;
  - climbs chain on at once;
  - climb ends are checked without the partner in the sim;
  - reach hook (jump over and hook a frozen partner out);
  - the safe way down when a swing search finds nothing in the air;
  - the apart searches after 2 s idle.

New scripts in `runs/`:
- `sec.sh`: one section from 6 starts per seed;
- `sw.sh`: the swing course;
- `attempts.sh`: each attempt's furthest waypoint;
- `stopsim.sh`.

## Compute
**114 s of brain time for the 429 s run, down from 1068 s.** The game is exactly the same as before: same plans, same messages, tick for tick.

- **Each brain searching for itself:** 215 s.
- **Sharing:** the two brains share their team searches, which only works with both bots in one process.
- **Profile:** `HH_PROF=1` prints where the time goes, and `HH_TEAMCALLS=1` prints every team search.

The main changes (details in `docs/HOOKBOT.md`, section Compute):
- **DDNet core, performance only:** the results are bit-identical, and DDNet's own tests pass.
  - `CCharacterCore` loops only over the players in use, when the world keeps that list (the bot's sim does; the game doesn't).
  - `Move` picks its collision partners once per move.
  - `MoveBox` skips its box tests where nothing solid is in reach, and otherwise reuses the last test while the corners stay in the same tiles.
  - `IntersectLine` checks each tile once.
  - Maps without stoppers or kill tiles skip those checks.
  - The build uses `-fno-semantic-interposition`, so GCC can inline again.
- **The bot:**
  - goal fields are cached and built with a bucket queue;
  - open-air reachability is answered from regions labelled once;
  - the reachability checks read a per-map tile table;
  - the joint search's nodes no longer copy their history;
  - the sim reuses the last step's freeze check.

About 70% of the team searches' ticks still go into searches that find nothing. Cutting them changes some plan, and with the bot this fragile the run then takes another path that usually fails, as other seeds do. Robustness comes first.

## This session's main changes
All in `src/game/hookbot.cpp` and documented in `docs/HOOKBOT.md`.

- **Finish drop (`PlanFinishDrop`):** rank 1's way through the last freeze layers. One tee falls past the last block. The other holds it under the block until it thaws. Then the holder walks in, and the thawed one drags it into the 3-wide gap.
- **Done at the finish:** a tee that crossed a finish tile does nothing more, and the other goes on alone.
- **Solo swing chains:**
  - a safe plan keeps its safe way down;
  - a plan left in the air keeps playing while the next search runs;
  - a landing only counts if the tee stays on the floor;
  - the search keeps clear of the flying partner;
  - a solo search falls back to the shared goal.
- **Climb hand-off:** both tees are at least 40 px apart when a climb hands them to their solo swings.
- **Live fly:** it doesn't start where something hookable is just above.
- **Start of the map:** the drop into the first shaft is allowed (the bots stood at its edge before).
- **Catch spots:** only ones reachable on foot or by swinging.
- **Retry:** a stuck pair retries the fall-catch with a bigger budget.
- **Walking rule:** no long drops while the partner is far behind.
- **Sim freeze check:** the sim checks freeze along a tee's path exactly as the server does: a sample every pixel, truncated to tiles. Before, it sampled differently and rounded, so a plan could brush a freeze corner that the server counted as touched.
- **Video:** `DDNET_READ_BACK_BUFFER=1` makes the client's video recorder work headless (Xvfb + Mesa). `DDNET_DEMO_RENDER_WAIT_MS` with `player_skin claude` loads the skin before playback starts.
