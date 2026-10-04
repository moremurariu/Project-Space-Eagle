# Fastest pre-grenade run: grenade pickup at race tick 992 (19.84 s)

- `pregren_992.txt`: inputs from spawn up to the grenade pickup (1065 lines).
- `pregren_992_plus35_viability.txt`: the same run plus 35 ticks of post-pickup movement (no shots) into the climb,
  reaching Teero's k1005 point at race tick 1027 without freeze. This is only the viability check, not optimised.

Format: one line per game tick, starting at spawn: `dir jump hook fire target_x target_y weapon`
(dir -1/0/1, jump/hook/fire 0/1, target = aim vector relative to the tee, weapon -1 = keep current).
The race starts at input 73 (server start tick 73), so input N is race tick N - 73; the pickup happens at input 1065.

Checked on real server code (`TasReplay.Run`, TAS_TRACE=1): start tick 73, identical positions to the simulator,
no frozen ticks, no double start. Teero's pickup (from the video timing) is at about race tick 980, so this run is
~12 ticks behind him at the pickup.
