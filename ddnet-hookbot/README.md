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
Both live brains take both tees from the spawn to the finish in **429.06 s with no restart**, at timing seed `HH_DET=2000`. The server marks both races finished.

- **Not robust yet:** with seeds 1800, 2200 and 2600 (`runs/seeds.sh`), no run finished in 15 min of game.
  - The first attempts fail at waypoints 127 (the swing course), 63 and 98.
  - Later attempts fail in the swing course, at the pool (52-54), just after it (63-64), and at the start.
  - So the finish at seed 2000 is one deterministic path, not a bot that gets through reliably.
- **Section by section,** from separate starts at seed 2000: everything passes. The swing course from 8 separate start positions gets through 5 times.

The video (`runs/render.sh stronghold_f7`) is rendered from the recorded demo (`HH_DEMO`, `SIM_DEMO_DIR`). `docs/HOOKBOT.md` has a section on making one.

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
