# Teero's pre-grenade run, recovered per game tick from his video

Source: the user's upload `Aip-GoresNEW_c.mp4` (2560x1440, 60 fps, 23.9 s; not in git). The camera follows grey Teero
(solo; the other tees are ghosts) and the bottom-right debug HUD shows his position (tiles), speed (tiles/s, x scaled by
the velocity ramp) and aim angle (degrees, y down) with 2 decimals.

- `hudocr.py`, `hudtrain.py`, `hudall.py`: HUD reader (colour threshold, column split, digit templates from 3 frames,
  `hud_templates.pkl`; the decimal point is restored from the fixed 2 decimals). `hudall.py TOOLS VIDEO TEMPLATES OUT`.
- `ticks.py HUD.csv OUT.csv`: the HUD values are linear interpolations between game ticks and frame f shows game time
  exactly f * 50/60 ticks (phase fitted: residual 0.03 px rms); least squares gives his state at every tick, and the tick
  positions come out on whole pixels (mean |frac| 0.09 px = HUD rounding), as DDNet quantises them.
- `teero_ticks.csv`: per tick: video tick, our tick (= video tick - 97: his first movement after spawn equals our
  tick 1 exactly), race tick (= video tick - 162: he crosses the start line on video tick 162), position (whole px),
  internal vx (HUD speed / ramp), vy, displayed vx, aim angle. Positions are exact; velocities are approximate
  (the displayed speed sometimes jumps by ~1 tile/s for 2-3 frames without any change in the motion).

Findings (vs our 962, nearest point on its path): level to race tick 50; he gains 2.7 ticks in corridor 1 (rt 50-260)
and 1.7 at the turn top (rt 260-320); we gain ~7 in corridor 2 (rt 510-650). At rt 44 the two are 3 px apart with the
same y and vy, yet he later moves 26 px/tick where we move 25 (rt 53, 57-59) and 27 where we move 26 (rt 71-75): with
whole-pixel steps that needs ~+0.5 vx, more than a lossless rotation can give from our energy, so he carries ~15-30
more energy hidden inside the pixel bands. Visible sources: a faster start crossing (22 px/tick before the line vs our
21) and his air jump at rt 6 (nearer the apex) instead of our rt 4. A tracker following his exact positions from our
rt-44 state falls 40+ px behind by rt 120; from spawn it fails within 3 ticks. His HUD also shows speed gains above
15 px/t without a jump, which DDNet's hook rule forbids (frametee's physics are altered on purpose), so part of his
corridor-1 lead may not be reproducible under real DDNet physics.
