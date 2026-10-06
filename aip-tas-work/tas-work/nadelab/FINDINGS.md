# nadelab: grenade boosts. Scenario 1 is maximum height from a single unhookable block

## Result

**Height gained: 2414 px = 75.44 tiles.** This is measured from the standing y (12785) to the minimum y (10371). The
spawn y is 12784, so the height is 2413 px from the spawn.

- **Input file:** `best_height.txt` (313 inputs; identical to `out_s1_0.txt`).
- **Map:** `block.map`. It is 300×500 tiles, all air except:
  - TILE_NOHOOK at tile (150,400);
  - the spawn entity at (150,399);
  - the grenade pickup at (150,398).
- **CTasGame replay** (`nadecheck`): 2414 px. CTasGame and CNadeG agree on every tick.
- **Real server** (`srvcheck.sh`, testrunner TasReplay.Run): 2414 px, min y 10371 at tick 409. Confirmed.
- **Optimality for this stack size:** the result equals the exact maximum for "ground jump + 3 grenades". Right after
  the stack, vy = -13.199 - 3·12 + 0.5 = -48.699 px/tick. The apex is computed with integer position quantization.

## Technique (input step numbers, 1-based)

**Setup.**
- Step 1: the tee picks up the grenade (the pickup sits 32 px above the spawn).
- Step 2: it switches to the grenade (weapon 3).

**Stage 0: 2-stack, launch vy = -36.7.**
1. Step 3: ground jump while drifting left.
2. Step 15: fire nearly straight up (aim 864,-30000) from (4770,12659), which is 46 px left of the centre and 126 px
   up. This lob hangs for 79 ticks and comes back down onto the block.
3. Air-control back onto the block and land.
4. Step 92: ground jump.
5. Step 93: point-blank shot straight down.
6. The lob and the point-blank shot explode in the same tick (93), 28 px under the tee. Each gives a kick of (0,-12).

**Stage 1: 3-stack, launch vy = -48.7.**
1. During the 2-stack flight, drift left at about 5 px/tick (steps 94–155).
2. Step 213: on the way down, fire nearly straight up (aim 5453,-30000) from (4451,11932), which is 365 px left and
   853 px up. This grenade's 101-tick lifetime ends in mid-air at (4816,12780), exactly under the tee's launch spot
   at tick 313.
3. Fall beside the block. At step 241 the tee is level with the block top and 274 px to its left. In that same step,
   fire up (aim 5680,-30000; this one lands on the block top 73 ticks later) and use the air jump.
4. Air-control right (+1) back over the block, land and stand.
5. Step 312: ground jump.
6. Step 313: point-blank shot straight down.
7. Three explosions hit at tick 313: the lifetime grenade 8 px below the tee, the 73-tick lob and the point-blank
   shot. Each gives (0,-12).

The apex is reached at tick 409.

## Why only 3 grenades: the theory behind the search

**Slots.**
- Grenades must be fired at least 25 ticks apart and live at most 101 ticks.
- Every grenade explodes either on the block or in mid-air at exactly tau = 101 ticks after firing.
- A ground jump *sets* vy = -13.2 and an explosion adds at most 12, so the jump comes first and the stack follows in
  the next tick.
- So a stack is: jump + point-blank shot + "gates", where a gate is a grenade fired at tau >= 26 that lands under the
  tee at the stack tick.

**Gate geometry.** A grenade fired tau ticks before the stack reaches the block under the tee only if the fire point
lies on a circle of radius 21+20·tau, centred 0.28·tau² above that block point.
- **Bottom of the circle (aim up):** needs tau >= 73 and a tee that is low.
- **Top of the circle (aim down):** needs a tee that is very high and falling fast: 730 px at tau 26, 1769 px at
  tau 51.
- **Sides of the circle:** 500 to 2000 px away from the block.

**No-kick bound.** Assume no explosion other than the stack touches the tee in the final stage. The tee was launched
from the block, so it can never move back toward the block faster than 5 px/tick (air control alone).
- Under those assumptions, no tee trajectory reaches 3 gates. This holds with free fall, one air jump, landing and
  any starting state.
- `nadebound` checked all 3276 gate triples in a relaxed model: any explosion point on the block top or sides, 2 px
  slack.
- So the maximum without kicks is **2 gates + point-blank + jump = 3-stack (2414 px)**.
- The exact searches (`nadesearch` with the `ladder.py` beam) agree: rest → 2-stack (1382 px) → 3-stack (2414 px),
  and nothing higher.

**What remains unproven: kick-assisted stacks.**
- A mid-air lifetime explosion beside the tee could kick it toward the block at up to 12 px/tick. That grenade would
  be fired from a slot outside the final 101-tick window.
- With such a kick (approach speed up to 17 px/tick), the relaxed model *does* admit 3 gates. That would be a
  4-stack worth **3734 px**; a 5-stack would be worth 5342 px.
- No real plan of this kind was built or ruled out. That needs a stage search with mid-flight kicks (`nadekick`,
  started but not finished).

**Small known gap (about 4 px).**
- The engine also counts the tee as standing while it is up to 4 px above the standing height, so it can ground-jump
  from there.
- That would give 2418 px, but none of the found 3-stack timings line up with it.

**Sub-agent's view of the true optimum.**
- If kick-assisted stacks are impossible: 2414 px, or 2418 px with the 4 px trick.
- If not: a kick-assisted 4-stack is the one open route above that, worth up to 3734 px.

## Important for the main search: client prediction disagrees with the server

The client prediction code (CTasGame and CFastG) makes a grenade explode **twice** when it hits a tile **on its last
lifetime tick (tau = 101)**.
- In the prediction's CProjectile::Tick, the collision branch does not return, so the lifetime branch fires as well.
- The server returns after the collision, so it explodes **once**. The exception is maps with the
  BUG_GRENADE_DOUBLEEXPLOSION mapbug.

`demo_lasttick_hit.txt` shows the difference:

| Replay | Height |
|---|---|
| CTasGame | 3185 px |
| Real server | 2166 px |
| CNadeG (server semantics) | 2165 px from the spawn, matching the server |

Searches built on CFastG could exploit a kick that does not exist on the server. (Main session: CFastG fixed to
server semantics. The lifetime branch is now `else if`; the backup is `fastg.cpp.bak_dblexpl`.)

## Tools

All files are in `nadelab/`. The build targets are appended to `ddnet/CMakeLists.txt`.

- **Build:** `cd ddnet/build-sim && ninja nademap nadecheck nadesearch nadeaim`.
- **`nade.h/.cpp`:** CNadeG, a copy of CFastG with two changes: it allows 16 grenades in flight, and it uses the
  server's projectile semantics (one explosion on a last-tick hit, counted in `m_DoubleRisk`). It also provides:
  - `FlyGrenade`: the exact flight of a grenade.
  - `SolveAim`: an integer aim for which a grenade explodes exactly tau ticks later, as close to straight under a
    target as possible (tile hit or lifetime end, with fine angle refinement).
- **`nademap OUT.map W H b:X,Y[,T] r:X0,Y0,X1,Y1[,T] s:X,Y g:X,Y`:** writes a map with blocks, rectangles, a spawn
  and a grenade pickup.
- **`nadecheck MAP INPUTS [trace] [extra]`:** replays on CTasGame and CNadeG side by side. It prints mismatches,
  every explosion and the height.
- **`srvcheck.sh MAP INPUTS [extra]`:** replays on the real server (testrunner) and prints the height.
- **`nadesearch MAP PREFIX OUTPREFIX [key=val]`:** runs one stage of the search for a scenario. A scenario is a map,
  a start state (the replay of PREFIX) and an objective (here: the apex above the launch platform).
  - It finds the platform under the spawn; `px=` and `py=` choose another.
  - It enumerates jump programs and computes, for every stack tick E, the band of fire positions from which a grenade
    lands under the tee ("gate bands").
  - It runs a depth-first search over gates, checking that the tee's horizontal path between them is possible.
  - It realises the best candidates exactly (x-controller, SolveAim, replay) and writes `OUTPREFIX<i>.txt`.
  - Main options: `tmax`, `wait`, `hop`, `top`, `out`, `kmax`, `align`, `airfinal`. `tp=x,y,vx,vy` teleports the
    tee and is for exploration only.
- **`ladder.py MAP PREFIX OUTDIR stages=3 beam=4 [nadesearch opts]`:** chains stages with a beam search.
  - `./ladder.py block.map p0.txt runs/block stages=3 beam=4 tmax=330 hop=0 top=60` reproduces 1382 px, then
    2414 px, in 16 s.
  - `p0.txt` holds the 2 setup inputs.
- **`nadeaim MAP PREFIX TX TY [tau0 tau1 allowlast onlyhits]`** and **`nadeaim MAP PREFIX fly AX AY`:** aims and
  exact flights from the state after PREFIX.
- **`nadebound*.cpp`:** a standalone relaxed feasibility checker for gate sets (build with `g++ -O2 -std=c++17`). It
  handles a single block only.
