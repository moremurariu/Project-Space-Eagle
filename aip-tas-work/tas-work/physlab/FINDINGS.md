# Physics lab findings (AiP-Gores KoG, pre-grenade) — tas-work/physlab/

(Written by the physics-experiments agent; saved here by the main session. All numbers measured in lab;
scripts are in this folder. State after N input lines = race tick N - 68.)

## Bottom line
- No input sequence beats `turn357` in the pillar turn; the arch is within ~1 tick of the physical limit.
  The remaining 1-1.4 ticks there are set by the corridor -> left-shaft entry (rt 276-296): Teero enters lower /
  further left with less upward speed (~-18..-19.5 at x~8960 vs our -20.1).
- No structural gain found by local search in rt 730-930; the loss there is horizontal speed (Teero ~21 px/t vs
  our 20; our vx 21-22 sits just under the 21-px step at ~22.6), turning vx into climb at rt 773/775, the landing
  at rt 736 (-292 E) and ceiling bumps at rt 816 (-87) and 898 (-133). Avoiding the rt 736 landing needs an earlier
  U-turn line (Teero's held -59 deg hook / seg dirmode=tan), i.e. a beam search, not local search.
- **Teleport jump-state bug** (old benchmarks only): setting m_Jumped to 2|3 after a teleport without setting
  m_JumpedTotal gives the air jump back on the next tick (tas tp=...,3, tpjumped/prefixjumped, polish, nest,
  tas2 jumped=, lab `jumped`). So old "jumps spent" teleport benchmarks (q14 table, "corridor 1 from a Teero-like
  crossing") actually had the air jump. seg's tp= does not touch the jump state (unaffected). Final runs never
  teleport (unaffected). Fix: also set m_JumpedTotal = 1 when bit 2 is set, or perform a real air jump first.

## 0. Tools
- `labx.py`: `run(prefix, inputs)` replays prefix + continuation and returns per-tick states; `batch(start, seqs)`
  runs hundreds of experiments in one lab call (~1 ms per 50-tick experiment): parks the tee on the spawn floor for
  160 ticks (clears freeze), does a real air jump if the start has jumps spent, then teleports and runs. Verified
  100/100 identical to full replays. Start state must have an idle hook. Parser note: lab prints `hook -1`
  (retracted); a regex expecting `hook (\d+)` silently drops those lines.
- `seqopt2.py` (dir per tick, hook segments with angle, jump ticks; mutation hill climbing), `win.py RUN RT0 NT GATE
  LAM ITERS SEED OUT` (window hill climb, objective t_gate - LAM*E), `perturb.py`, `track_win.py` (Teero-track
  imitation at fixed lag), `tcmp.py RUN k0 k1 step` (Teero smoothed / de-ramped v and E vs a run, nearest point),
  `rotscan.py` (rotation-pulse efficiency table), `q1_*.py`, `q2_*.py`.
- Local / perturbation search over inputs gains at most 0.1-0.6 tick per window: the corridor entry and the U-turn
  exit are very narrow tubes in input space (4/25000 and 68/6000 jittered variants survive).

## 1. Turn over the pillar (Q1)
### 1a. Teero's line = turn357's line
Teero's 37.3 deg aim from k300 (8848,514) hits the pillar's left face at (9024,~648) (the freeze column 281 in front
doesn't stop the hook). turn357's 64.2 deg aim fired at rt 298 from (8920,427) hits (9024,642): the same anchor,
held through the arch with dir 1. Best continuation from the 992 run's rt-296 state: `runs_q1/A_turn357.txt_0.txt`,
full `runs_q1/q1_turn_A.txt`.

| run | y>=450 in shaft | y>=600 | y>=1000 (x<9300) | y>=1100 (x<9248) |
|---|---|---|---|---|
| 992 | 334.40 (v 4.5,19.7) | 341.16 | 356.88 | 360.61 |
| turn357 | **332.10** (v 3.0,19.6) | **339.00** | **354.42** | - |
| q1_turn_A (from 992 @296) | 332.42 | 339.71 | - | - |
| q1_turn_full | 332.42 | 339.55 | 355.92 | 360.00 |

Nothing better than turn357 found (20k random one-hook families, ~100k hill-climb evaluations, earlier pillar hooks
rt 293-297, straight-down hook (out of reach), air-jump brake (needs a jump we lack), hill climb from rt 276 (best
332.05), 25000 jittered corridor entries (best 332.05)).

### 1b. Why: vertical-acceleration limit
t450 depends only on the vertical profile (top 12 of 6480 dir/hook-length variants all 332.53). Down-pull is *0.3,
so the vertical deceleration while rising is at most 1.4/tick incl. gravity; the line achieves 1.38-1.39 from rt 300
to 314. It can't start before rt 300: above 15 px/t the pull applies only if |v| drops; rising with dir 1 that needs
tan(phi) > ~3.17*vx/|vy| (anchor nearly straight below). Measured with one-tick hooks at a floor below:

| v | pull applies for anchor angles | predicted |
|---|---|---|
| (22,-21) | >=75 deg | 73 |
| (22,-10) | >=85 deg | 82 |
| (12,-21) | >=65 deg | 61 |
| abs(v)<15 | all | - |

For the pillar-face anchor this first holds at x~8960 (rt 300); a hook fired earlier grabs at rt 297 but doesn't pull
until rt 300. Nothing usable lies below the tee (bare blocks at (277,26) ~407 px away, beyond the 380 px range).
dir 0 / -1 lowers the x factor of a right-side anchor (0.95 -> 0.75) and widens the cone to tan(phi) > 2.5*vx/|vy|
(why the optimizer picked dir 0 at rt 297). Descending above 15 px/t, a down-pull only applies together with vx braking
(pillar anchor: about +0.7 vy, -1.4 vx per tick).

### 1c. Remaining gap
From (8960,385) to y=452 in the shaft: Teero 31 ticks, turn357 / best 32.1-32.4. His apex 263, ours 250-254: less
upward speed at x~8960 (his ~-18..-19.5, noisy; ours -20.1 at rt 300, -21.4 at rt 299). Remaining 1-1.4 ticks are set
by the corridor -> left-shaft entry (rt 276-296). Lowering vy alone (teleport) doesn't help (-1: +0.2 tick; -2..-4:
worse, the line meets the slanted pillar face); the entry must also be lower / further left like Teero's (8848,514) at
true tick 297 vs our (8901,449). Nearest-point lag jumps ~3 ticks at the right corner from line geometry alone: gate
this section at y>=450 (or 600) in the right shaft, scored t - 0.3*vy.

## 2. U-turn exit to the loop (Q2, rt ~730-930)
### 2a. Energy ledger of the 992 run (E = v^2 - y)
| rt | event | dE |
|---|---|---|
| 708-724 | U-turn hook braking | ~ -680 |
| **736** | **landing on block (256-288, top 1728) at vy 18.4** | **-292** |
| 758 | air jump at vy +4.8 | +133 |
| 761->762 | edge hover on bare pillar (800-832, top 1792), ground jump while rising vy -9.6 | +95 |
| 773 | air jump at vy -7.3 + up-left pulse | +104 |
| 785->786 | edge hover, ground jump while rising vy -10.6 | +74 |
| 812 | air jump at vy +0.8 | +155 |
| **816** | **ceiling bump under block (59-61,40) at vy -9.6** | **-87** |
| 854/855 | landing vy 5.8, ground jump at vy 0 | -35, +187 |
| 872 | air jump at vy -3.4 | +144 |
| **898** | **ceiling bump under block (113-114,36) at vy -12.3** | **-133** |
| 906-934 | hook braking | ~ -280 |
| 980-990 | braking for the pickup | ~ -800 |

### 2b. Teero vs us (`python3 tcmp.py ../kog_pregren_992.txt 740 930 3`)
True lag grows linearly: +5 (k740), +8 (k812), +10 (k848), +13 (k926). Cause: horizontal speed; Teero's de-ramped vx
is 1-2.5 px/t higher: 21 px/tick vs our 20 (19 at rt 830-842); over 150 ticks that is the 7-8 ticks. Our vx 21.0-22.1
sits just under the 21-px step (needs vx ~22.6 at these speeds). At rt 773/775 up-left pulses turn vx 23.9 -> 21.0 into
climb that never comes back. Teero: dir 1 every tick, same jump count (7), edge-hovers the pillar at true ~756 and
air-jumps at 757, passes corner (288,1728) ~5 px higher instead of landing, dives to y~1835 and turns the fall into vx
with up-right aims (vx 22 -> 25). The "E up to 150 higher" from the track is only +-50 per point (noise).

### 2c. Demonstrated
- win.py rt 731 -> x>=1100: 775.71 vs 776.26 (-0.55 tick, E -902 vs -929), `runs_q2/w731_s1.txt` (drops the rt 773/775
  pulses, air jump at 776, keeps vx 24.27) - myopic: that lower line fails the climb before x>=1800.
- rt 804 -> x>=3100: -0.2; rt 848: -0.08; rt 880 -> x>=4100: none. perturb.py rt 717 -> x>=1800: 68/6000 survive, same
  time (811.8), E +14.
- No sequence avoids the rt 736 landing (rt 731: 10200 pulse variants, all survivors land; rt 717: 9120 two-hook
  variants, all land and are slower; imitation climbs stay in the landing basin). Needs an earlier U-turn line from a
  beam search.

## 3. General mechanics (verified)
### 3a. Hover-ground (grounded without contact)
IsOnGround tests the two bottom corners at (x+-14, y+19): within 5 px above a bare solid at the start of a tick counts
as grounded. That tick, without contact: jumps refill with no velocity change (1-5 px above: yes; 6: not yet); jump =
ground jump (-13.2) **keeping the air jump**; one corner over a block edge is enough (centre x 2546 with edge 2560 works,
2545 doesn't). **Ground friction applies while hovering**: dir 0 halves vx (20 -> 10 in one tick), dir -1 costs 2,
dir +1 above vx 10 costs nothing. A hard landing can be replaced by a jump if already grounded at the tick start. The
992 run uses edge hovers at rt 522, 761, 785, 971. Repro:
`lab AiP-Gores.map "quiet;tp 144 300 0 0;in 0 1 0 0 0 -1 1;in 0 0 0 0 0 -1 1;tp 2600 1678 20 4;loud;in 1 1 0 0 0 1000 1;in 1 0 0 0 0 1000 11;in 1 1 0 0 0 1000 1"`

### 3b. Ramp couples vy into x displacement
x displacement per tick = round(vx*ramp(|v|)) with |v| incl. vy (measured at (8300,700), dir 1; formula exact):

| vx \ vy | 0 | 5 | 10 | 15 | 20 |
|---|---|---|---|---|---|
| 20 | 19 | 18 | 18 | 18 | 17 |
| 22 | 20 | 20 | 20 | 19 | 19 |
| 24 | 22 | 21 | 21 | 21 | 20 |
| 26 | 23 | 23 | 23 | 22 | 22 |
| 28 | 24 | 24 | 24 | 24 | 23 |
| 30 | 26 | 25 | 25 | 25 | 24 |

Upper bounds of x lost to vertical speed in the 992 run: rt 0-296 49 px (~1.8 ticks), 296-745 154 px (~6.9),
745-930 38 px (~1.9), 930-992 40 px (~1.9). Rounding is half away from zero (vy 0.5 moves 1 px).

### 3c. Rotation pulses turn a falling v ~2.5x faster than a rising one
One-tick hook, dir 1, best aim with |v| not increased (`python3 rotscan.py`):

| v | rotation/tick | d|v| | dvx |
|---|---|---|---|
| (20,+15) falling | 6.72 deg | -0.34 | +1.38 |
| (20,-15) rising | 2.48 | 0 | +0.61 |
| (25,+8) falling | 6.51 | -0.20 | +0.61 |
| (25,-8) rising | 2.04 | 0 | +0.25 |
| (15,+20) falling | 6.57 | -0.59 | +1.85 |
| (15,-20) rising | 3.08 | 0 | +1.02 |
| (30,+5) falling | 5.64 | -0.10 | +0.30 |
| (30,-5) rising | 1.71 | 0 | +0.11 |

Route rule: trade height for speed on the way down (up-right pulses), never turn vx into climb.

### 3d. Vertical limits
Down-pull <= 0.9/tick + gravity 0.5 (cone-limited while rising, needs vx braking while falling above 15). Up-pull up
to 3/tick. Above 15 px/t the only energy sources are gravity and jumps: air jump +144 - vy^2, ground jump +174 - vy^2.
A jump and a hook pull can act in the same tick (rt 773: vy -12 -> -14.39).

### 3e. Convex corners zero both velocity components
MoveBox zeros both components when the diagonal sub-step collides but neither single-axis step does. Top-left corner
(2560,1696) of block (80-82,53), v (10,10): tp (2541,1677) -> v (0,0) (full stop); (2541,1676) -> lands (vx kept);
(2540,1677) -> wall (vy kept). One pixel flips the outcome: corner grazes are high-variance when re-searching a cut.

### 3g. Smaller facts
- A first-tick grab whose pull would raise |v| above 15 does nothing that tick; the pull starts on the first allowed tick.
- The hook retracts when > 380 px from the tee's current position (anchors behind a fast tee become unreachable).
- The map has only solid, freeze, start and finish tiles.

## 4. Suggestions for the beam search
1. Q1: gate the turn at y>=450 in the right shaft with a -0.3*vy credit; search from rt 276 with the arch as one
   pillar-face hook (anchor (9024,~645) held ~29 ticks); target upward speed <= 19 at x~8960, lower / further left.
2. Actions: macro "hold hook N ticks"; hover-aware jump (allowed whenever grounded at the tick start; ground jump that
   keeps the air jump).
3. Scoring: keep quant=1; charge contacts by lost speed (vy^2 for landings / ceilings, vx^2 for walls; rt 736/816/898
   cost 292/87/133); prefer turning fall into vx over turning vx into climb.
4. Q2: start a seg/LNS cut before rt 717 with the gate at x>=1800 (k812); keeping vx above the 21-px step looked worth
   ~0.5 tick per 40 ticks if the climb can still be made.
5. Fix the teleport jump state in tas/polish/nest/tas2/lab and re-check "jumps spent" teleport benchmarks.
