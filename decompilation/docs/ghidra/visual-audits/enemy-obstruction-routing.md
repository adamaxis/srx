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
- [C] `0x14914` checks the obstruction line between the actor and target in
  both directions. `0x14902` groups world coordinates in `0x200`-unit cells;
  `0x14B4E` projects route cells to their `+0x100` centers.
- [C] The ordinary movement collision path checks terrain and does not test
  another actor's coordinates. The independent field-enemy stacking trace
  records that source boundary in
  [field-enemy-stacking-trace.md](field-enemy-stacking-trace.md). This change
  leaves target selection and line-of-sight policy intact.

## Host implementation

Lua stores a target-linked list of typed world waypoints in the actor's
existing `game.actors[actor_id]` state. On a blocked dynamic-actor movement
step, it checks the dominant axis, asks the host whether direct and parallel
segments are clear, then steers through the selected corridor. It advances
the waypoint index as the actor enters each waypoint cell and measures the
live target again when the route ends.

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

The corridor search's side order and host sampling remain [H]. They are a
readable native policy informed by the source route, not a claim of matching
the source's internal algorithm or exact waypoint sequence. The referenced
`/workspace/shadowrun/out/p49b/repro.txt` route capture is absent from this
checkout; local obstruction-grid snapshots do not establish enemy route
fidelity.

## Validation

- `ai_think_tests.lua`: synthetic horizontal and vertical obstructions, clear
  direct path, sealed corridor, opposite-side retry, target-linked route state,
  waypoint advancement, and target reacquisition.
- `shadowrun_gameplay_script_runtime_qa`: verifies the Lua planner reaches the
  typed `ScriptMovementPathQuery` callback through the C++ binding.
- The complete `run_lua_suites.sh` run passes; `ai_think_tests.lua` reports
  315/315 and `entity_motion_tests.lua` reports 13/13. Its separate
  `equiv_heartbeat` output is informational and continues to report the
  documented pre-split mismatches.
- Debug and Release Qt builds and focused runtime CTest pass with headless Qt
  and dummy audio.
- No scene `actual.bmp` was produced for this shared movement mechanism, so
  the obstruction-routing hill remains partial.
