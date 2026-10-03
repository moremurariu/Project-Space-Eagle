# Rendering TAS runs to video

A TAS input file (one line per tick: `dir jump hook fire tx ty weapon`, see `docs/PHYSICS-SIM.md`) is turned into a video in two steps:
1. Replay it on the real server inside `testrunner`, which records a demo.
2. Play the demo in the DDNet client with `demo_render`, which writes an mp4.

The replay also checks the run: it prints the start and finish ticks and fails on a double start or a missing finish.

Needs `build-sim/testrunner` (`cd build-sim && ninja -j2 testrunner`) and `build-client/DDNet` (built with `-DVIDEORECORDER=ON`).

## 1. Record the demo

One run (`TasReplay.Run`):
```
cd build-sim
SIM_DEMO_DIR=$OUT TAS_DEMO=myrun TAS_MAP=/path/AiP-Gores.map TAS_INPUTS=/path/best.txt \
  ./testrunner --gtest_filter=TasReplay.Run
```

Two runs in one demo, one tee each (`TasReplay.Two`):
```
SIM_DEMO_DIR=$OUT TAS_DEMO=two TAS_MAP=/path/AiP-Gores.map \
  TAS_INPUTS=a.txt TAS_NAME=57.08 TAS_DELAY=30 \
  TAS_INPUTS2=b.txt TAS_NAME2=57.66 \
  ./testrunner --gtest_filter=TasReplay.Two
```
- Both tees are solo. They never collide, hook each other or push each other with explosions, so each run behaves exactly as it does alone. Check that the printed times match the single-run times.
- `TAS_NAME` / `TAS_NAME2` set the name tags shown above the tees.
- `TAS_DELAY` / `TAS_DELAY2` hold a tee's spawn back by N ticks. The tee waits as a spectator, and its inputs are not changed. To line both runs up at the start line, delay the run that starts earlier by the difference of the two `start tick` values that `TasReplay.Run` prints (e.g. 107 − 77 = 30).
- Tee 0 is client id 127 and tee 1 is client id 126.

The demo is written to `$OUT/<TAS_DEMO>.demo`. It always ends with a 100-tick (2 s) hold after the last input.

## 2. Render with the client

Use a private client storage so your real DDNet settings are not touched:
```
H=$OUT/.client-home
mkdir -p $H/user/demos
printf 'add_path %s/user\nadd_path $DATADIR\nadd_path $CURRENTDIR\n' "$H" > $H/storage.cfg
cp $OUT/two.demo $H/user/demos/
cd $H && /Users/c29/ddnet/build-client/DDNet "gfx_fullscreen 0; gfx_screen_width 1280; gfx_screen_height 720; \
  cl_default_zoom 10; snd_enable 1; cl_video_sound_enable 1; \
  demo_render demos/two.demo two 4 1 127"
```
- The video goes to `$H/user/videos/<name>.mp4`.
- The output is 2560×1440 at 60 fps on a Retina display, about 90 MB per minute.
- `demo_render <demo> <video name> [speed index] [quit when done] [spectate id]`. Speed index 4 = 1×, 1 = 0.25×, 0 = 0.1×. The spectate id picks the tee the camera follows.
- Sound: `snd_enable 1; cl_video_sound_enable 1` (or both 0 for a silent video).
- Optional: `cl_overlay_entities 100` shows the game tiles; a higher `cl_default_zoom` zooms in.

## Video time ↔ input line
The demo starts with one paused tick, then plays one input line per tick at 50 ticks/s. So at 1× speed, line N appears at about `(N + 1) / 50` s into the video. The in-game timer reading T s corresponds to line `start_tick + 50·T`, where `start_tick` is the one the replay printed. With `TAS_DELAY`, add the delay to the line number to get the demo tick.

## Pitfalls
- A dead player is respawned by the server on its own. That is why a delayed tee waits as a spectator, and why the unused third debug dummy is moved to spectators in `TasReplay.Two` (otherwise it shows up at the spawn point).
- The camera follows a single tee, so the other tee is only visible while it is near.
