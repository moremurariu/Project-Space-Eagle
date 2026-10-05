# AiP-Gores (KoG) pre-grenade: exact fast simulator + beam search (work in progress)

Builds on the snapshot in branch `claude/fervent-cray-0pser4` (`aip-tas-work/`, not modified).

## Status
- **No valid improvement yet: the best usable run is still the previous `kog_pregren_978` (pickup at race tick 978).**
- `runs/kog_pregren_977.txt` ... `975.txt` and `kog_pregren_968_INVALID_dies_after_pickup.txt` reach the grenade earlier
  (server-checked up to the pickup) but are **invalid**: they dive through the pickup at ~37 px/tick with both jumps spent
  and freeze on the floor of the grenade pocket right after. The search's survival check counted the pickup as success.
- Post-pickup check (small search from the pickup to the climb point 5254,2176 without freezing): 978 reaches it at race
  tick 1011, 979 at 1014; none of the 977-968 runs reaches it.
- `runs/best.txt` = the 978 run.

## What works
- `ddnet.patch` (against upstream DDNet, `DDNET_BASE_COMMIT.txt`): exact fast collision paths and `CFast`, a copyable
  single-tee stepper on DDNet's own `CCharacterCore` (ideas from ddnet_physics: precomputed tile tables, broad-phase
  checks). ~5.4x faster than the snapshot's `CTasGame`, identical over 1.07M fuzzed ticks (`fastcheck`).
- `replay`, `eacct`, `ddsearch` (beam search, incumbent, survival rollouts, rotation-pulse aims), `testrunner`
  `TasServer.Run` (server-code replay: start tick, pickup race tick, freeze, double start), `tools/lns.py`.
- ddnet_physics itself is not bit-identical with DDNet (diverges at tick 3 under hook pull; frametee states the physics
  are altered on purpose), so it is not used as the simulator.

## Known issues
- The `ddsearch` source in the patch predates the viability fix: pickups must also be checked for a usable exit.
