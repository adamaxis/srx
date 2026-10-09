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

The connected weapon-projectile animation path preserves this split. The Lua
`weapon_projectile` profile receives each active primary and secondary record
during the `0x4618` render callback, with its selector, finite-list length,
frame, accumulator, increment, and period. Lua applies the signed-word
`0x5FDC` cadence and `0x600E` list bound, returning the frame and completion
bit. The next `0xA8EA` pass observes that bit and retires the finite record
before its following render pass. For the regular `0x8003` list, QA records
frame 0 on the shot, frames 1 and 2 on the next two updates, completion marked
while frame 2 remains selected on update 3, and record retirement before
rendering update 4. The special `0x800B` list uses the same `0x270F` increment
and `0x1000` period; signed word overflow in `0x5FDC` holds frames 4 and 9
through their source intervals, with the terminal marker on update 19 and
retirement before update 20. The Lua runtime QA checks this 19-pass list
sequence; the connected field route has only captured the regular family so
far. This is source pass ordering, not a single callback that advances and
retires together.

## Projectile behavior is a separate Lua heartbeat

The ROM behavior pass at `0x14E0A` checks the stopped bit at `+0x11.3`. A
moving primary runs `0x14F50` four times per field update (`DBF` from 3), and
each attempt enters `0x14F26` before the collision scan. `0x14F26` forms
signed 8.8 probes from fixed-point world position `+0x00/+0x04` and signed
offsets `+0x4C/+0x4E`; `0x14F50` samples terrain, reads the boundary result
from `FFDFE2`, decrements the byte retry counter at `+0x89`, commits the
probe, then calls `0x15ABA`. Lua owns this ordering, wrapping arithmetic,
four-attempt cadence, and retry decision. Native callbacks return the current
terrain sample and typed entity-scan result; they do not select the next state.

Terrain rejection or retry exhaustion branches to `0x14E32`: set `+0x11` bits
0 and 3, clear selector `+0x16` and offsets `+0x4C/+0x4E`, and request audio
event `0x3C` when `D1` is zero. `FFDF68` independently clears the active bit
at `+0x10.0`; the audio request still occurs when that marker is set. Lua QA
covers both outside termination and retry wrap to `0xFF`. A successful entity
hit returns source carry with the target position copied to the projectile
record and `D1` cleared; the sound is requested, then the unresolved linked
actor/status path remains a boundary. Other source-carry exits retain `D1` and
enter the same linked path without the hit sound.

When the stopped record sees the preceding render pass's finite-list bit,
`0x14F86` decrements signed byte `+0x41`; a negative result clears the active
bit, while a nonnegative result begins linked action setup and derives new
heading offsets. Lua owns the countdown and retirement choice; the linked
record resolver is not connected. The secondary `0xAC4E` path only waits for
its finite-list marker and then clears its active bit. Lua owns that retirement
decision too. The active scene currently supplies no live collision candidates,
so no NPC hit or damage behavior is fabricated.

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
Lua's selected coordinates and selector to the field model. Connected weapon
primary and secondary records now receive Lua behavior and render heartbeats.
Lua owns four-probe primary movement, retry and terminal flags, outside-bound
retirement, the `0x3C` request, the stopped-record countdown, and secondary
finite-list retirement. C++ supplies terrain/entity scan results, applies the
typed record snapshot, and plays requested audio. The scene has no live actor
candidates, so target hit, linked status/damage, and `0x14F86` follow-up stay
explicit boundaries. The runtime still does not enumerate the complete
16-record population. General NPC movement, targeting, combat, and source
actor-slot dispatch remain open; this slice does not establish that every
on-screen actor is connected.

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
