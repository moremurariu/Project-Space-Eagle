# KoG AiP-Gores: pre-grenade runs

## Current best: grenade pickup at race tick 976 (`kog_pregren_976.txt`, 1044 inputs)
Server-identical (start 68, pickup at input 1044), no freeze; climbs to the upper shaft at race tick 1007. Found by
anchored seg from rt 690 with a strong line penalty against the current best (latpen=0.1 latdz=8, ghoste=0.06);
deviates from the 977 run at rt ~779.

## Previous: grenade pickup at race tick 977 (`kog_pregren_977.txt`, 1045 inputs)
Same format as below (spawn 144,208, race starts at input 68). Server-identical over all 1045 ticks (TasReplay on
upstream DDNet 470eead4a, see ../setup_upstream.sh), no freeze, no double start. Post-pickup viability: the tee still
has its air jump at the pickup and a search from the pickup reaches the upper shaft (x 5150-5420, y 1900-2200) at
race tick 1006 without freeze (the 978 run: 1009). Pickups that dive into the pocket without braking (no air jump,
|v| ~37) are rejected: the tee freezes 1-2 ticks after taking the grenade.

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
