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
The default-map NPC profile still does not receive the general branch tree's
complete `+0x10`, `+0x11`, `+0x4C`, `+0x9E`, and route state. The temporary
random-world actor path now carries typed `+0x10` service flags and the
resolved `+0x11` render/selection flags into Lua, plus the ROM entry delay and
the creature's directional C0B2 frame count. Remaining ordered template
overrides are retained as source data but are not yet all interpreted by the
runtime. The player
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

The actor-service pass scans 16 records, but the ordinary template-list
allocator does not offer 16 default NPC slots. `0x43B6` prepares any pending
list, drains `FFE124` and `FFE120`, then processes the scene default list.
Each row at `0x4466` asks `0x1A342` to scan seven records from `FF0400` at a
`0x100` stride; the returned list indices are 3 through 9. If no record is
free, the list ends immediately. The paired effect allocator at `0x1A2CC`
can also stop the list. The Qt default-list projection now retains those
indices as typed `FieldWorldActorSlot` identities and caps the current
scene-`0x14` route at seven active rows. `0xA800` pending-list occupancy and
effect-slot exhaustion are not part of that projection, so it is not a
general-purpose replacement for the ROM entity pool.

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

### Creature continuation movement vectors

The target decision and movement step have different cadences and inputs.
`0x13904` refreshes the target policy every 21 actor-service passes, while
`0xA8EA` calls the actor's `+0x3A` continuation on every pass. For kinds other
than `0xD`, `0xB574` approaches the accepted heading and chooses one of the
source displacement tables from actor flags and `+0xC8`, the destination
distance written by `0x145F2`. Direct reads from the ROM give these eight-way
step sizes:

| `+0xC8` / actor condition | ROM table | Per-axis step |
| --- | --- | ---: |
| `< 0x100` | `0xBDD2` | 8 |
| `< 0x200` | `0xBDB2` | 10 |
| `< 0x360` | `0xBD92` | 15 |
| `< 0x460` | `0xBD72` | 20 |
| Far, ordinary record (`+0x10=0x25`, `+0xB0=0`, `+0x9E=0x40`) | `0xBEA2` | 18 |
| Far, high-effect route (`+0xB0.7`) | `0xBEC2` | 30 |
| Far, world-actor target pool (`+0x10.3`) | `0xBFE2` | 31 |
| Far, transition bit (`+0x9E.5`) | `0xBE82` / `0xBE62` | 22 / 10, selected by `FFDF84` |

Kind `0xD` uses a separate frame gate: frame `+0x1E == 2` sets `+0x08=-20`
and moves with `0xBE42` (36-unit vectors); other frames set `+0x08=0` and use
the zero-vector table `0xBE22`. The common record bytes at ROM `0x7361A`
initialize `+0x08` to zero. The connected Lua actor heartbeat now keeps the
21-pass target refresh separate from per-pass continuation movement, selects
these vectors from the typed actor flags and target distance, and preserves
that common `+0x08` value. B selection's `+0x11.4` latch is also kept out of
the `+0x10` service byte, which previously corrupted the actor's active flags.

This is a partial movement port. The random-encounter records currently have
the ordinary common flags, so their far-range vector and four near-range
bands are resolved. The global `FFE168` special table branch, complete
template overrides, kind-`0xD` animation frame cadence, close-range effect
allocation, item/spell attacks, hit resolution, and damage/death are not yet
connected. The host movement query still shares the existing collision/route
service; it does not choose a creature target or movement vector.

### Target candidate contract

`0x13874` clears the shared candidate gate. When `FFDFE8` is nonzero, it builds
both lists. Otherwise, the low six bits of `FFDF88` select the seven-entry
list for `0x0B`, the three-entry list for `0x0F`, or neither list for other
values. `0x1438C` scans seven records beginning at
`FF0400`; it keeps records that are active, not effect-locked or transitioning,
not on the high-effect route, clear of status bits 5 and 3, and whose `+0x0C`
word is nonzero. `0x14446` scans only the three records beginning at
`FF0100`; it keeps records that are active, not effect-locked or transitioning,
and whose `+0x0C` word is nonzero. Keep the `+0x0C` predicate as a source fact
until its wider lifecycle meaning is traced; the candidate scan alone does not
justify naming it a collision or alive flag.

`0x14492` uses the seven-entry list when the acting record's `+0x10` bit 3 is
set; otherwise it uses the three-entry list. If its current `+0xA2` link still
belongs to that pool, the ROM keeps it. If the link is no longer eligible, it
selects an index with the source PRNG. Switching to a different record in the
three-entry pool loads `+0xB5` with 6; the seven-entry path does not load that
delay here. An empty pool clears `+0xA2`; the seven-entry path can also reset
service records when `FFE162` is clear. The downstream `0x13FFE` caller only
falls back to the controlled player for its flagged fallback branch. These
rules explain why proximity alone cannot select a target and why a default
map profile is not enough to authorize an NPC attack.

The converted-source cross-check is split across
`entities/entity_animation_link_conversion.cpp` and
`entities/entity_action_candidate_conversion.cpp`; their named tests cover
filters, target retention, replacement, and empty pools. The live Qt field path
now gathers typed facts for active party runners and world actors, applies the
ROM `+0x0C` projected-Y gate without adding a viewport-clipping rule, and lets
Lua retain/select a target with the source bounded-random service. This is
connected for the temporary random-world actor path; default-profile NPC target
links and the full sixteen-record service loop remain partial. Movement and
collision use the existing typed host queries, while Lua chooses the creature
target heading. Item/spell action selection, attack effects, damage/death, and
retirement after combat are not connected.

The B rising-edge path at `0x1DC4C` now selects a live field actor through the
same typed record service. The service keeps the two source selection fields
distinct: candidate reservation is `+0x10.4`, while B's selected-target latch
is `+0x11.4`. It must not be inferred from the action-link candidate flag.
Target-marker rendering and the special runner-pool mode remain incomplete.

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
16-record population. Temporary random-world creatures now spawn from the
ROM's selected template sequences, use extracted directional entry frames,
wait their initial source delay, and receive Lua behavior/render heartbeats.
Their target acquisition and B selection are connected to typed source flags;
attacks, item/spell actions, hit resolution, damage/death, and actor removal
remain open. Default-profile NPC movement/targeting and complete source
actor-slot dispatch also remain open. This partial slice has a Release build,
but no new CTest or scene dump, so it is not defended and has no visual-fidelity
claim.

The word table at ROM `0xB05E` was read directly for all sixteen kinds. The
random-encounter kinds use these entry events: kind 4=`0x2F`, 5=`0x2E`,
6=`0x58`, 7-9=`0x2E`, 10=`0x58`, 11=`0x2F`, 12=`0x56`, and 13-14=`0x5A`.
Lua now returns the matching cue on the same behavior heartbeat that starts
the directional entry transition. Their table-2 slots are 46, 47, 88, 86,
and 90; the imported audio manifest has no files for those slots, and its
extraction policy omits verified silent/no-op entries. The native audio host
therefore receives the source request but plays no sample; these cues have no
audible duration to reproduce. The source cue request and audible sound are
separate facts.

On the first Lua behavior heartbeat after a dynamic actor record is created,
the script now enters the same `+0xB6` delay/entry branch instead of returning
from a script-only initialization pass. This keeps the source delay count
aligned to actor-service passes and prevents a creature from taking a movement
step before its initial entry transition.

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
