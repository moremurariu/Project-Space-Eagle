# Teero's hooks, extracted from his video

Video: https://www.youtube.com/watch?v=eHJJNU-hQoU ("Aip-Gores TAS in 50.720", 2560x1440, 60 fps; not in git). The
camera is centered on Teero; Tater appears as a solo-mode ghost and never interacts.

- `vidreg.py`: camera registration. Scale 1.75 screen px per world unit; registering frames to `../../map.txt` by
  FFT cross-correlation of the terrain shows that video time t shows the track's (`../../teero_track.txt`) race tick
  `(t - 1.383) * 50 - 2.6` (median error 7 px over 25 frames; with -2.6 removed: 90 px).
- `hookdet.py VIDEO T0 T1 OUT`: per frame, the hook chain from the screen center: for every 0.5 deg direction the
  first solid tile (a grabbed hook ends there, <= 380 units) and the fraction of chain-like (dark, neutral gray)
  pixels on the segment; plus the longest chain run (flying hooks). ~15 frames/s. `hooks_raw.csv` = 21.0-52.4 s.
- `hooktl.py RAW OUT`: per tick of his track: G (grabbed, anchor stable over frames), F (flying: a chain >= 90 units
  that does not reach a tile; shorter runs are his name tag / aim arrow / weapon), - (none), the anchor, and the pull
  direction against his velocity (track, +-2 ticks). `teero_hooks.csv` = k 978-2537.
- Spot checks (frames with the anchor and his velocity drawn): grabbed / braking / no-hook labels match the picture.
