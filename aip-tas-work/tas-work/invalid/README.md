These runs finish on a plain x86-64 build (gcc, no FMA) but freeze at input line 2167/2168 (race ~2099, (774, 3328))
on builds with fused multiply-add (the user's server; gcc -mfma -ffp-contract=fast; clang -mfma -ffp-contract=on):
their x is 1 px smaller there and DDNet's 1-px freeze sampling of the 2166 -> 2167 move lands 0.6 px inside freeze
tile (24, 102). Not valid runs; kept for their lines. See NOTES.md "Cross-platform validity (Oct 11)".
