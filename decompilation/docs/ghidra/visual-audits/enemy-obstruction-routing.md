# Enemy obstruction routing

Status: **partial**. The active Lua route policy uses typed path and map-cell
queries and is covered by headless tests. A Scene 2 integration QA now writes
a route trace and `actual.bmp` from a live encounter actor crossing the
imported collision map. Route selection still has no independent GPGX movement
capture, so the visible route is not confirmed against the ROM game.

## Source behavior

- [C] ROM `0x14682` selects an obstruction route. It consults the dominant-axis
  chooser at `0x148D2`, the first-open scan at `0x148AA`, paired bypass probes
  at `0x14854`, and the route queue at `0x149C0`. If those paths fail, the
  retry uses the five-direction strip search at `0x14AF4`.
- [C] `0x148D2` selects D7 as the greater absolute target axis, with X taking
  ties, and D6 as the signed minor axis. ROM words at `0x148CA` are
  `-0x40, +1, +0x40, -1`; they map zero minor deltas to -Y on horizontal
  routes and -X on vertical routes. `0x148AA` scans along D7 until its first
  occupied cell, backs up one D7 step, and returns the last open cell
  (`0x148B6..0x148C8`). When that differs from D5, `0x146C8..0x146E2` queues
  that returned cell; it does not queue only the first neighbor. If D7 is
  blocked immediately, `0x14854` scans D6, tests each candidate cell and its
  D7 neighbor, and retries from the origin in the opposite D6 direction when
  the first scan fails (`0x14864..0x148A8`).
- [C] `0x148D2` first stores `abs(actorX-targetX)` in D5 at `0x148E8` for the
  dominant-axis comparison. At `0x148FE` it restores the saved actor X/Y pair;
  execution falls through to `0x14902`, which converts those actor coordinates
  to a grid index and overwrites D5 at `0x14910`. Back in `0x14682`, `0x146B8`
  checks that actor-origin cell in `FF1700` before scanning. A blocked origin
  goes directly to `0x14756`, clearing both route flags. The Lua planner now
  queries that typed origin cell before testing any detour candidates.
- [C] `0x14914` converts both endpoints with `0x14902` and asks `0x1494C` to
  check two axis-aligned corridors: across the first endpoint's row and then
  along the second endpoint's column, followed by the same ordering with
  endpoints swapped. A corridor is blocked by any nonzero cell. The route is
  blocked only when both corridors contain an obstruction; a clear ordering
  retains direct movement. `0x14902` groups world coordinates in `0x200`-unit
  cells; its low six index bits represent X and the next row begins `0x40`
  cells later. `0x14B4E` projects route cells to their `+0x100` centers.
- [C] `0x14AF4` retries five heading-rotated direction pairs. Each call to
  `0x14800` walks primary cells until a primary cell is blocked or the active
  map bounds are reached; five is the retry count, not the strip depth.
- [C] Scene setup at `0x3074` selects the scene descriptor from the `0x3232`
  table and copies its 48 words to `FFDDC6` (`0x3092..0x30AA`). Descriptor
  `+0x0E` is therefore `FFDDD4`, and `+0x24` is `FFDDEA`. Before reading the
  primary cell, `0x14816..0x14836` rejects negative D1, compares its low-six
  column to `FFDDEA`, then compares its row to `FFDDD4 >> 5`. The adjacent
  side-cell read at `0x14842..0x1484A` has no corresponding bound check. Scene
  2's descriptor at `0x172C80` contains `+0x0E=0x0518` and `+0x24=0x0078`;
  these exact source values are kept in the typed field metadata.
- [C] The ordinary movement collision path checks terrain and does not test
  another actor's coordinates. The independent field-enemy stacking trace
  records that source boundary in
  [field-enemy-stacking-trace.md](field-enemy-stacking-trace.md). This change
  leaves target selection and line-of-sight policy intact.
- [C] Obstruction routing is not limited to dynamic encounters. `0x142AE`
  loads a default patrol actor's next `+0xE0` waypoint into `+0x96/+0x98`;
  after a blocked shared movement pass, `0xBB66` marks `+0xB8=-1`, and
  `0x145A2 -> 0x14BA6` takes the negative-state branch at `0x14BF8` into
  `0x14682`. These are ordinary source actor records using the same terrain
  route selector.
- [C] `0x13904` can retain a linked target at `+0xA2` and enter `0x13D0C`,
  which measures target-relative movement and hands its destination to
  `0x145A2`. The source target-list selector at `0x14492` uses the seven-actor
  pool when actor `+0x10` bit 3 is set; the typed host fact for that condition
  is `lifecycle.ai_runner`. Blocked movement reaches the shared `0x14682`
  selector, so this path does not require dynamic-encounter or patrol-list
  metadata.
- [C] The visible flee pass also uses the shared movement target: `0x13E9E`
  derives the field-boundary destination and stores it at `+0x96/+0x98`, then
  `0x13E8C -> 0x145A2 -> 0x14BA6` reaches `0x14682` when movement is blocked.

## Host implementation

Lua stores a target-linked list of typed world waypoints in the actor's
existing `game.actors[actor_id]` state. On a blocked movement reset, it uses
the live target candidate for a dynamic actor or for an actor whose typed
`lifecycle.ai_runner` fact selects the linked world-target pool. It also uses
the current Lua-owned `ai_route_target` for an actor with extracted
patrol-route data. The route target remains identified by the actor and patrol
cursor; Lua does not expose the source record's `+0x96/+0x98` fields. The
typed map-cell query scans D7
through the last open cell and stops at the first blocked cell, matching
`0x148AA`; it then checks D6 candidates and their D7 neighbors, matching
`0x14854`. The same fact query checks primary and adjacent cells in the
heading-rotated strips from `0x14800`. Lua stores those results as ordinary
world waypoints. It advances the waypoint index as the actor enters each
waypoint cell and measures the live target again when the route ends.
The visible flee branch also resolves its Lua-owned `flee_target_x/y` as a
named `flee-boundary` target when the actor projection is active and its
behavior facts say it is fleeing. That target comes from `0x13E9E` and joins
the same `0x145A2 -> 0x14BA6 -> 0x14682` route; no source record offsets are
exposed to Lua [C]. The shared host eligibility gate admits this projected
fleeing state even when the actor is neither marked as a dynamic encounter nor
registered with a default patrol route. The flee destination takes precedence
over a retained combat-target link because the ROM writes the flee destination
into the shared movement target before this caller [C].
When a queued segment becomes blocked, ROM `0x149C0 -> 0x149CE ->
0x14756` clears the two route flags. The next `0x14682` selection derives its
route again from live position and target; the source does not retain a prior
side choice. Lua recomputes from live state instead of flipping a host-only
side field [C]. The start cell is the actor's current `+0/+4` coordinates,
converted by `0x14902`; `0x148D2`'s temporary absolute X delta is replaced
before `0x14682` indexes the map [C].

When no paired bypass is usable, Lua tries strips in the source heading order
from `0x14AF4`. It represents candidates as typed world waypoints and asks the
map service about each primary and adjacent cell. A regression reaches an
opening at depth six, beyond the five direction pairs. The direct `0x148AA`
and `0x14854` loops have no source scan-depth counter [C], and Lua no longer
adds a 64-cell limit. Those scans stop when the typed cell query finds a
blocker or when the candidate reaches the host's `0..0xFFFF` coordinate edge;
that edge policy remains [H] because ROM word-wrap and out-of-map behavior
have not been independently verified. The `0x14800` strip scan instead uses
a typed `source_strip_primary_cell_is_in_bounds` query from the active field
descriptor and has no fixed depth cap.

The strip terminus now preserves the last traversable primary cell. ROM
`0x14800` advances the primary index while the primary cell is open and the
neighboring side cell remains blocked; when the next primary cell is occupied
or beyond the map bounds, it subtracts one primary step and returns the prior
index (`0x14812..0x14852`). `0x14AF4` then accepts that result if it differs
from the starting cell and stores it as the one-entry route (`0x14B22..0x14B48`).
Lua now keeps the last clear typed waypoint when its next source-cell query is
blocked or outside the imported map, and queues it if it is outside the
actor's starting cell. The new `ai_think_tests.lua` case checks a multi-cell
strip whose next primary cell is blocked and asserts that the preceding open
cell is retained. This implements the source's scan-termination placement;
the independent scene route remains unverified.

If a replan cannot find a clear candidate, Lua drops the previous waypoint
queue and restores the live target heading. This follows ROM `0x14ADE`'s
failure branch to `0x14756`, which clears both obstruction-route flags. The
`ai_think_tests.lua` regression starts with a queued detour, blocks every
candidate, and checks that movement returns to the target rather than reusing
the stale point.

Lua calls
`game.movement.source_cell_is_clear(x, y)` for the direct FF1700-style cell
probes in `0x148AA`, `0x14854`, and `0x14800`. For the primary cell in the
`0x14800` strip only, Lua separately asks
`game.movement.source_strip_primary_cell_is_in_bounds(x, y)`. The C++ adapter
converts that position with `0x14902` and checks the typed field descriptor
values using the source comparisons. The cell query reads the active terrain
map through the named host service, including the live field-barrier overlay
and selected effect-marker replacement. Neither query exposes FF1700 or a
raster array to Lua. `game.movement.source_path_is_clear(from_x, from_y, to_x, to_y)` remains
the separate `0x14914/0x1494C` two-corridor query for an active route. If
either corridor is clear, Lua retires the queued detour and measures the
current target again, matching the
`0x149C0 -> 0x149CE -> 0x14756` branch. A cell outside the imported map is
blocked for host safety [H]. The selector now checks the typed actor-origin
cell first, matching `0x146B8`; if occupied it drops route selection without
probing a detour. The former 32-unit sampled host collision sweep was removed.

ROM `0x14682` enters candidate selection without calling `0x14914`; it scans
the direct cells used by its `0x148AA/0x14854/0x14800` helpers. The Lua
planner now follows that separation: source-cell facts choose waypoints, and
the path query only validates a waypoint queue that is already active [C].

The actor-origin lookup, D7 last-open-cell return, signed D6 probes,
opposite-D6 fallback, blocked-route replan state, heading order, descriptor
edge comparison and previous-cell return of the `0x14AF4` retry, and `0x14914`
path query are [C]. The typed host cell query treats a cell outside the
imported collision map as blocked [H]. The unchecked `0x148AA/0x14854` cap,
coordinate-wrap edge cases, and resulting route pixels remain unverified.
These are readable native waypoint policies, not a claim of matching the
complete waypoint sequence. The referenced
`/workspace/shadowrun/out/p49b/repro.txt` route capture is absent from this
checkout; local obstruction-grid snapshots do not establish enemy route
fidelity.

## Validation

- `ai_think_tests.lua`: mock typed cell facts and source-corridor responses
  for occupied-origin rejection, positive and negative major directions,
  multi-cell last-open placement,
  signed minor bypasses, opposite-D6 fallback,
  clear direct path, sealed corridor, heading-oriented strip fallback
  including an opening at depth six, last-open-cell retention, live-position
  replan, failed-replan cleanup, target-linked route state, waypoint
  advancement, target reacquisition, and a blocked default patrol route using
  `field_scene_20_actor_0`. These mocks validate the typed policy, not imported
  occupancy or an independent Genesis route.
- The flee regression uses a non-dynamic projected actor with no patrol route
  at `(0x2000,0x2000)` and the source-derived boundary target
  `(0x3000,0x2000)`, while retaining a combat target at `(0x5000,0x2000)`.
  It blocks the direct step and verifies that shared route eligibility lets
  the selector queue the last open cell `(0x2300,0x2100)` under target ID
  `flee-boundary`. Full Lua suites and
  `shadowrun_gameplay_actor_ai_rules_qa` pass in headless Debug and Release.
  This verifies target plumbing and typed waypoint policy; it does not prove
  imported occupancy or independent Genesis route pixels.
- A non-dynamic actor with typed `lifecycle.ai_runner=true`, no patrol route,
  and a live linked target now enters the shared blocked-route planner. The
  Lua regression forces the movement probe to block and checks that the
  planner queues a waypoint for the retained target. This validates semantic
  eligibility; its collision facts are test inputs.
- `shadowrun_gameplay_script_runtime_qa`: verifies the Lua planner receives
  the separate typed map-cell and source-corridor callbacks through the C++
  binding.
- `entity_route_primitives_conversion_test`: checks the two `0x1494C`
  corridor orders, the `0x14914` either-clear result, and a diagonal-only
  obstruction that the source query misses.
- The focused headless Qt CTest run passed in Debug and Release for the
  gameplay Lua suites, script-runtime binding, actor AI rules, world activity,
  and field-barrier collision. The separate `equiv_heartbeat` output remains
  informational and reports its documented pre-split mismatches.
- After adding default-patrol routing coverage, Debug and Release
  `shadowrun_gameplay_actor_ai_rules_qa` and
  `shadowrun_gameplay_script_runtime_qa` both pass headlessly.
- A Lua regression scans a 70-cell primary strip to an opening, beyond the
  previous host-only 64-cell cap. The scan now asks the typed field-descriptor
  bound before each primary-cell probe; adjacent side-cell probes retain the
  source's unchecked behavior.
- The direct `0x148AA` and `0x14854` scans also have no ROM depth counter. The
  Lua D7 regression passes 65 clear cells, stops at the next blocked cell, and
  returns the preceding open waypoint; the D6 regression passes 65 candidates
  before it accepts the adjacent D7 cell. Both direct scans stop at the host
  16-bit coordinate edge [H].
- `entity_route_primitives_conversion_test` checks negative-index rejection
  and the descriptor row/column comparisons. The field source-assets test
  verifies Scene 2's imported `+0x0E=0x0518` and `+0x24=0x0078` values.
- Headless Debug and Release passed the focused Qt Lua/runtime tests (2/2 in
  each configuration) and the route-primitives/source-assets tests (2/2 in
  each configuration). These are focused checks, not full-suite results.
- `shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` now resets
  Scene 2, spawns the ordered Weenr pair, and moves one live actor toward the
  controlled runner across an imported-map wall. The fixture checks the
  blocked row 14 corridor (cells 27..33), the clear row 12 detour, the live
  `party.runner.0` link, clear occupied cells during movement, and lateral
  progress past the wall. Its `obstruction-route/route.tsv` and
  `obstruction-route/actual.bmp` are headless internal port artifacts. The
  imported cell layout and route pixels remain [H] until checked against an
  independent Genesis capture; #49 remains partial.
- The focused field encounter CTest passes in headless Debug and Release. The
  Debug and Release `obstruction-route/actual.bmp` files have matching SHA-256
  `8261A58F77F75333BFCDB5F185FCC8118AC62860D4165ED566488AB3587E5517`.

## End-to-end Scene-2 route to engagement range (2026-11-07)

ROM `0x15C58` accepts an ordinary creature's linked target below distance
`0x0200`; the distance measure at `0x15D4E` is max-axis distance plus half the
min-axis distance. The Scene-2 integration QA now keeps the spawned Weenr
active through the full obstruction detour and live-target reacquisition,
rather than stopping once it has crossed the wall. It checks the row-14 wall,
the row-12 clear detour, the live `party.runner.0` target, and every traversed
cell before ending at the source engagement threshold.

The headless Debug and Release routes both reach distance `508` at route frame
`352`, with the Weenr at `(17695,6931)` and the runner still selected. Each
run emits 45 route samples plus `obstruction-route/actual.bmp`; both full-screen
images have SHA-256
`5CDAF8061A18BB3E56E6E6445A6C782AE9B7507D2CFAA8889090091F14693B99`.
This proves the integrated port path in this imported Scene-2 fixture. The
collision-map layout and route pixels remain [H] without an independent
Genesis route capture, so item #49 remains partial.
