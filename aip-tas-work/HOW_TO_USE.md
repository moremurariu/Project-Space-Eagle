# AiP-Gores TAS work (saved snapshot)
Unrelated to the rest of this repository: a snapshot of the AiP-Gores TAS work (DDNet tool-assisted run).
Restore: unpack the original `aip-tas.zip`, copy `ddnet/src/tas/*` and `tas-work/*` from here over it (or apply
`changes.patch` for the files that existed in the zip), add `pf` and `seg` to the `foreach(TAS_TOOL ...)` list in
`ddnet/CMakeLists.txt` (the patch does this), then `bash setup.sh`. Status and results: `README.md`, `tas-work/NOTES.md`.
