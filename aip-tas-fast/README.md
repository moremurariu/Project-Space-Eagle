# AiP-Gores (KoG) pre-grenade: exact fast simulator + beam search

Builds on the snapshot in branch `claude/fervent-cray-0pser4` (`aip-tas-work/`, not modified).
Map: KoG AiP-Gores, sha256 `353b27cf72168cd0bb917c56eb46a83ef8d4266f310ec79b68db90f61637deaa` (not included; put it at
`aip-tas-fast/kog.map`).

## Result (metric: race tick of the grenade pickup)
- **968** (`runs/kog_pregren_968.txt`, also `kog_pregren_968b.txt`), previous best 978 → **−10 ticks**.
  Checked on DDNet's prediction code (`replay`) and on the real server code (`testrunner` `TasServer.Run`): start tick 68,
  pickup at input 1036 = race tick 968, no freeze, no double start up to the pickup.
- Like all runs from this search (977 … 968), the tee arrives at ~37 px/tick diving down-right with both jumps spent and
  freezes on the pocket floor right after the pickup. This is accepted for this metric (a separate step turns such
  arrivals into viable ones for a few ticks). `viacheck` measures the exit: from the 978 run the climb point
  (5254, 2176) is reached at race tick 1011; from the 977 … 968 runs it is not reachable.
- Format: one line per tick from spawn, `dir jump hook fire target_x target_y weapon`; the race starts at input 68.

## How (ideas taken from ddnet_physics / frametee)
- ddnet_physics is a reimplementation of the DDNet physics built for speed (precomputed per-tile flags, broad-phase
  rectangle checks, a flat copyable state). It is **not** bit-identical with DDNet (it diverges at tick 3 under hook pull
  on the 978 run; frametee's README says the physics are deliberately altered), so it is not used as the simulator.
- Its ideas applied to DDNet's own code, exactly: `ddnet.patch` adds opt-in fast paths to `CCollision` (solid table,
  2D prefix sums for "any solid in this rectangle", broad-phase skips in `MoveBox` / `IntersectLineTeleHook`, no
  move-restriction scan on maps without stoppers) and `CFast`, a 608-byte copyable single-tee stepper on DDNet's
  `CCharacterCore` for the segment before the pickup (start line / freeze / pickup checks replicate `CCharacter`).
  **5.4x faster** than the snapshot's `CTasGame`; `fastcheck` compares tick by tick with the plain prediction code:
  identical over 1.07M randomly mutated ticks.
- `ddsearch`: beam search to the pickup on `CFast` (100k–150k states are cheap now) with a time model against the best
  run (no Teero track needed), the best run's own continuation kept as incumbent, survival rollouts, and rotation-pulse
  hook aims at the exact edge of the hook's acceptance region (`rothook=2`, bisection).
- What found the big gain: **splicing** (`tools/splice.py`): a search from an early cut that stops before the left
  U-turn (`dumpat=`) gave a prefix 6 ticks ahead at race tick 690 (corridor 2 played faster); a second search from that
  prefix (`prefix=`, line-following `lp=0.05` / `0.2`) carried it to the pickup at 968. Plain re-search from cut points
  (`tools/lns.py`) only found 1-tick gains (978 → 973), because an early gain had to survive the search's own,
  less polished downstream.

## Build / check
```
bash aip-tas-fast/setup.sh     # clones DDNet at DDNET_BASE_COMMIT.txt, applies ddnet.patch, builds, runs the checks
```
Tools (in `ddnet/build`): `replay MAP INPUTS [every]`, `fastcheck MAP INPUTS [trials]`, `eacct MAP INPUTS`,
`viacheck MAP INPUTS`, `ddsearch MAP best=FILE cut=RT ...` (keys documented at the top of `src/tas/ddsearch.cpp`),
`TAS_MAP=... TAS_INPUTS=... testrunner --gtest_filter='TasServer.*'`.
Drivers (run from a folder next to `ddnet/` and `kog.map`): `tools/splice.py BEST DIR [hours= cores=]`,
`tools/lns.py BEST DIR [hours= workers= cutmin= latep=]`.
