# Enemy obstruction routing

Status: **partial**. The active Lua path policy and the source-grid route
query are implemented and covered by headless tests. Route selection has not
been compared with an independent GPGX movement capture, and no field-scene
image dump defends the visible result.

## Source behavior

- [C] ROM `0x14682` selects an obstruction route. It consults the dominant-axis
  chooser at `0x148D2`, the first-open scan at `0x148AA`, paired bypass probes
  at `0x14854`, and the route queue at `0x149C0`. If those paths fail, the
  retry uses the five-direction strip search at `0x14AF4`.
- [C] `0x148D2` selects D7 as the greater absolute target axis, with X taking
  ties, and D6 as the signed minor axis. ROM words at `0x148CA` are
  `-0x40, +1, +0x40, -1`; they map zero minor deltas to -Y on horizontal
  routes and -X on vertical routes. `0x148AA` scans along D7 until it finds
  an occupied cell. If the first D7 neighbor is open, its caller queues that
  first step (`0x146C8..0x146E2`). If it is occupied, `0x14854` scans D6,
  tests each clear candidate's D7 neighbor, and retries from the origin in
  the opposite D6 direction when the first scan fails (`0x14864..0x148A8`).
- [C/H] `0x148D2` writes `abs(actorX - targetX)` to D5 at `0x148E8`, then
  returns to `0x14682`, where D5 is immediately used as an `FF1700` grid index
  (`0x146B8`) and as the starting index for `0x148AA`/`0x14854`. The ROM
  register dataflow is confirmed, but the intended origin represented by that
  value remains unresolved. The typed host implementation uses the actor's
  current cell as its origin [H]; it does not reproduce this register anomaly.
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
- [C] The ordinary movement collision path checks terrain and does not test
  another actor's coordinates. The independent field-enemy stacking trace
  records that source boundary in
  [field-enemy-stacking-trace.md](field-enemy-stacking-trace.md). This change
  leaves target selection and line-of-sight policy intact.

## Host implementation

Lua stores a target-linked list of typed world waypoints in the actor's
existing `game.actors[actor_id]` state. On a fresh blocked dynamic-actor
movement step, it checks the first D7 (dominant-axis) neighbor. When clear,
that one typed waypoint is queued. When blocked, it scans D6 (the signed
minor axis), then the opposite direction, and queues the first clear minor
candidate plus its clear D7 neighbor. Each candidate leg uses the typed
source-cell corridor query. It advances the waypoint index as the actor enters
each waypoint cell and measures the live target again when the route ends.
When a queued segment becomes blocked, ROM `0x149C0 -> 0x149CE ->
0x14756` clears the two route flags. The next `0x14682` selection derives its
route again from live position and target; the source does not retain a prior
side choice. Lua recomputes from live state instead of flipping a host-only
side field [C]. Candidate waypoint placement remains [H], including the
unresolved source D5 origin dataflow above.

When no paired bypass is usable, Lua tries strips in the source heading order
from `0x14AF4`. It represents candidates as typed world waypoints and checks
each segment with the same source-cell corridor query used by the queued
route. The route does not expose the source raster to Lua. A regression
reaches the first open adjacent cell at depth six, beyond the five direction
pairs. The native 64-cell scan cap and out-of-world handling are host policies
[H]; exact source boundary behavior depends on active map data and remains
under review.

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
`game.movement.source_path_is_clear(from_x, from_y, to_x, to_y)` for candidate
legs and queued-route validation. The typed host service applies the
`0x14902/0x14914/0x1494C` cell-corridor rule against the active terrain map.
The cell lookup includes the live player field barrier overlay, the selected
effect-marker replacement, and map-event counter updates. A cell outside the
imported map is blocked for host safety [H]. If either source corridor is
clear, Lua retires the queued detour and measures the current target again,
matching the `0x149C0 -> 0x149CE -> 0x14756` branch. The candidate waypoint
placement and native out-of-map policy remain [H]. The former 32-unit sampled
host collision sweep was removed so candidate planning and queued validation
use one obstacle definition.

The D7 dominant-axis first step, signed D6 scan, opposite-D6 fallback,
blocked-route replan state, heading order and scan-termination result of the
`0x14AF4` retry, and `0x14914` cell query are [C]. The typed host query still
treats a cell outside the imported collision map as blocked [H]. The source
D5 origin anomaly, ordinary first-bypass placement, host scan bounds, and
resulting route pixels remain unverified. These are readable native waypoint
policies, not a claim of matching the complete waypoint sequence. The referenced
`/workspace/shadowrun/out/p49b/repro.txt` route capture is absent from this
checkout; local obstruction-grid snapshots do not establish enemy route
fidelity.

## Validation

- `ai_think_tests.lua`: mock source-corridor responses for positive and
  negative major directions, signed minor bypasses, opposite-D6 fallback,
  clear direct path, sealed corridor, heading-oriented strip fallback
  including an opening at depth six, last-open-cell retention, live-position
  replan, failed-replan cleanup, target-linked route state, waypoint
  advancement, and target reacquisition. These mocks validate the typed
  policy, not imported occupancy or an independent Genesis route.
- `shadowrun_gameplay_script_runtime_qa`: verifies the Lua planner uses the
  typed source-cell corridor callback through the C++ binding for both
  candidate legs and an active queued route.
- `entity_route_primitives_conversion_test`: checks the two `0x1494C`
  corridor orders, the `0x14914` either-clear result, a diagonal-only
  obstruction that the source query misses, and the runtime cell-query
  callback contract.
- The focused headless Qt CTest run passed in Debug and Release for the
  gameplay Lua suites, script-runtime binding, actor AI rules, world activity,
  and field-barrier collision. The separate `equiv_heartbeat` output remains
  informational and reports its documented pre-split mismatches.
- No route-specific scene `actual.bmp` was produced for this shared movement
  mechanism, so the obstruction-routing hill remains partial.
