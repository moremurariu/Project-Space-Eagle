# KoG AiP-Gores: pre-grenade run, grenade pickup at race tick 979 (19.58 s)

- `kog_pregren_979.txt`: inputs from spawn to the grenade pickup (1047 lines), KoG map version
  (sha256 353b27cf72168cd0bb917c56eb46a83ef8d4266f310ec79b68db90f61637deaa).
- `kog_pregren_979_plus45_viability.txt`: the same run plus 45 ticks of movement (no shots) into the climb after the
  pickup (reaches Teero's k1005 point at race tick 1024 without freeze). Viability check only, not optimised.

Format: one line per tick from spawn: `dir jump hook fire target_x target_y weapon`. Spawn = first spawn point in map
order (tile 4,6 -> 144,208). The race starts at input 68, so input N = race tick N-68; the pickup is at input 1047.
Solo only: no other tees on the map (they can be hooked / collided with).

Checked on real server code (TasReplay, other debug tees kept as spectators): positions and velocities identical to the
simulator on every tick, no freeze, no double start. Teero picks the grenade up at about race tick 980-981.
