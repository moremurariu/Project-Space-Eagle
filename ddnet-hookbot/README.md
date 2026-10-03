# DDNet hookbot: Stronghold work

The bot's DDNet work. It sits on top of the handoff commit `24c453e2c` ("messy catch all commit number 2") in your DDNet fork. The goal is two live hookbot brains (`CHookBotBrain`, one per tee) beating Stronghold in the sim (`SimMapBots.Hammerhit`).

## What's here
- **`work-since-24c453e2c.patch`:** everything since 24c453e2c, as one patch. That covers the handoff's uncommitted work, its untracked files, and the bot work since. It applies cleanly to 24c453e2c.
- **`files/`:** the same changed and new files at their DDNet paths, so you can read them without applying anything.
- **`runs/`:** the scripts I run the sim with. They expect the fork at `/home/user/ddnet` with `build-sim` built. Adjust the paths.
  - `at.sh <tag> <route index> <x0> <x1> <y> [seconds] [seed]` runs from any spot.
  - `corr.sh <seed>...` runs from the corridor after the shaft (waypoint 144). `SECS` sets the length.
  - `swbatch3.sh` runs the swing course from several start positions.

The bot's behaviour, the reasons for it, its failure cases, and every test and env var are in `docs/HOOKBOT.md` (in the patch and in `files/docs/`).

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
These are separate runs from fixed start spots, not one run from the start.

| Section | Waypoints | State |
|---|---|---|
| Start to the swing course | 0-122 | Done in earlier sessions. Not re-run after this session's changes. |
| Swing course, shaft climb, corridor entry | 122-145 | 5 of 8 start positions get through. |
| Corridor's end: freeze column drop and long shaft | 145-164 | Works. 7.5 s against rank 1's 6. |
| Drag under the floor, then the catch out of the freeze column | 174-176 | Works in one run. In the latest run the swing down to the catch spot missed, and the run restarted. |
| Freeze pools, then the launch and climb up the unhookable room | 176-193 | Works live from the ledge. |
| Chamber: fling across the freeze block, catches up and across | 193-205 | Works live from the chamber floor. |
| Drop down the shaft after the striped room, climb up the shaft beside it | 205-213 | **Open.** From a calm start at the foot the climb works. The launch into it doesn't yet. |
| Rest of the map | 213-254 | Not started. Rank 1 has about 10 more freeze moves there. |

New team moves this session, all in `src/game/hookbot.cpp` and all documented in `docs/HOOKBOT.md`:
- `PlanColumnDrop`
- walking catches in `PlanFall` (re-hook, hammer through corners, jump onto the block)
- `FindCatchSpot` (waiting where the partner's fall can be caught)
- `PlanLaunch`
- `PlanFling`

Two changes affect every section:
- A tee lying in freeze counts only as far as its rescuer's spot (`HH_OLDRESCUEGAIN=1` restores the old rule).
- Team moves aren't retried at a spot where they already failed in one more case.

A new `SimMapBots.MapRegion` test (`MR_RECT`) prints the map as text.
