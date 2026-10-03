# DDNet (local fork with physics experiments)

## Machine limits
- Use **at most 2 cores**: `ninja -j2` / `make -j2`, one simulation process at a time.
- Watch memory on long-running test binaries (a harness bug once used ~950 GB of swap). See the pitfalls in `docs/PHYSICS-SIM.md`.

## Builds
- `build-sim/`: Release, server + tests only (`-DCLIENT=OFF`). Target: `ninja -j2 testrunner`.
- `build-client/`: Release client with `-DVIDEORECORDER=ON` (Homebrew ffmpeg). Target: `ninja -j2 game-client`.
- Both are untracked. Regular build docs: `docs/BUILDING.md`.

## Physics experiments (branch `physics-experiments`)
- `docs/PHYSICS-SIM.md`: how the simulation harness works, how to run it, how to record and render demos, and pitfalls.
- `docs/PHYSICS-NOTES.md`: measured results (killtile skipping, velocity ramp, ninja, aleds), with pointers into the source.
- `docs/DEMO-ANALYSIS.md`: pulling runs from ddnet.org/watch and reading them tick by tick (`demo_dump`, `scripts/fetch_watch_demo.sh`, `scripts/demo_trace.py`).
- `docs/TAS-RENDER.md`: rendering TAS input files to video, one or two runs (tees) in one video, with sound, start-line alignment.
- `docs/HOOKBOT.md`: the chat-controlled hookfly/aled/pseudofly bot (`sv_hookbot 1`, `scripts/run_hookbot_server.sh`).
