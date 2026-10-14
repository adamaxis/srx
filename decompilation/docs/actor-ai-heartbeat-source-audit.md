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
uses `0x28`). It also copies source template byte `+0x04` to live record
`+0x4C` and consumes template bytes through `+0x49`; ordered list overrides
apply to those live-record offsets after the copy. Thus `+0x10` is a
service/active flag byte, not the actor kind. The offline default and dynamic
actor catalogs had read `+0x4C` from the ROM template instead. That byte is
outside the `0x4A`-byte template payload and can be the next template's flags:
for example, the Hell Hound template at `0x1D6816` has `+0x04 = 0x0A`, while
`+0x4C = 0x20` is the Gargoyle template's palette flag. The catalogs now use
the initializer's actual source byte and honor a live `+0x4C` override. This
corrects the actor meter used by encounter state and collision eligibility.
Behavior should read the resulting record facts rather than derive flags from
a package name.
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

The connected Lua motion path uses the item-template `+0x27` profile from
`0xD4540` or spell-template selector from `0xD4EA0` while the source
re-target delay is nonzero. The four finite bounds at `0x13E7C` are
`0x0500..0x0640`, `0x05A0..0x0780`, `0x0640..0x08C0`, and
`0x08C0..0x0C80`. A distance in a selected band clears the delay and builds a
lateral destination at heading `+0x4000` and twice the target distance; a
range at or beyond the upper bound uses the linked position. The `+0xB5`
delay is six route updates, not six actor-service passes: `0x13904` reaches
`0x13D0C` every 21 passes, and the first delay decrement occurs in the update
that links the runner. C++ measures direct and lateral route step costs and
destination distances from typed world positions. Lua applies the source
`0x01E0` close-route gate, and carries the chosen destination distance into
`+0xC8` so `0xB574` selects the matching displacement table. Each `+0x3A`
continuation pass slews the current heading toward the stored route heading in
`0x0800` steps. The 50 item templates now expose their source byte, including
Bear Totem item 43's profile 2. Spell entries whose selector is `0xFF` remain
outside the four finite profiles.

The encounter path review found an inverted branch in the shared heading
conversion used by this route. US-ROM bytes at `0x15D4E` select a basis word
from `0x15DF6`; at `0x15DB0`, `BNE` skips `EXG D4,D5` when basis bit 6 is set,
so the exchange occurs only when the bit is clear. The native helper had done
the opposite. That made a positive-X target resolve to heading `0x8000` instead
of the ROM's `0x4000`, sending the actor along a different cardinal vector.
The condition now follows the ROM, and the registered movement-metric CTest
checks all eight compass headings against the extracted direction basis. The
Scene 2 trace now follows the slower source route cadence and turns its
heading over multiple service passes. At 1,200 passes both natural-spawn actors
remain outside the viewport; their long path is not yet corroborated by an
independent Genesis capture, so approach and arrival behavior remain open.

This is still partial: action kind/code are currently the actor's selected
Lua profile values, and attacks/hit outcomes are not implemented. The separate
`+0x10.3` world-actor route now receives the typed live posture fact that the
ROM reads from actor `+0x4D`: `0x13D84` indexes the nine-pair table at `0x13E58`,
and the script-runtime QA exercises posture 2 in its `0x0640..0x0780` band.
The production Scene 2 Weenr profile uses flags `0x25`, so its captured route
does not activate this `+0x10.3` branch. Debug and Release script-runtime,
encounter, and Hound-effect QA pass; the 12 Scene 2 and 8 Hound full-frame
captures match byte for byte across those builds. The Scene 2 review includes
the 1,200-pass frame, where the creatures remain outside the viewport. The
Hound comparison checks the HUD but does not treat
the animating full frame as a static golden or claim a specific map-object
depth order for this effect. These are internal port checks, not independent
Genesis visual proof.

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

The B rising-edge path at `0x1DC4C` now runs its normal/special candidate scan
in the player's Lua behavior heartbeat. The host supplies the typed record
facts and publishes Lua's selected actor ID to the selected record. The scan
keeps the two source selection fields distinct: candidate reservation is
`+0x10.4`, while B's selected-target latch is `+0x11.4`. Lua advances the
`+0xA6` marker countdown on the render heartbeat, selects the four ROM
tile/palette groups, and feeds the transparent PNG corner assets to the field
renderer. The separate automatic target-service call remains on the native
conversion path. The alternate `+0x9C` marker action-byte route and the special
runner-pool mode remain incomplete.

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

### C0A2 target-action pose and cue

After target acceptance, `0xB1DC` sets delay mode 1, selects ordinary cue
`0x4C` or Hell Hound cue `0x55`, derives a directional selector from the
linked-target heading, installs the `0xC0A2` finite list, and resets the
animation accumulator/frame. Actor initialization at `0x1514E` supplies the
`0x1000` period. The importer exports the directional frames and a named
profile; Lua owns the action state, source increment/period cadence, and final
frame hold on the separate `0x4618` render heartbeat. The extracted profiles
have 2 frames at `0x0200` for kinds 4-11 and 14, 1 frame at `0x01F4` for kind
12, and 3 frames at `0x0200` for kind 13.

The ordinary encounter QA observes the connected transition, both finite-list
frames, mode-1 delay, and the `0x4C` then `0x4D`/`0x4E` cue sequence. The Lua
runtime QA exercises both FFF9F8 variants. The full 320x224 snapshots retain
the HUD and map with no black sprite rectangle or stale control. Audio event
`0x4C` now plays extracted table-2 resource 76; the audio QA checks playback
and its 98,511-sample, 44.1 kHz duration (2.234 seconds). Hound cue `0x55`
uses slot 85, absent from the extracted non-silent resource set, so it remains
unmapped. After linked-heading acceptance, `0xB1DC` selects `0x4D` or `0x4E`
from FFF9F8 bit 7. These now play resources 77/78; each contains 93,360 samples
at 44.1 kHz (2.116 seconds), and the audio QA checks both. The Hound
close-effect diversion is separate and source-silent. An
independent same-action Genesis capture is not identified. `0x55C76` still
owns the unresolved combat reaction/hit path; this visual transition does not
claim that damage or actor death is connected.

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
and a focused Release CTest with full-frame captures. The QA selects Scene 2's
sum-3 two-Weenr row using injected bounded selector rolls, so it verifies list
resolution, actor allocation, entry transition, target acquisition, movement,
and the B edge; the production field loop now asks Lua to enforce the ordinary
`0x1E166` block/cadence/chance policy before C++ consumes the source PRNG.
Script-runtime QA covers those gate branches, while the encounter-spawn QA
still injects selector rolls and does not verify the entire random dispatch
end to end. The spawn caller also uses live camera `FFDDC6/FFDDC8` and map
bounds `FFDDCE/FFDDD0`; the sampler normalizes candidates to the same `>>4`
coordinate space as `0x1E41E`. The target-state QA retains the selected actor
after B release and verifies the source path makes no sound request.

The captures show two distinct visual limits. The ROM edge-spawn coordinates
are beyond the active viewport, so `encounter-entry.bmp` and the later
production frames contain no creature pixels. A separate QA-only close-range
fixture moves the runner next to an encounter actor and captures two distinct
C062 walk poses while preserving the HUD. For the selector pixel capture, the
test repositions the already-selected source record beside the runner after
leaving the production spawn frames untouched. It captures both the shrinking
and settled `0x5C7E` marker, uses alpha-preserving PNG tiles, and verifies
pixels from all four source corner descriptors in the complete frame. The
right-side HUD remains unchanged. Independent section-06 footage at frames
002190/002220 and 002430/002460 shows yellow and red corner brackets around
combatants, corroborating the shape but not the exact actor/action-to-palette
mapping. Keep this hill partial: production random-dispatch capture, the
alternate `+0x9C` selector data route, complete actor pool, natural approach,
and creature combat resolution remain open.

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

## Current Release QA snapshot

`shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa` writes
`initial.bmp`, `encounter-entry.bmp`, `encounter-after-30.bmp`,
`encounter-after-180.bmp`, `encounter-after-480.bmp`,
`encounter-after-1200.bmp`,
`encounter-visible-qa-fixture.bmp`, `creature-walk-frame-0.bmp`,
`creature-walk-frame-3.bmp`, `target-selection-b.bmp`,
`target-selection-b-static.bmp`, and `actual.bmp` under
`build/conversion/qt/release/qt_runtime/field-random-encounter-spawn-qa/`.
All 12 full 320x224 frames were reopened. Production edge-spawn frames at
30, 180, 480, and 1,200 service passes keep the same map/HUD composition;
both creatures remain outside the viewport, and their long path still needs
independent ROM footage to confirm approach behavior. The controlled
fixture shows the two creatures and distinct walk poses. The marker fixture
checks the four source tile corners at their projected positions and leaves
the right-side HUD unchanged. B retains the typed target, and the source path
makes no sound request. Entry cues are checked as requests (`0x2E` for this
sequence), not as audible samples or durations. Section-06 longplay frames
2190/2220 and 2430/2460 independently show the target-bracket visual form; they
do not expose the source actor/action bytes for this exact scene.

## Creature target acceptance and route handoff

The ROM's `0x13FFE` dispatcher calls `0x14C7C` after selecting an actor's
linked target. A rejected target leaves the actor in route mode 2. An accepted
target copies the actor's current coordinates to `+0x96/+0x98`, then selects
mode 1 or 3 from signed `+0xDC`; this exits pursuit and hands the record to
its action continuation. Lua now reports that outcome as
`ai_target_engaged`, preserves the selected `+0x34` heading, and returns no
movement step for that pass. The Qt runtime accepts this typed engaged state
without fabricating a collision-resolved move. Lua now plays the source-backed
C0A2 pose and requests the target-action cue on the `0xB1DC` transition, but
the following `0x55C76` combat reaction remains unconnected. Engagement and
the visible action pose therefore do not mean a completed attack or hit.

The accepted ranges are sourced from `0x14C7C`, `0x14D58`, `0x15C58`, and
`0x15C84`. Ordinary kinds accept an active, unlocked linked actor only below
`0x200`; their branch writes `+0x34` and leaves `+0x2E` to the action
continuation. Hell Hounds accept below `0x300` when `+0xDC` is nonnegative,
or after a free effect slot is allocated when it is negative; the latter
sets both headings. They also set both headings from `0x300` through `0x5FF`,
and reject at `0x600` and beyond. Gargoyles and Wendigos accept the linked
actor's measured coordinate distance below `0x200` and set both headings.
Gargoyle animation frame 2 instead writes `0x1000` to `+0x1A`, clears carry,
and continues routing for that service pass.

Although the Gargoyle/Wendigo branch calls `0x14C56`, the range helper reached
through `0x15C84` measures the linked record's actual coordinates; the
projection result is not used as that threshold. The host no longer computes
or passes a projected-target distance for this decision. The script-runtime
QA covers the strict `0x200`/`0x300`/`0x600` boundaries, the `0x5FF` Hound edge,
the effect-slot path, and the Gargoyle frame-2 carry behavior in Debug and
Release.

## Hell Hound close-range effect

The ROM branch at `0x14C7C` separates heading from attack selection by range.
At distance `>= 0x600`, it leaves the target heading alone. From `0x300` to
`0x5FF`, it writes current and target heading. Below `0x300`, a nonnegative
signed actor timer at `+0xDC` only updates the target heading. Once that timer
is negative, the branch asks `0x1A2CC` for the first free shared effect slot;
with all 18 slots occupied it exits without beginning the effect. On success,
it records the allocated slot and sets the actor effect-lock bit at `+0x11.6`.
The close branch does not request a sound.

`0xA8EA` decrements signed `+0xDC` once per active actor behavior-service
pass while the timer is nonnegative. The action-delay path at `0xABFC` calls
`0xD7EA`: the first enabled `+0x90` bits 12-14 contribute 4, 8, or 12; the
wrapped average of `+0x78` and `+0x7B`, minus one, contributes the remaining
factor; the factor is floored at zero. The final delay is `max(0x6A - 4*factor,
0x0F)`. The Hell Hound template at `0x1D6816` supplies `+0x2A=0x0806`,
`+0x2D=0x02`, and no capability bits in `+0x44`; the live `+0x78/+0x7B`
values are therefore 8 and 2, producing a 90 behavior-pass delay.

The effect renderer at `0x4E86` is separate from the behavior branch. Its
signed timer advances one phase every seven calls. Phases 1, 2, and 3 each
append one directional effect sprite for seven render passes. The next call
enters phase 4, clears the effect slot and actor lock, and appends no sprite;
there is no sound request in this path either. ROM effect descriptors and the
shared effect palette are extracted offline to individually named host art:
56 field scenes x 8 headings x 3 phases. Lua owns the slot request, action
delay, effect phase, heading, and visibility state. The Qt field runtime
reserves/releases the shared slot and draws the named frame at the actor's
projected location.

The focused Scene 0 QA uses the real sum-zero Hell Hound/vampire sequence, then
places one live Hound within `0x200` of the runner to expose the effect on
screen. That placement is diagnostic, not a natural approach replay. Its
full-screen frame set checks the pre-encounter map, sequence entry, pre-effect
state, effect start, phases 1-3, and release. At this placement the plume
overlaps nearby field artwork. The QA verifies the full map composition and
right-side HUD, but does not claim that an independent source image confirms
the exact map-object depth order. The Hound sprite keeps animating, so the
full-frame phase-3 comparison does not claim that the whole phase matches
release or prove that no stale pixels remain after release; the effect-slot
state clears on phase 4. The audio request stays empty. No reviewed longplay
frame sequence
can be confidently tied to this exact Hound attack, so these captures verify
source-driven state and internal composition, not independent Genesis visual
fidelity. This branch is connected; general actor-slot dispatch, other
creature attacks, hit/damage/death, and other effect families remain open.
