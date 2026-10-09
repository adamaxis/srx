# Genesis actor AI and heartbeat source audit

## ROM and trace basis

Primary source: `input/shadowrun_genesis_us.bin`, 2,097,152 bytes, SHA-256
`92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E`.
ROM file offsets match the 68000 addresses used below. For example, bytes at
`0x0AB4` are `4E B9 00 00 A8 EA`, a direct `JSR 0xA8EA`; bytes at `0x4618`
begin `41 F9 00 FF 01 00`, loading the first record in the actor array.
`export/disassembly.s` is the instruction-level transcription used to follow
the branch targets. Ghidra decompilation names are secondary labels.

## Where the heartbeat runs

The top-level loop at `0x075E` has two field paths:

| Path | ROM call sequence | Meaning |
| --- | --- | --- |
| Field-entry settling | `0x08F0: JSR 0xA8EA`, `0x08F6: JSR 0x3A1C`, `0x08FC: JSR 0x4618`, repeated 30 times before `0x0BD2` waits for VBlank | Thirty actor-service and render passes run before the first wait. They are bootstrap settling passes, not a promise of 30 displayed frames. |
| Ordinary field loop | `0x0AB4: JSR 0xA8EA`, then `0x0ABA: JSR 0x3A1C`, `0x0AC0: JSR 0x4618`, palette and world-frame services | One actor-service pass runs in each ordinary field-loop iteration. Source flags can keep the inner loop iterating, so the reliable cadence unit is a field-loop pass, not a fixed wall-clock 60 Hz interval. |

Both call sites resolve to `0xA8EA`. It begins with `0x13874`, which clears
the shared action-candidate gate and gathers candidate entities according to
the current world mode. Then it visits 16 records starting at `0xFF0100`,
with a `0x100` byte stride. `+0x10` bit 0 gates an active record; inactive
records are skipped. For an active record, the service decrements `+0xDC`
when its signed value is nonnegative, examines the record flags and state at
`+0x10`, `+0x11`, `+0x16`, `+0x4C`, and `+0x9C`, and selects an entry,
action, collision, or completion path. The selected continuation at `+0x3A`
is called once before the record loop advances.

The notable dispatch targets are:

| ROM entry | Role established by the branches |
| --- | --- |
| `0xB330` | Starts an entity behavior transition when the record state reaches the entry branch. |
| `0xACAE` | Services the action path, including shared player/action dispatch and action-table state. |
| `0xAC84` | Enters the entity update coordinator (`0x13904`) or its special `0x138B8` entry path; schedules a collision recheck when the global collision flag is set. |
| `0xAB02` | Completes selected transitions and may dispatch a weighted entity/world activity. |
| `0xB49C` | Runs the per-actor fallback continuation stored at `+0x3A`. |
| `0xB77E` / `0xB7B8` | Probe directional terrain collision and advance movement/mode-transition work. |

This is not an independent AI callback at an arbitrary render frequency.
It is a source-ordered service pass with per-record flags, timers,
continuations, and shared candidate/global gates.

The record initializer at `0x44BC` is essential context for those gates. Its
ROM instructions copy the common record template, write `0x05` to record
`+0x10`, copy the entity kind separately to `+0x0E`, and initialize `+0xB6`
from the source PRNG low nibble (`0..15`; world activity mode `0x37` instead
uses `0x28`). Thus `+0x10` is a service/active flag byte, not the actor kind.
The initializer later applies ordered template overrides, so behavior should
read the resulting record facts rather than derive flags from a package name.
The NPC Lua profile slice receives kind/action identity, initial delay, and
heading; it does not yet receive the remaining `+0x10`, `+0x11`, `+0x4C`,
`+0x9E`, or route fields needed for the general branch tree. The player
heartbeat receives the normalized controller nibble and source heading table.
Lua chooses its target heading, advances the current heading with the `0xB530`
`0x0800`-step rule, and now runs the `0xB77E`/`0xB732`/`0xBADE`/`0xBB86`
movement decision path. It picks the initial candidate, handles preserved
negative collision recovery, resolves aligned route markers, tries the
diagonal alternate, updates movement flags, and resets blocked movement using
the `0xB442` rules. The host exposes collision results (including the raw
`FFDF64` terrain value), route-table facts, and the existing `0xB7B8` map-event
recovery callback as explicit services; it applies Lua's selected position
and C052/C062 animation selector to the field model. A blocked reset selects
idle facing from `+0x2E`; the separate `+0x36` word remains unchanged as in
the ROM. This policy port covers the controlled player only.
The corresponding Lua-owned per-instance state lives at
`game.actors[actor_id]`; it is shared script state rather than a transient
heartbeat result or a C++ behavior-state object.

## AI cadence, targeting, action data, and movement

`0x138B8` and `0x13904` decrement the actor's word timer at `+0xB6`. When it
becomes negative, the code checks the shared `0xFFE175` gate and reloads the
timer with `0x14` before running a behavior branch. Starting from zero, this
means the gated branch runs every 21 service passes. The interval is a count
of source loop passes; it is not a seconds-based timer.

`0x13904` branches on entity kind `+0x0E`, route/activity flags at `+0xB0`,
transition flags at `+0x9E`, collision/state at `+0x0C`, and resource/action
state. It can select a target link at `+0xA2`, dispatch the action state, or
call `0x13D0C` for target-relative motion. `0x13D0C` measures relative
heading/distance, uses item/spell action data when selecting a range, forms a
destination, applies the close-range check at `0x144E8`, and hands off to the
route service at `0x145A2`. Action byte `+0x56` selects item versus spell
tables in `0x14D6A`; `+0x57` identifies the selected action row. Target
candidates are collected at `0x1438C` and `0x14446`; actor links are not
simply the nearest-screen-sprite rule.

Movement is collision-coupled. `0xB77E` builds a directional probe and calls
`0xB48C`; `0xB48C` calls terrain collision `0xE7C8` and stores the collision
result in the actor record. `0xB7B8` then advances or rejects movement and
may enter a map interaction transition. `0xB38A`, the initial transition
continuation, also calls `0xB48C` at the actor's current coordinates. The
collision and movement hand-off therefore have to be exposed as explicit
actor services when the later AI branches move into Lua; a timer-only Lua
heartbeat would not reproduce them.

## Animation and rendering are a separate source pass

`0x4618` loops over the same 16 active records. It projects each record,
calls `0x5FDC`, then calls `0x600E` to resolve the current sprite descriptor.
`0x5FDC` adds the actor's animation increment at `+0x18` to its accumulator at
`+0x1A`, compares against the period at `+0x1C`, and advances the frame
counter at `+0x1E` when that period is reached. `0x600E` reads the animation
list selected by kind `+0x0E` and animation selector `+0x16`, including the
terminal marker and next increment. This work occurs in the render pass; it
does not replace the AI/action update at `0xA8EA`.

The native port should retain that distinction in its scheduler: Lua receives
one behavior heartbeat per source field-service pass and one render heartbeat
per source actor-render pass. The behavior heartbeat owns decisions and timers;
the render heartbeat owns animation frame/cadence. Lua also selects movement
and advances player heading, while the host supplies collision query/route,
position commit, spawn/despawn, named audio playback, and rendering services.
Drawing consumes the script-produced pose without advancing it a second time.
Actor state is keyed by a stable instance ID, not the archetype ID, so two
guards can run independent timers and decisions.

## Port coverage and current boundary

`gameplay/scripts/actor_ai.lua` now keeps independent state per actor instance
and returns an animation snapshot on every Lua heartbeat. The profile-actor
entry branch ports the source's random 0..15 initial delay, 21-pass completion
check, directional C0B2 selection, two-frame `0x0200`/`0x1000` cadence,
terminal hold, and source cue request. The Qt renderer consumes the returned
direction/frame. No C++ actor timer or animation accumulator remains for this
profile-actor entry sequence.

This is a narrow first port, not the Genesis AI system. The current extracted
profile actors cover the 3 scene-2 definitions and 8 scene-`0x14` definitions;
New Game's scene-2 list remains inactive under the observed ROM lifecycle.
They are not yet the complete 16 live entity records. General target
selection, general NPC movement, item/spell action selection, weapon firing,
damage/death state, linked actors, and world activities remain outside the Lua
heartbeat. The player now receives behavior and render heartbeats; Lua resolves
controller/scripted target headings, applies the `0xB530` slew, owns the
player's animation clock, and selects each player movement candidate and
fallback plus the C052/C062 selector. C++ remains responsible for terrain
collision sampling, source route data access, map-event recovery, and applying
Lua's selected coordinates and selector to the field model. The 16-slot
population and projectile animation paths are not yet all connected to these
callbacks. General NPC movement, targeting, combat, and source actor-slot
dispatch remain open; this slice does not establish that every on-screen actor
is connected.

The entry sounds selected by `0xB330` are kind-table events `0x2E` and `0x2F`.
The extracted audio table maps both to silent slots, so the Lua request has no
sample or playback duration to reproduce.

## Implementation review checklist

For every new actor behavior branch:

1. Trace its ROM entry and all direct branch/callback targets.
2. Record the source fields, gates, timers, and pass cadence it reads or writes.
3. Port the policy and per-instance state to the actor's Lua package.
4. Keep host collision, audio, rendering, and spawn operations as explicit
   services; do not put an AI decision back into a C++ timer.
5. Check multiple gameplay frames and neighboring screen composition. Compare
   with independent Genesis footage when that same route is visible; native
   captures alone prove only the conversion path.
