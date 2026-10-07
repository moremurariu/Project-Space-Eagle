# AiP-Gores TAS work: how to use this folder
Unrelated to the rest of this repository: the AiP-Gores (KoG map) tool-assisted-run work, merged from the branches
`claude/compassionate-davinci-5q4n6j-postnade`, `claude/fervent-cray-0pser4`, `claude/fervent-cannon-0rrvvi` and
`claude/wonderful-faraday-klv0ms` (`aip-tas-fast/`). Status, benchmarks and the recommended tools: `README.md`,
`BENCHMARKS.md`. The full research log: `tas-work/NOTES.md` (and `fast/README.md` for the pre-grenade fast search).

## Build
```
bash setup.sh        # from aip-tas-work/
```
No `aip-tas.zip` is needed any more. `setup.sh` clones upstream DDNet at `DDNET_BASE_COMMIT.txt` into `ddnet-up/`, applies
`ddnet-upstream.patch`, links `ddnet-up/src/tas` to `ddnet/src/tas` (the sources in this folder), builds the tools into
`ddnet-up/build` (= `ddnet/build-sim`, `fast/ddnet/build`), fetches the KoG map (sha256 `353b27cf...`) to
`tas-work/AiP-Gores.map`, and runs the checks: `simbench` exactness fuzz (expect 0 mismatches) and the server-code
replays of `tas-work/kog_full_best.txt` and `pre_grenade_kog/kog_pregren_best.txt`.
Build a tool that is not in setup.sh's list with `ninja -C ddnet-up/build NAME`.

`ddnet-upstream.patch` contains:
- CMake targets for every tool: CTasGame tools (`seg pre lab pf polish match mapdump ...`), CFastG tools (`segf rdv
  fgcheck eaudit lobscan rejoin perturb`, every `src/tas/x_*.cpp`), CFast tools (`ddsearch replay fastcheck viacheck
  brute eacct`) and `simbench`;
- the exact collision fast paths of both lines of work, merged (see README "Simulator"): block / exist prefix sums with
  broad-phase skips, the TileExists cache, `ms_TasSolo`, plus the per-tile solid table for TestBox / IsOnGround /
  hook rays. `TAS_NOFAST=1` turns all of them off (plain DDNet code);
- the prediction-world fix for a grenade's double explosion on its last lifetime tick (matches the server);
- the server-code replay tests `TasReplay.Run` (used by `tas-work/srvfin.sh`, `srvcheck.sh`) and `TasServer.Run`.

## Input files
One line per tick from spawn: `dir jump hook fire target_x target_y weapon`. The race starts at input 68, so race
tick = input index - 68. Check any file on the real server code: `tas-work/srvfin.sh FILE`.

## Data that is not in git
`tas-work/teero_track.txt` (Teero's position per race tick, "k x y", k = -70..2539) is local only (`.gitignore`); so
is anything new in `tas-work/teero/` (the input extractions already there, e.g. `teero_inputs_0-3131.csv`, are tracked). The Teero-guided tools (`x_tig`, `tigloop.py`, `seg ghost=3 sinks=...`, `mapk.py`) need
the track; everything else runs on references built from our own runs (`SEG_TRACK=FILE segf ... prefix=RUN`).

## Pitfalls (from the notes)
- zsh does not word-split `$A`: put parameter sets in bash scripts.
- Never `cp` over a binary that has already run (rm first).
- `until ! pgrep -f X` loops match their own command line.
