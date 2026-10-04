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
All runs are deterministic (`HH_DET=2000`).

| Section | Waypoints | State |
|---|---|---|
| Start, the pit and the pool | 0-53 | Passes in full runs. |
| On to the swing course | 54-122 | Clean in one run, 104 s. |
| Swing course | 122-148 | 5 of 8 start positions get through, and one full run got through. |
| Corridor, column drop, long shaft, drag, catch | 144-186 | Passes from the corridor. |
| On to the last block | 186-245 | From the corridor, one run went all the way to the finish drop with no restart. |
| Finish drop | 245-255 | Both tees finished in 2 of 2 runs. |

One full run from the spawn reached waypoint 181 in 294 s before a restart. The fix for that spot is in.

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
- **Video:** `DDNET_READ_BACK_BUFFER=1` makes the client's video recorder work headless (Xvfb + Mesa).
