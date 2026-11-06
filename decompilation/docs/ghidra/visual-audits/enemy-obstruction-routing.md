# Enemy obstruction routing

Status: **partial**. The active Lua path policy and typed host geometry query
are implemented and covered by headless tests. Route selection has not been
compared with an independent GPGX movement capture, and no field-scene image
dump defends the visible result.

## Source behavior

- [C] ROM `0x14682` selects an obstruction route. It consults the dominant-axis
  chooser at `0x148D2`, the first-open scan at `0x148AA`, paired bypass probes
  at `0x14854`, and the route queue at `0x149C0`. If those paths fail, the
  retry uses the five-direction strip search at `0x14AF4`.
- [C] `0x148D2` selects the greater absolute target delta as its major axis,
  with X taking ties. It keeps the signed minor-axis direction as the first
  bypass side; the direction table at `0x148CA` maps zero minor deltas to -Y
  on horizontal routes and -X on vertical routes. `0x14854` tries the opposite
  side only after its first side fails.
- [C] `0x14914` checks the obstruction line between the actor and target in
  both directions. `0x14902` groups world coordinates in `0x200`-unit cells;
  its low six index bits represent X and the next row begins `0x40` cells
  later. `0x14B4E` projects route cells to their `+0x100` centers.
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
movement step, it checks the dominant axis and tries the signed minor-axis
side first, then checks whether the direct and parallel world segments are
clear through the host query. It advances the waypoint index as the actor
enters each waypoint cell and measures the live target again when the route
ends. Reversing the side after a failed replan in the same cell is host policy
and remains [H].

When no parallel corridor is usable, Lua tries a strip from the actor's
current heading. The source rotation order follows `0x14AF4`, and each scan is
bounded by the source grid dimension and the active scene's typed collision
query. Lua represents the candidate as a world waypoint, advances the terrain
query one cell at a time, and checks each candidate's adjacent open cell. The
incremental query keeps long scans from repeatedly sweeping the full distance
from the actor. A regression reaches the first open adjacent cell at depth
six, beyond the five direction-pair retry count. This gives an actor a local
step to replan from around a blocked route without exposing the source raster
to Lua.

If a replan cannot find a clear candidate, Lua drops the previous waypoint
queue and restores the live target heading. This follows ROM `0x14ADE`'s
failure branch to `0x14756`, which clears both obstruction-route flags. The
`ai_think_tests.lua` regression starts with a queued detour, blocks every
candidate, and checks that movement returns to the target rather than reusing
the stale point.

`game.movement.path_is_clear(from_x, from_y, to_x, to_y)` is a narrow read-only
host service. `FieldSceneRuntime` evaluates the active field's typed terrain
collision model at segment samples, using local collision records and sample
state. It does not publish movement events or mutate the actor's next movement
probe. Lua does not receive or construct an `FF1700` raster.

The initial signed side order and the heading order/scan bound of the
`0x14AF4` retry are [C]. The host's geometry sampling, repeated-replan side
choice, and resulting route pixels remain [H]. These are readable native
waypoint policies, not a claim of matching the source's exact grid occupancy
or complete waypoint sequence. The referenced
`/workspace/shadowrun/out/p49b/repro.txt` route capture is absent from this
checkout; local obstruction-grid snapshots do not establish enemy route
fidelity.

## Validation

- `ai_think_tests.lua`: synthetic horizontal and vertical obstructions, clear
  direct path, sealed corridor, opposite-side retry, heading-oriented strip
  fallback including a first opening at strip depth six, failed-replan cleanup,
  source signed-side and X-tie ordering, target-linked route state, waypoint
  advancement, and target reacquisition.
- `shadowrun_gameplay_script_runtime_qa`: verifies the Lua planner reaches the
  typed `ScriptMovementPathQuery` callback through the C++ binding.
- The complete `run_lua_suites.sh` run passes; `ai_think_tests.lua` reports
  326/326 and `entity_motion_tests.lua` reports 13/13. Its separate
  `equiv_heartbeat` output is informational and continues to report the
  documented pre-split mismatches.
- The Debug and Release focused CTest runs pass for
  `shadowrun_gameplay_lua_suites` and
  `shadowrun_gameplay_script_runtime_qa` with offscreen Qt and dummy audio.
- No scene `actual.bmp` was produced for this shared movement mechanism, so
  the obstruction-routing hill remains partial.
