# KoG AiP-Gores: pre-grenade runs (spawn -> grenade pickup)

Format: one line per tick from spawn: `dir jump hook fire target_x target_y weapon`. Spawn = first spawn point in map
order (tile 4,6 -> 144,208). The race starts at input 68, so input N = race tick N-68. KoG map version
(sha256 353b27cf72168cd0bb917c56eb46a83ef8d4266f310ec79b68db90f61637deaa). Solo only.

| file | pickup race tick | after the pickup | from |
|---|---|---|---|
| `kog_pregren_best.txt` = `kog_pregren_962.txt` | **962** | dive at ~37 px/t, both jumps spent, freezes on the pocket floor | faraday (`ddsearch` + splice) |
| `kog_pregren_963.txt` ... `kog_pregren_968.txt`, `968b`, `975` | 963-975 | same kind of dive | faraday |
| `kog_pregren_978.txt` | 978 | viable (the old full runs continue from it) | trunk |
| `kog_pregren_979.txt` (+ `_plus45_viability.txt`) | 979 | viable | trunk |

All of them replay identically on the real server code (`tas-work/srvfin.sh FILE`). The previous best full run
(`tas-work/kog_full_2613.txt`) picks up at race tick 971: it is the 968 dive re-fitted into a braked, viable pickup
(`tas-work/physlab5/FINDINGS.txt`, section 4).

**The 962 dive is now part of the best full runs**: `tas-work/kog_full_2608.txt` is 962 up to race tick 930, a corner
graze + braked pickup at 966 found by `x_graft`, then the 2613 run's inputs (see ../README.md, "The 962 graft");
`kog_full_2607.txt` (the current best) is that graft polished with `x_ds`.

The faraday runs were searched for the earliest pickup only (no viability constraint), which keeps the search simple;
turning such a dive into a viable pickup costs a few ticks (968 -> 971). The 962 dive is the 968 dive's line, ahead by
5.5-5.9 ticks from about race tick 930 on (measured along the line; within ~2 px of it).
