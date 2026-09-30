# Shadowrun Genesis Ghidra scratchpad

This directory is the single checked-in Ghidra decompilation export for the US
Genesis ROM. It is source-analysis evidence. Native code uses typed host state
and direct assets; it does not execute this decompilation or perform ROM/RAM
lookups at runtime.

## Regeneration record

- ROM: `../input/shadowrun_genesis_us.bin`
- ROM SHA-256: `92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E`
- Ghidra: 11.4.2
- Loader: patched `lab313ru/ghidra_sega_ldr`, upstream commit `aef1046`
- Language: `68000:BE:32:default` / compiler `default`
- Exported functions: 906
- Resolved direct call sites: 2,978
- Unresolved or indirect call sites: 16

Semantic names in `functions.tsv`, `callgraph.tsv`, `call-sites.tsv`,
`disassembly.s`, and the generated C files come from the reviewed-name table in
`../tools/ghidra/ExportShadowrun.java`. The current pass has reviewed all 906
exported functions; no generated `FUN_<address>` names remain. The six
hardware-vector entries and the six largest/previously native-owned entries
are named explicitly as vector, renderer, terrain, menu, or save boundaries.

The reviewed startup-menu trace is modeled in the separate conversion tree:
`0x59092` owns New Game / Load Game, `0x59172` owns Save Slot 1 / Save Slot 2,
and `0x59248` owns Joshua's Samurai / Decker / Gator Shaman choices. Their
typed state and source callback order live in
`ghidra_conversion/menus/startup_menu_conversion`; the conversion-runtime
handoff that composes those contracts lives separately in
`ghidra_conversion/integration/startup_integration_conversion`. This is not
native-port runtime wiring and does not replace the native port's presentation
code.

The startup record inputs are now a separate direct-asset slice. The offline
`shadowrun_ghidra_startup_record_extract` tool copies the exact source ranges
used by these routines into `assets/records/startup/`:

| Source range | Direct asset | Consumer |
| --- | --- | --- |
| `0x7291A..0x72C19` | `records/startup/joshua-archetypes.bin` | `0x42B0`, three selectable Joshua templates |
| `0x72C1A..0x73719` | `records/startup/recruited-runner-records.bin` | `0x59318`, twelve compact save-roster inputs |
| `0x59972..0x59985` | `records/startup/save-roster-header.bin` | `0x59318`, fixed save-work-buffer header |

The conversion-side loader combines those named files into typed fixed-size
records. `enter_start_options_integration()` performs the source-order
`0x59318` roster initialization before dispatching `0x59092`; selecting New
Game then runs the converted `0x42B0` copy and publishes the selected live
Joshua record plus the three source selection markers. Missing or malformed
record assets are reported as an incomplete conversion rather than replaced
with native-port state or guessed defaults.

The adjacent load-restoration span is also source-mapped before it is exposed
to a runtime caller. `0x59360` first normalizes the active SRAM pair through
`0x59456`; only when that succeeds does `0x59372` validate the two work-buffer
copies. `0x59372` has four observable outcomes: keep a valid primary and
repair an invalid secondary, keep both valid copies, copy a valid secondary
back into the primary, or return with no save. There is no ROM-side fallback
callback after both copies fail, so the conversion does not invent one.

When `0x594E0` receives a validated work buffer, its source order is:

| Source write | Meaning established by consumers | Conversion field |
| --- | --- | --- |
| `FFF9FA..+0x247` | copied saved field payload | `SaveFieldRestoreState::work_payload` |
| `FFDDC4` | world/field mode word | `world_mode` |
| `FFDDC6`, `FFDDC8` | field camera/origin words | `camera_x`, `camera_y` |
| `FF0100/04`, `FF0200/04`, `FF0300/04` | Joshua and two recruited-runner positions | `runner_positions[0..2]` |
| `FFFC0C`, `FFFC0D` | signed recruited-runner template markers | `RunnerRosterState` markers |
| `FF0110`, `FF0210`, `FF0310` | field/entity flags and recruited-runner effect flags | typed restore flags |
| `FF0150` | field profile pointer selected by `FF019B` (2, 3, or default) | `FieldProfileSelection` |

The direct callees `0x03074`, `0x042F2`, `0x0431C`, `0x5966E`, and `0x1E4AC`
remain explicit conversion boundaries. A successful menu callback must not be
treated as a completed load until this restoration path has a validated host
save image and the source-derived roster templates.

## Mode-2 map descriptor and collision records

The field startup record sweep now includes the source data that was missing
from the first field-session pass. The pointer-table row at `0x323A` resolves
mode 2 to the `0x172C80` descriptor. Its fields are not a generic blob:

| Descriptor field | Source meaning | US-ROM value |
| --- | --- | --- |
| `+0x00/+0x02` | initial mode-2 camera fixed-point pair | `0x3400, 0x2280` |
| `+0x0C/+0x0E` | terrain X/Y exclusive upper bounds | `0x0754, 0x0518` |
| `+0x10/+0x12` | terrain X/Y inclusive lower bounds | `0x002C, 0x002C` |
| `+0x26` | pointer to five-word terrain collision records | `0x172DAA` |
| `+0x42..+0x4C` | three initial field-record position pairs | `0x3CE0/0x2D00`, `0x3BA0/0x2D00`, `0x3DA0/0x2D00` |
| `+0x4E` | pointer to the 32-byte field event-mode table | `0x172CDC` |

`0x0EE10` is the consumer of the collision record stream. It clears the
64-by-64 status map, reads five words per record, rasterizes inclusive
`0x20`-unit X/Y steps, and stops when the first word has its signed high bit
set. Mode 2 contains 101 records from `0x172DAA` through `0x17319B`; the
`0xFFFF` sentinel is at `0x17319C`. The direct asset intentionally contains
the 101 records without the sentinel, and the conversion loader rejects an
early sentinel or changed record count instead of silently accepting a
truncated table.

The event table is consumed indirectly. `0x03074`/`0x03084` select and copy
the 48-word scene profile and its two `0xE10`-word terrain layers; this map
descriptor is not the startup descriptor for the currently selected normal
profile. `0x0BC36` builds the four
collision-probe points from the active entity and snapshots the current field
origin into `FFDF50/FFDF52`. `0x02F32` later uses that live snapshot when
initializing field-scene state and copying entity anchors. This is why the
conversion keeps the row-2 descriptor camera (`0x3400/0x2280`) separate from
the currently selected row-0 scene-profile descriptor (`0x0A00/0x0280`)
instead of forcing one into the other.

The offline extractor writes the three direct records under
`assets/records/field/` with aliases
`records.field.mode.2.descriptor`,
`records.field.mode.2.terrain.collision.rectangles`, and
`records.field.mode.2.event.table`. The conversion loader decodes them into
`FieldSceneSourceMetadata`, including the typed rectangle list and the host
collision-map projection. This metadata is now carried through
`FieldSceneBufferInput`/`FieldSceneBuffers`. The current field-mode-2 startup
input intentionally carries the row-0 descriptor from
`scene-profile-normal.bin` plus its 65 records at `0x0D55DE` and event table at
`0x0D54B2`; the row-2 data remains available for a later map-selection path.
Movement probes and dynamic entity presentation remain the next source-backed
consumers rather than being guessed in the Qt bridge.

The startup distinction is explicit in the conversion source. The normal
profile selected by `0x03084` is the first pointer-table row at `0x3232`, and
its descriptor fields are `0x0A00/0x0280` camera, bounds `0x0770/0x03BC`, and
initial runner positions `0x1310/0x0980`, `0x1220/0x08C0`, and
`0x1400/0x08C0`. Its collision records are the 65 records at `0x0D55DE`,
terminated by the sentinel at `0x0D5868`; its 32-byte event table is at
`0x0D54B2`. These records are separate direct assets, not a second copy of
the 48-word normal profile.

## Field movement direction table

The field movement entry at `0x0BABC` derives an eight-way ordinal from the
active heading, then passes the doubled ordinal into `0x0BADE`. The probe at
`0x0B77E` uses that ordinal to read one signed `(x,y)` pair from the source
table at `0x0C002`, adds the pair to the active entity position, and calls the
reviewed collision leaf at `0x0B48C`. The US-ROM table is:

| Direction ordinal | X offset | Y offset | Source bytes |
| ---: | ---: | ---: | --- |
| 0 | `0` | `-40` | `00 00 FF D8` |
| 1 | `40` | `-40` | `00 28 FF D8` |
| 2 | `40` | `0` | `00 28 00 00` |
| 3 | `40` | `40` | `00 28 00 28` |
| 4 | `0` | `40` | `00 00 00 28` |
| 5 | `-40` | `40` | `FF D8 00 28` |
| 6 | `-40` | `0` | `FF D8 00 00` |
| 7 | `-40` | `-40` | `FF D8 FF D8` |

The offline graphics/resource extractor now copies exactly these 32 bytes to
`assets/records/field/entity-movement-direction-table.bin`, exposed through
the alias `records.field.entity.movement.direction.table`. The conversion
loader decodes them into `MovementOffset[8]`; the separate Qt conversion
runtime now uses that typed table when it wires field input to the already
converted `probe_entity_directional_collision()` and
`advance_entity_movement_step()`.
No runtime path reads `0x0C002` or any other ROM address.

The controller side of the same path is also a source record, not a host
guess. `0x195C` masks the sampled controller word to its D-pad nibble and
indexes the sixteen-word table at `0x19D2`; `0xFFFF` entries reject neutral,
opposing, or unsupported combinations. The valid US-ROM mappings are:

| D-pad nibble | Target heading | Movement ordinal |
| ---: | ---: | ---: |
| `0x1` | `0x0000` | `0` |
| `0x2` | `0x8000` | `4` |
| `0x4` | `0xC000` | `6` |
| `0x5` | `0xE000` | `7` |
| `0x6` | `0xA000` | `5` |
| `0x8` | `0x4000` | `2` |
| `0x9` | `0x2000` | `1` |
| `0xA` | `0x6000` | `3` |

The offline extractor writes the complete table to
`assets/records/field/entity-controller-heading-table.bin`, exposed as
`records.field.entity.controller.heading.table`. The conversion helper keeps
the source `0xBABC` rotate/add-one rule in
`field_movement_input_conversion`, returning both the movement ordinal and
the doubled source slot used by `0xBADE`/`0xB77E`.

## Converted field input/movement handoff

The separate Qt conversion composition layer now consumes both named movement
records. Its field-ready path is:

`held Qt D-pad -> 0x195C/0x19D2 target heading -> 0xB530 heading slew ->
0x0BABC movement ordinal -> 0xBADE step -> 0xB77E vector probe -> 0xB48C
collision-record wrapper -> 0xE7C8 terrain result -> typed position commit ->
0x5A62 projection`.

This is a conversion-runtime connection only. It is under
`ghidra_conversion/qt_runtime/` and has no dependency on the native-port or
GenesisBus targets. The host input layer maintains a four-bit D-pad mask so
the source diagonal entries (`0x5`, `0x6`, `0x9`, and `0xA`) remain available;
neutral and unsupported table entries remain invalid just as in the source.
The source vectors are not duplicated in the Qt layer: the loader supplies
the typed `MovementOffset[8]` array from the direct record.

The current startup field input uses the row-0 normal-profile metadata already
selected by the field-entry conversion. Terrain collision is evaluated from
that metadata's typed `0x1000` collision map and bounds. A positive terrain
code reaches `0xE9EE` by using the same row-`terrain_code` five-word record
stream that `0xEE10` rasterizes; the conversion now supplies its left/right and
top/bottom words to the source's +/-12 and +/-7 comparison. The aligned
blocked-step route table is likewise now extracted from `0xBBF6` as four rows
of collision-sample indexes and fallback direction slots, so the Qt bridge
passes those records through `evaluate_entity_movement_route` instead of
failing closed at an invented boundary.

The `0xE7C8` conversion now also carries the four source sample bytes that
`0xB48C` copies into the entity collision record. Expanded probes replace all
four bytes; center probes replace only `FFEF10` and preserve `FFEF11..13`, and
the outside-bounds path publishes the source `00 7F 00 7F` pair. The Qt bridge
keeps that typed sample state between candidate probes instead of flattening
it into a fresh synthetic record on every call.

Field camera follow is ROM `0x3A1C`. The ordinary path looks `0x3C0` ahead of
the entity heading, compares that point with the camera plus `0x880`/`0x700`,
and steps by one or two coordinate units with the `0x180`/`0x300` hysteresis.
`0x3C36` shifts the step by four and rejects a sum outside profile words 2
through 5. `FFE186 == 3` instead chases the fixed point `(0x600, 0)` through
the `0x4290` direction table. Dirty layer words and the interaction lock skip
the write. The tile-edge uploads `0x4018` and `0x40E2` stay callbacks.

Field walk/idle sprites follow the ROM selector tables `0xC052` and `0xC072`.
An accepted step publishes the walking selector and copies the list auxiliary
`0x0300` into the `0x5FDC` increment; the template period `0x1000` stays.
`0x600E` keeps the last of the two walk poses until `0xAF6C` restarts the
clock on a selector change. No input returns the idle table.

Field HUD overlays follow the `$F000` nametable writes: `0x35D4` portrait at
`F1C6`, `0x3674` physical bar at `F1C4`, mental bar at `F3C6`, `0x35F0` item
at `F448`, and `0x36FC` ammo at `F4C4`. Slot stride is `0x400` (64px). Only
records with live `+0x10` bit 0 emit overlays, so the new-game composition
keeps companion frames empty.

The separate smoke command
`shadowrun_ghidra_conversion_qt --field-smoke-move-right` verifies the
connected path without opening a window. With the US-ROM startup records it
advances Joshua from `0x1310/0x0980` (`4880,2432`) to `4920,2432`, matching
the extracted right vector. This proves the typed handoff and host execution,
not full Genesis fidelity; the field hill remains partial until the missing
route/object/event/animation consumers are source-traced and independently
verified.

The current blocked-probe bridge now carries the source `0x0BC36` four-bank
collision offsets into the converted `0x0B7B8` transition routine. The Qt
composition keeps the entity `+0x10` behavior byte (`0x05` from template
initializer `0x044BC`) separate from the movement record's `+0xB0` route flags;
the latter starts at the extracted template's zero value rather than being
invented from the behavior byte. `0x0B7B8` also preserves the source write
boundary for `FFFBF9`: only the ordinary fall-through path publishes the
mode-event byte, while the early `0x1A`, `0x18`, and `0x1B..0x1F` activity
branches leave the previous value intact. The `0x1E2E0/0x1E2EC` host bridge
therefore records only the typed activity continuation and timer start; the
event-table lookup remains the separate `0x167AE` conversion and is not
pretended to be a return value from the timer helper. The active-entity
selection path `0x0C7F8 -> 0x0C802 -> 0x0391A`, which supplies the live
`+0x10` bit-1 state required for the negative-collision branch, is now
connected in the separate Qt composition through the typed `RunnerSwitchState`:
source types `0, 1, 2` are installed, Joshua starts active in slot zero, and
the converted `0x391A` walk publishes the bit-1 marker even when the recruited
primary/secondary slots are not active. The Qt callback also runs the already
converted `0xC64C` finalizer, whose `0xC802` refresh remains typed and bounded.
The recruited roster-selection path is still unresolved, so the composition
does not invent active companions. `0x1651E` now runs from the blocked-movement
callback. `0x167AE` publishes FFDF40 as mode 1, 2, or 3 from the scene id
(`< 0x15`, `< 0x2C`, `< 0x38`); scene `>= 0x38` stays the source `stop`.
The Qt bootstrap now returns that stop instead of writing world mode 0.
Carry is set for mode 0 and for a zero action word, and clear for every
requested action in a nonzero mode. Word writes use the two `0xE10`-word
layers `0x03074` copies to FF2700 and FF4700. Scene `0x14` writes the traced
ROM words `0x03AD`/`0x03AE` from `0x1CD296`. Mode 0 still installs an auxiliary
slot in the ten-record FFD740 table when `+0xC6` matches `0xC0` and a leading
word is free. The marker clear uses the FF1700 collision byte selected by
`0xE8A0` (`FFDFE4`). `0xE9EE` can leave A2 on the object table, but that index
is only defined for the collision map, so the host clears the map byte.
Scene 4 with low bits 6 publishes the eight ROM words at `0x1667A` as the
pending profile `0x2B34` consumes on the next scene entry. The empty-pointer
branch copies profile words 33 through 38 (FFDE08) into the three entity
slots and leaves the camera where `0x03074` put it. Slots 1 and 2 are stored
with the field records; the recruited roster path still decides whether they
are active.

### Field scene runtime dispatcher: 0xC7F8

The direct disassembly makes `0xC7F8` a small but important dispatcher rather
than a generic scene hook. It tests the typed field-action counter that was
`FF014C` in the ROM. A nonzero counter enters `0xC802`
(`refresh_field_runner_state_and_assets`); a zero counter branches to `0xC64C`
(`finalize_field_mode_exit_transition`). The conversion now exposes this as
`prepare_field_scene_runtime()` and invokes it from the `0x03074` field-buffer
preparation path before the selected profile is copied. The two callees remain
separate contracts: the dispatcher does not duplicate their runner/effect
state, and the Qt composition records which typed path was selected without
introducing a ROM-address lookup.

## Export files

- `functions.tsv`: one row per function and generated C file.
- `callgraph.tsv`: resolved caller/callee edges with call-site counts.
- `call-sites.tsv`: every direct call plus every unresolved/indirect call.
- `disassembly.s`: instructions with resolved call comments.
- `ghidra-c/fn_*.cpp`: individual decompiler output with annotated callees.

## Runner record field access

The live runner record is a 0x100-byte source structure. Direct field access is
now exposed in the separate conversion-only `runner_record_access_conversion`
library. `read_runner_record_fields()` and
`write_runner_record_fields()` preserve the source big-endian layout and only
touch fields with confirmed consumers; the remaining bytes stay opaque.

| Source path | Confirmed field use |
| --- | --- |
| `0x044BC` `initialize_entity_record_from_template` | copies the template and initializes posture, ammunition, equipment, attributes, skills, and archetype/status fields |
| `0x036FC` `advance_entity_asset_meter` | updates weapon ammunition or inventory phase bytes and clears exhausted equipment |
| `0x038AC` `service_entity_ammo` | services the equipped weapon's ammunition/clip state |
| `0x0D82A` `render_attributes_and_skills` | clears the error marker, submits the Attributes/Skills grid, evaluates the selected attribute/skill upgrade (attribute direct cost; skill 2x or ceil-1.5x branches), then renders the selected label, overlay, and Karma `+0x8D`; its fixed heading leaf is `0xDA4A` |
| `0x0DA4A` `render_attributes_skills_heading` | preserves caller registers and submits the fixed encoded Attributes/Skills heading from `0xC1B3B` to `0xCC90` |
| `0x0D01C` `run_attributes_and_skills_menu` | initializes the two-column Attributes/Skills screen, then gives directional input priority before A/B upgrade, C active-runner cycling through `0xD4C8`, and Start field exit |
| `0x0D5DC` `render_attribute_skill_detail_panel` | renders the selected Attributes/Skills detail cell, Essence/Magic special values, and first-match cyberware modifier rows |
| `0x107CE` `adjust_runner_posture` | clamps the five-state posture byte at `+0x4D` |
| `0x11740` `run_inventory_actions` | dispatches inventory cell bytes, phase bytes, equipped slot/item, armor, clips, and action branches |
| `0x11B58` clips case | reads `+0x4E/+0x4F` and performs reload, trade, and distribution actions |
| `0x12CEE` `update_equipment_combat_bars` | selects the equipped item/cyberware branch used for attack/defense bars |
| `0x597EE` / `0x598B0` | projects confirmed live-record fields to and from compact save records |

The concrete field-consuming operations are in the separate
`shadowrun_ghidra_runner_record_operations` library:

| Source entry | Converted operation | Fields exposed |
| --- | --- | --- |
| `0x0D82A` | `read_attributes_and_skills()` | race, archetype, `+0x77..+0x8C` |
| `0x0D5DC`, `0x11950` | `read_cyberware_detail()` | all sixteen bits at `+0x90`; the `0x0D5DC` panel also consumes typed attribute, skill, auxiliary, and modifier fields |
| `0x036FC` | `advance_entity_asset_meter()` | flags, equipped slot/item, ammo, inventory code/phase |
| `0x038AC` | `service_entity_ammo()` | record type, equipped item, ammo; delegates to `0x036FC` |
| `0x107CE` | `adjust_runner_posture()` | posture `+0x4D` |

These are state operations derived from the source routines, not bookkeeping
labels and not emulator hooks. The VDP/menu calls around the original
mutations remain outside the typed operations.

## Save-roster projection audit

The compact roster record is not a generic blob at the conversion boundary:
`0x598B0` and `0x597EE` establish the exact bidirectional field map used by
the save system. The host keeps the source byte order inside the record
containers, but the projection itself is expressed as ordinary range copies:

| Compact save range | Live runner range | Source meaning currently established |
| --- | --- | --- |
| `+0x00..01` | `+0x0E..0F` | live record kind/header word |
| `+0x02..03` | `+0x2C..2D` | source record word consumed by runner logic |
| `+0x04..07` | `+0x4C..4F` | physical/mental and ammunition bytes |
| `+0x08..09` | `+0x56..57` | equipped inventory slot/item |
| `+0x0A..17` | `+0x58..65` | confirmed live record word/longword fields |
| `+0x18..27` | `+0x66..75` | inventory item and phase bytes |
| `+0x28..3F` | `+0x76..8D` | armor, attributes, skills, and auxiliary bytes |
| `+0x40` | `+0x8E` | auxiliary source byte |
| `+0x41..43` | `+0x9A..9C` | species/archetype/mental values |
| `+0x44..45` | `+0x90..91` | cyberware bitfield |
| `+0x46..47` | `+0xA0..A1` | magic-table selector |
| `+0x48` | `+0x8F` | post-population byte |
| `+0x49` | `+0x9D` | magic-force byte |

`0x5966E` always projects compact record zero first, then uses the signed
`FFFC0C` and `FFFC0D` markers as 1-based compact-record indexes for the
primary and secondary live records. `0x59798` performs the same sequence in
reverse when saving. `0x42F2` and `0x431C` install the selected source record,
write their role metadata (`+0x3E`, `+0x20`, `+0x26`), refresh this
population, and finally apply the caller's `+0x8F` byte. The conversion tests
exercise the complete map by projecting a populated record in both directions
and checking that every compact byte round-trips.

## World-effect mode selection

The intro/world-effect selector is now converted as one typed slice rather
than four address-only labels. The direct source trace is:

| ROM entry | Recovered behavior | Typed conversion |
| --- | --- | --- |
| `0x562D4` | Adds the unsigned `FFE169` effect-timer byte to the D4 difficulty word. | `add_world_effect_timer_modifier(base, effect_timer)` |
| `0x562DE` | Adds twice the unsigned `FFE16A` effect-phase byte to the same difficulty word. | `add_world_effect_phase_modifier(base, effect_phase)` |
| `0x56116` | Uses `FFF0A3` as a three-state intro marker. A negative marker performs two thresholded rolls, compares them, optionally advances the HUD transition, enters the menu service, and either initializes or disables world effects. A zero marker starts mode 2 unless variant 1 is already active. | `advance_world_effect_intro_gate()` |
| `0x561E8` | Reads the selected secondary compact record, increments the current animation index when it differs, then routes the signed `+0x13` status into action-count, delayed-effect, or intro-gate behavior. | `select_world_effect_animation_mode()` |

The compact record fields used by this cluster are animation index `+0x10`,
effect status `+0x13`, intro timing `+0x06`, and delayed-effect timing `+0x09`.
The selector's status ranges are source-derived: statuses below 2 enter the
intro gate; 2--4 use the action-count branch; 5--6 use the delayed-effect
branch; 7--10 use the intro gate for the base variant and the delayed-effect
branch for variant 1; 11--16 use the intro gate for the base variant and the
action-count branch for variant 1; 17--20 always enter the intro gate; and
statuses 21 and above use the action-count branch. The action branch records
`FFF0DD` in `FFF0A4`, displays and advances HUD mode 8, and returns 0 or -1.
The delayed branch uses the timer-plus-phase difficulty path, decrements
`FFF0E7` while gated, or subtracts one from the thresholded-roll count, sets
`FFF0E7` to `10-count`, marks `FFF0E6` negative, and returns mode 3. The intro
comparison preserves the source `SUB.B`/`BLE` overflow flags; after a positive
comparison it calls `0x1989A` before honoring a blocked `FFF0B1` exit.

`WorldEffectModeSelectionCallbacks` deliberately leaves random rolls,
selected-record lookup, action-delay calculation, HUD services, menu entry,
and world-mode initialization as typed boundaries. It is conversion/reference
code only; it is not linked into the emulator or native runtime.

The immediate transition resolver at `0x562EC` now preserves the source
`SUB.B`/`BGT`/`BEQ` flags when comparing the prior and current effect counts.
Its HUD-mode callback is a late dependency: equal, mode-one, and comparison
gate exits redraw rows without selecting a HUD mode. The typed test includes
the `0x80 - 0x01` overflow case that would be misrouted by a signed cast.

## Menu selection timing

The three timing helpers called by the menu services are now represented by
one typed conversion slice:

| ROM entry | Caller | Recovered behavior | Typed conversion |
| --- | --- | --- | --- |
| `0x56434` | `0x19E32 run_menu_selection_service` | Clears `FFF0A3`, subtracts the service delta from selected record `+0x0F`, and applies the source overflow-aware `SUB.B`/`BGT` expiry test before decrementing `FFF0CE` by two; a negative rollover resets it to `0x0F` and increments `FF018D`, `FF028D`, and `FF038D`. | `advance_menu_selection_service_timer()` |
| `0x56482` | `0x19EA6 run_menu_selection_variant` | Clears `FFF0A3`, subtracts the variant delta from selected record `+0x10`, clears the selected menu record on signed-byte expiry, then uses the same phase-counter rollover. | `advance_menu_selection_variant_timer()` |
| `0x564D6` | `0x19F18 run_menu_progression_variant` | Halves D1 with a logical shift, clamps the delta to one, subtracts it from selected record `+0x06`, and clamps that timing byte to two. | `reduce_menu_progression_timing()` |

The conversion keeps audio-frame initialization and selected-record clearing
as callbacks. The record vector and phase counters are host state, not a ROM
addressed runtime structure; an unavailable selected record returns an
explicit result instead of asserting or reading arbitrary memory.

## HUD/menu service branches: 0x19D66-0x1A106

The contiguous menu-service family is now source-traced beyond its generated
function names. The canonical disassembly and Ghidra call lists establish the
following concrete behavior:

* `0x19D66` starts with HUD selection code `0x0B`. A signed
  `FFFB30[FFF102]` marker below one is cleared, publishes value `0x0A`,
  renders the selected text, refreshes the eligible HUD grid, and prepares the
  menu offset. A positive marker publishes value `0x06`, merges the selected
  runner's flags, renders progression text, and renders the slot status; it
  does not prepare the offset on that path.
* `0x19DF2` calls the eligibility-nibble reader. When its result is positive,
  the selected compact runner record's byte `+0x05` selects metadata at
  `flags * 0x20`; when it is zero, the metadata leaf receives offset zero.
  The host conversion requires the extracted table/record values explicitly
  instead of using a ROM-address fallback.
* `0x19E32`, `0x19EA6`, and `0x19F18` publish HUD codes `0`, `1`, and `2`.
  Each maps its signed marker to value `7`, `8`, or `9`. The positive branches
  use, respectively, the service timer, the low byte of
  `(marker * 2) + (FFFBBE+0x0F >> 1)`, and the progression-timing reducer;
  the final menu-offset preparation is unconditional in all three routines.
* `0x19F7E` scans the five slot codes from slot four down to slot zero for
  code `7`. It clears that slot's active value, stores
  `calculate_matrix_program_load_steps() * 10` in its offset word, refreshes
  the menu number/grid branch, and then clears the corresponding status row.
* `0x1A09C` and `0x1A106` are distinct selection refreshes, not aliases. The
  first publishes code `9` with value `6` after a positive `FFF0AA` clear (or
  `0x0A` otherwise). The second publishes code `0x0A`, prepares the offset
  for a non-positive marker, and clears the selected record for a positive
  marker before publishing value `0x0A` or `6`.

These paths are represented by typed state and callback contracts in
`field_menu_state_batch_conversion.*` and
`title_effect_animation_batch_conversion.*`, with tests for source constants,
signed branches, callback order, metadata offsets, and the reverse slot scan.
They remain `partial` rather than `defended`: the direct behavior is verified
against `disassembly.s`, but the callers still need to be composed into the
actual menu controller and exercised by a screen-level hill.

## Menu-text variant and countdown handoff: 0x1A01E / 0x1A12E

Two adjacent helpers also had more behavior than their generic wrappers
exposed:

* `0x1A01E` publishes HUD code `8` with value `0x0A`, then calls the menu
  text follow-up. The fixed constants are now part of the typed callback
  contract.
* `0x1A12E` is called with an incoming D1 value. If `FFF0D2` is nonzero, it
  adds the current countdown byte to D1's low byte, sign-extends the result,
  decrements `FFF0D2` as a source byte, and calls `0x19F72` only when the
  post-decrement signed byte is no longer positive. The computed sign-extended
  value is passed to the code-five finalizer; a host countdown wrapper that
  merely decrements and emits a parameterless expiry was not source-faithful.

The exact overload is covered with both the non-expiry and expiry values,
including the negative-countdown byte path. It remains partial until the
code-five finalizer and its callers are composed into the menu controller.

## Title/world runner action gate: 0x1A030 and omitted 0x56050 continuation

The old `select_menu_entry` label was not describing the source behavior. The
routine is called by the title/world transition after the behavior record is
started; it gates the active runner's action result before the scene hands
control to the next behavior:

* A nonzero `FFDF84` publishes HUD code `9` with value `0x0B`; a selected HUD
  mode of `1` or `8` publishes code `9` with value `0x0C`. Both render the
  follow-up text and tail-call `0x1B706`. The source returns with D0 still
  equal to `9` because the preparation routine saves and restores D0.
* Other modes first call `0x55FD8`. That routine preserves the signed HUD-mode
  D0 value. A negative value prepares the menu offset and returns `-1`; a
  nonnegative value enters the previously omitted `0x56050` continuation.
* `0x56050` obtains D6 from `0x19BF6`, clamps the `FFFBBE+0x0A` difficulty to
  two, runs `0x0DBA`, subtracts the qualifying-roll byte from the live
  runner-action result, and clears that live result only when BPL sees a set
  sign bit. The raw post-SUB.B D4 byte is then what `0x1A094` publishes to
  `FFF0AA`; it is not the same value as the possibly-cleared live result.

This is now `resolve_title_world_action_selection()` plus
`resolve_title_world_action_marker()` in the menu-services module. The test
covers the exact branch order, signed D0 preservation, difficulty clamp,
underflow marker, and BPL-only clear. The entries remain `partial` because
the surrounding title/world controller is not yet a defended screen hill.

## Field-action resource selection: 0x575F8 / 0x57610 / 0x57628 / 0x5763C

The four helpers immediately before the field-action purchase routines are a
Matrix-capacity replacement calculation, not generic menu bookkeeping. The
canonical disassembly resolves the register flow that the generated C omits:

* `0x5763C` selects one of the two 0x30-byte tables at `0x1AB276`: mode 6
  selects the first table and every other mode selects the second. `FFF0B8`
  selects a 0x10-byte record, and `0x57628` adds `FFF0D0 * 2` to point at the
  selected word.
* `0x57610` reads the selected word's first byte into D7, loads `FFF0D6` into
  D1, and calls `0x19538`. The resulting weight is therefore
  `FFF0D6 * FFF0D6 * footprint[record_byte]`; the byte is also retained as the
  program index for the following storage lookup.
* `0x575F8` first calls `0x19558` for available Matrix capacity, then keeps
  the `0x57610` weight in D2, calls `0x1952C` for the selected program's
  current usage, and computes `D6 = D6 + D1 - D2` with word arithmetic.

The conversion is `field_action_resource_selection_conversion.*`. It models
the two extracted resource tables as typed byte arrays, returns the selected
`MenuResourceEntry` and source byte together, and returns `std::nullopt` when
the supplied extracted table cannot satisfy the selected record. The outer
result exposes the unadjusted Matrix capacity, selected-program usage, and
wrapped adjusted capacity. It does not retain `A0`, ROM addresses, or a
Genesis memory map.

## Field-action dispatcher routing: 0x56DC4

The large field-action routine is now split at its first meaningful boundary:
the top-level `FFF0B7` result and `FFF0B8` saved-selection tests are converted
in `field_action_selection_dispatch_conversion.*`. The source routing is:

* A negative result selects one of three action-record tables for saved
  selections 0, 1, or 2; selection 3 enters the field-action detail path,
  selection 4 renders an action description, and later selections enter the
  runner-action removal path.
* A zero result sends saved selection 3 to removal. Neutral action acquisition
  is blocked when the active runner's signed `+0x9B` archetype/status byte is
  at least 2; the source writes grid variant B, renders the unavailable
  message, and waits for a controller event. Status bytes 0 and 1 continue
  through the action-selection matrix: selection 1 uses its dedicated action
  table, while selection 0 chooses the low-menu-mode, mode-3, or high-menu-mode
  offer path.
* A positive result of 1 enters the equipment-action branch. Other positive
  results route selections below 3 to resource/quantity selection, selection 3
  to resource-record purchase, selection 5 to quantity purchase, and the
  remaining selections to the field-resource purchase routine.

`run_field_action_selection()` clears the source service-result byte, returns
this typed route, and dispatches through named leaf callbacks: selected action
application, field-action detail purchase, runner-action description, removal,
runner-unavailable handling, Matrix-program installation, resource-record
purchase, quantity purchase, field-resource purchase, or equipment upgrade.
The leaf routines still own their own typed records, funds, runner fields, and
presentation callbacks; the dispatcher no longer exposes one generic
`execute_route` hook that hides which game operation is being invoked.

The `0x578C4` description leaf is now `show_runner_action_description()`. It
refreshes the five-row detail panel, reads the active runner's selected action
slot at `+0x66`, rejects an empty slot, rejects action codes `0x0B` and above,
and for codes 1 through 10 exposes `description_record_index = action_code - 1`
for the `0xD4540` action-description table before rendering the fixed text and
waiting for input. The conversion deliberately leaves text presentation as a
callback while retaining the actual slot bounds and source code range.

The negative-result saved-selection-3 branch at `0x57026` is now
`apply_field_action_detail_purchase()`. It draws the action-detail grid first,
rejects categories 6 and above, checks the selected runner slot's companion
bit for an already-owned category, and applies the source compatibility rule:
action codes below 8 are accepted only for categories 1 or 3, while codes 8
and above reject only category 1. A confirmed purchase deducts `FFF0BE`, sets
the selected category bit in the companion byte, and clears companion bit 3
when category 0 is selected. Category 3 has the additional source gate at
`0x570EC`: companion bit 0 blocks the purchase even when category 3's own bit
is clear. Invalid, owned, incompatible, insufficient-fund, and cancelled paths
are explicit typed outcomes.

The removal branch at `0x56F4A` is `remove_runner_action()`. It refreshes the
detail panel, rejects empty slots and protected action codes `0x29..0x2B`,
shows the refund and confirms it, then adds `FFF0BE` back to funds. When the
removed code equals the active runner suffix marker plus `'+'`, the marker is
cleared; when the removed slot is the active menu slot, the conversion invokes
the typed `clear_menu_slot` boundary. The action and companion bytes are then
cleared, matching the writes performed by `0x1275C`.

The menu-result-1 equipment branch at `0x571B2` is now
`apply_field_equipment_upgrade()`. It resolves the selected category into a
target upgrade bit, including the source scans for category 5 (bits 5..8),
category 6 (bits 9..11), and later categories (bits 12..15). Bit 15 is the
terminal exhausted marker for the later-category scan, not a new target. It rejects an
already-set target, applies the category-2/bit-3 conflict, subtracts the
target's `0x1AA8A0` meter threshold from the runner's `+0x7E/+0x8E` two-digit
meter, preserves the source signed-byte availability test, confirms and
charges the purchase, sets the target bit, and clears the previous bit for a
replacement. For archetype/status values below 2 it also updates `+0x7F` and
the tracked minimum meter entries. The ROM's grid/message/confirmation calls
are explicit callbacks; no Genesis bitfield or address is used at runtime.

The positive-result saved-selection-below-3 branch at `0x5751A` is now
`install_selected_matrix_program_resource()`. Its quantity gate preserves the
source `CMPI.B #9/BPL` wrapped-byte behavior, reads the selected menu-resource
byte as the Matrix program index, checks the preview cost and the signed-word
result from `0x575F8`, confirms the purchase, deducts funds, clears all five
loaded-program selectors, and writes the requested quantity into that
program's availability byte. Invalid quantity/resource records publish
`FFF112 = -1`; insufficient-funds and insufficient-storage messages wait for
the controller event before returning. The
source `0x58D7E` helper is represented by
`reset_field_action_cost_state(std::array<int8_t, 5>&)`, whose only state
effect is the verified `FFFBD8..FFFBDC = 0xFF` reset. This is grouped with
Matrix program conversion because the field menu is mutating the Matrix
program availability model, not a generic resource counter.

The result-zero action branch at `0x56E5A` is now represented by
`purchase_field_action_progression()`. Its preceding screen-preview routine
`0x57D7E` is represented separately by
`prepare_field_action_progression_quote()`, which publishes the typed pending
record slot, action code, and discounted cost consumed by the purchase branch.
The three source offer families are explicit typed tables:
`0x1AAEB0` for saved selections two and above, `0x1AAEF8` for saved
selection one, and the three menu-mode blocks at `0x1AAF40` for saved
selection zero. Saved selections 0, 1, and 2 all enter this application path;
only selection 3 diverts to the separate removal leaf. Each four-byte offer contributes its signed action marker and
base cost; the source byte at offer `+1` is retained as metadata because this
purchase path does not consume it.

The conversion searches a typed runner progression ledger instead of carrying
the `FFF0C2` pointer; its leading word is the source's last DBF-scanned slot
index, so each scan covers `count + 1` records. An existing `(action_code,
level)` record pays `(level + 1) * base_cost`, subject to the signed `+0x7F`
runner limit; a new record pays the offer base cost. The preview preserves the
source heading, action-name, level/cost, limit-message, and glyph-transfer
order. The signed high-bit offer disable, discounted cost publication, and
the `0x56E5A` level-six early return, funds deduction, record-count increment
for a zero-level slot, and level increment are all preserved. The successful
purchase has no second confirmation prompt; insufficient funds writes grid
variant B, renders its message, and waits. Status, disabled-offer, and
missing-record-slot boundaries are typed outcomes/callbacks rather than
generic memory services.

The action-detail text helper at `0x5878E` is now
`select_field_action_detail_text()`. Its source ranges are explicit: codes
below `0x19` use their code, `0x19..0x24` collapse to table entry `0x18`,
`0x25..0x27` continue at entries `0x18..0x1A`, `0x28..0x2A` intentionally
render no detail text, and codes `0x2B..0x2F` select entries `0x1B..0x1F`.
The typed selector rejects anything beyond the 32 extracted pointers at
`0x1AC7FC`; it does not carry a ROM pointer or invent a replacement string.

The caller at `0x586CA` is now represented by
`render_field_action_detail()`. Its input is a typed sequence of the source's
three-byte action records: record byte 0 is the action type, byte 1 is the
detail/resource value consumed by the existing `0x130C0` conversion, and byte
2 is the quantity used by the cost path. A signed high-bit type takes the
unavailable-message path and then the common tail. Type `'E'` takes the
special grid/header/label path and discounts byte 2 directly. All other types
build the existing typed detail plan, select the `0x5878E` detail text when
that range has text, render the normal header, and calculate the record cost
through the typed base-cost lookup and field discount context. Presentation
steps are callbacks, so this conversion records the source ordering without
turning the runtime into a VDP or ROM-pointer interpreter. Every branch then
finishes through the common tail text, including ordinary details and the
host's explicit `missing_cost_data` result; no tail step is silently dropped.
The source-bound overload feeds both the `0x130C0` +0x24 detail factor and the
`0x57A44` +0x22 cost word from the same zero-based `UsItemCatalog` row; the
older resource-table/cost-callback form remains an audit adapter. If the
typed record is neither unavailable nor special `E` and its D4540 row is
missing or has the wrong one-based identity, the typed path returns
`missing_source_data` before building a detail plan, calculating a price, or
calling presentation callbacks. It does not turn a missing row into an empty
plan or zero-cost action.

The remaining field-selection presentation cluster is now explicit as well:

The parent dispatcher at `0x57C7A` is now explicit as
`field_action_selection_render_conversion.*`. It writes grid variants B and C
first on every call, then preserves the source's signed result/selection
matrix: three separate negative-result detail tables, the negative quantity
and runner/inventory detail branches, the zero-result dynamic/fixed detail
families, result-1 equipment presentation, positive resource-description and
cost-summary selectors, the selection-5 quantity branch (with a source no-op
at primary cursor 5 or above), and the feedback-mode boundary. The host plan
carries semantic table-family choices,
the selected menu-resource entry, and the source cursor values; it does not
retain the original table addresses or turn the renderer into a VDP emulator.

The positive selection-5 branch embedded at `0x58394` is now represented by
`render_field_action_quantity_offer` in
`field_action_quantity_offer_conversion.*`. It is the panel for upgrading the
deck's MPCP or one of its four attributes, not a generic quantity screen:

* Category 0 selects the next MPCP rating from the extracted six-byte table at
  `0x1AB190`, while categories 1..4 select the next attribute rating from the
  typed current-rating array corresponding to `FFFBC6..FFFBC9`.
* MPCP equality with the `FFFBBE` capacity emits the `...MPCP checks OK`
  limit notice; an attribute whose current rating reaches that capacity emits
  `Maximum attribute rating equals MPCP value.`. Both retain the source's
  stale-quantity cost calculation and set the negative availability marker.
* Available categories calculate the exact `0x5787C` cost: cubic `quantity^3 *
  0x19` for MPCP and quadratic `quantity^2 * 3/2 * 100` for attributes 1/2
  versus 3/4. The typed callbacks preserve the source order of category label,
  category heading, upgrade label, quantity, cost, final grid, and the
  non-MPCP tail text.
* Category 5 and above never enter this source branch; the parent dispatcher
  exposes that as `no_action`, while missing extracted table rows are reported
  explicitly rather than read through an unrelated host object.

The negative-result runner/inventory detail path at `0x58540` is now a separate
typed renderer, `render_field_inventory_action_detail`. It reads the selected
active-runner item and phase from typed eight-slot arrays, accepts only source
item codes 1 through `0x2B`, and routes zero/out-of-range codes to the
unavailable message. Its source-bound overload uses the typed D4540 row for
the `+0x24` detail value and `+0x22` cost word; the older resource-table and
cost-callback form remains an explicit audit adapter. Valid items reuse the
`0x13228` inventory detail plan and the `0x5878E` action-detail text selector.
Action-dependent item codes still
perform the source `0x12148` lookup, but `0x13228` restores the original item
code before returning; that lookup result is retained as audit data and does
not replace the item-code key used by the detail-text, cost-record, or name
presentation.

The branch sets the source runner-cost suppression marker before rendering.
When that marker is clear in a host scenario, the typed path reads the selected
D4540 row's `+0x22` word cost, halves it at the same source point, applies the
runner surcharge, and emits header/cost/name callbacks in order. The source branch's
normal path suppresses pricing but still renders detail text and the common
item-name step; missing cost data preserves that name step and is reported as
`missing_cost_data`.

* `0x580FE render_field_action_description` receives the selected
  menu-resource byte as a description-table index into `0xE31F2`. The host
  conversion uses an extracted alias, renders the normalized/wrapped
  description, and selects the `0x1AC90C + index * 0x0C` grid record on every
  path. The source uses `CMPI.B #9` followed by `BMI`, so the quantity/capacity
  summary is selected by the wrapped result's N flag, not by a host signed
  comparison. The `Level`, `Size ... Deck Stg ... Mp`, and `Price` labels are
  followed by quantity, selected program storage weight, the wrapped
  `0x575F8` storage total after adding that weight back, and pending `FFF0BE`
  cost. Values such as `0x80` take the larger-quantity branch because
  `0x80 - 9` has a clear sign bit, while `0xFF` wraps back into the summary
  branch. Larger quantities render the
  `You have the highest version of this program.` notice instead. Missing host
  inputs are reported after the source-independent description/label callbacks
  and never suppress the final grid selection.
* `0x5825C render_field_action_cost_summary` reads the used fields of the
  `0x1AB118` 0x18-byte row in source order (`+6`, `+7`, `+C`, `+D`, `+8`,
  `+A`). A signed current-cursor/row-marker comparison selects grid variant A
  without discounting when the row is blocked. Otherwise the exact long-cost
  boundary is preserved: values below `0x10000` use `0x57A6E`, and values at
  or above it use `0x57B42`.
* `0x58316 render_field_action_feedback` treats modes 0..3 as valid feedback:
  the mode message is indexed directly, the quantity/result message starts
  at `mode * 5`, and the grid group uses the source `mode / 0x14` quotient.
  Modes 4 and above select the unavailable message, then both paths finish
  through the common tail text callback.

These are presentation plans and callbacks, not emulated VDP calls or ROM
address reads. Missing extracted description data, Matrix-selection data, or
pending cost data is reported as an explicit conversion result.

The field-mode dispatcher at `0x587DA` also required a source audit. Its
negative-result cursor 0, 1, and 2 paths do not share one marker table: they
select `0x1AAC86`, `0x1AADBF`, and `0x1AAD77` respectively before entering
the common eight-record loop. Cursor 3 instead writes the six-plus-two grid
batches, and later cursors transfer the glyph row. In the zero-result branch,
cursor 3 uses the glyph row; cursor 0 enters the dynamic
`0x1AAF40 + (menu_mode - 3) * 0x20` table after the source
`CMP.B #1`/`BMI`, while the other cursors select the fixed `0x1AAEB0` table,
the `0x1AAEF8` selection-one table, or the `0x58B1E` glyph-row path before
scanning markers. The conversion now carries those table choices as typed
callbacks and supplies separate extracted marker rows; it no longer silently
reuses one array for all three negative-result cases.

The same `0x587DA` audit preserves the source N flag for every byte boundary:
negative-result cursors 0..2 use the three dedicated tables only when the
wrapped `CMP.B #2`/`BMI` path says so; high-bit cursor bytes fall through to
the glyph row. Positive results use the wrapped `CMP.B #3`/`BMI` split, so a
high-bit cursor is not mistaken for a resource-description cursor. The
zero-result selector preserves `CMP.B #1`/`BMI` as well, including the
ordinary cursor-0 dynamic-table case and the high-byte fixed-low-table case.

The three helpers called by this dispatcher are now represented by their
actual data decisions as well. `0x58A2E` treats a raw marker of `E` as the
grid sentinel; other markers transfer glyphs, with negative-byte markers
using the blank glyph/high plane and the source `CMP.B #0x0F` split selecting
the low or high glyph plane for ordinary markers. `0x58AA4` reads eight item
codes from the active runner record at `+0x66..+0x6D`, maps nonzero codes to
the item-definition row at `(code - 1) * 0x30`, uses the blank asset for zero,
and performs the result-selection-4 encoded-text pass. The typed conversion
exposes these as record and inventory-glyph plans rather than hiding them
behind eight anonymous transfers.

The neighboring panel leaves are also data-specific: `0x58B5E` chooses the
negative/positive 18-grid panel or the zero-result 12-grid panel, while result
one suppresses the grid; `0x58BB6` chooses result-specific grid/text families
and applies primary-cursor strides of 8 bytes, `0x2A` bytes, and six screen
words. These choices are available as typed plans before the compatibility
callbacks perform the host presentation work.

The positive branch inside `0x587DA` is no longer represented as one generic
eight-grid case. Primary cursor 3 selects `0x1AB196` with a `0x70`-byte row
offset for non-mode-six menus (zero offset for mode six), then uses each row's
sign bit to choose the blank grid or `0xCE29A`. Cursor 5 uses the `0x1AC9B4`
five-plus-three split; other positive cursors use the `0x1AC9F0` four-plus-four
split. Cursors below 3 select the saved menu-resource record and use its
negative-marker/`0x1AC90C + marker * 0x0C` grid rule. These are exposed as
typed positive-branch and resource-row plans before the compatibility grid
callbacks run.

The conversion now carries the eight source markers into those plans. The
cursor-3 path uses one marker per `0x0E` source row and calls the blank grid
for a negative marker or the ordinary `0xCE29A` grid otherwise. The
sub-three resource path carries eight two-byte markers and resolves each to
the blank grid or its `0x1AC90C + marker * 0x0C` record. Detailed row callbacks
can consume these typed decisions; the old count-only callback remains a
compatibility fallback.

The adjacent action-selection dispatcher at `0x57C7A` had several host
comparison shortcuts that only worked for ordinary 0..5 menu bytes. Its
`CMP.B`/`BMI` boundaries for saved selection, the `0x585FA` cursor gate, and
the positive resource-description branch now use the wrapped source N bit.
The positive quantity path likewise preserves the `CMP.B #5`/`BPL` cutoff;
high-bit cursor bytes no longer become ordinary quantity purchases by virtue
of being negative `int8_t` values.

The `0x5825C` cost-summary leaf was checked in the same pass. Its row gate is
`CMP.B (marker),D1` followed by `BMI`, and its cost split is
`CMPI.L #0x10000,D1` followed by `BPL`. The typed renderer now evaluates those
source flag decisions directly, including the wrapped cursor and `0xFFFFFFFF`
cost cases, instead of using host `<`/`>=` operators.

## Field HUD row meter

`0x1792E render_menu_hud_rows` calls `0x56506` once for each eligible row.
The result is multiplied by the row-glyph stride, so this is the numeric HUD
meter value rather than a generic introductory helper:

| ROM entry | Recovered behavior | Typed conversion |
| --- | --- | --- |
| `0x56632` | Scales D6 by the source difficulty table after clamping D4 to 2: 100% at difficulty 2, then 80/60/40/20%, and 10% when the source word `7-D4` is not positive under `BGT`. | `scale_menu_hud_value_by_difficulty()` |
| `0x56506` | Status 4 reads row availability and scales it from the profile's special status bucket; statuses 5--7 return row availability directly; all other statuses combine action delay, the row availability value, the current record's cost/timing, the phase modifier, and the status-adjusted scale before clamping the display level. The full path exits on the source overflow-aware `SUB.B`/`BLE` cost gate. | `calculate_menu_hud_row_meter()` |

The conversion exposes `0x19C5C` row-availability reads,
`0x19BF6` status-adjusted scale selection, and `0x55F44` action-delay
calculation as typed callbacks. It does not pretend that those unresolved
source services are a runtime memory interface.

The direct slot lookup audit found an important conversion constraint. `0x19C54`
loads `FFF0D4` as the slot selector, then `0x19C5C` reads the loaded program
index from `FFFBD8 + slot*0x1` and uses that index to read the availability
byte at `FFFBCC + program`. The result remains in D7 as a byte; the callers
at `0x55FD8` and `0x56506` use it numerically in comparisons, addition, and
difficulty scaling. `read_matrix_program_availability_for_slot()` therefore
returns `availability_byte`, not `bool`. The old host conversion's truth-value
collapse was not source-faithful and is now covered by a test using `0xA5`.

The storage consumers at `0x19558` and `0x1957E` were audited against their
word instructions as well. Their D5 accumulators use `ADD.W`, and the final
D6 values use `SUB.W`; the host capacity results now retain 16-bit wrap rather
than widening the sums and publishing a signed deficit. This matters even
though normal Matrix data keeps the totals small, and is covered by an
overflowing availability/footprint test.

The load-timing leaf at `0x195AA` was audited in the same pass. `D1.w` is
copied to `D0.w` before `DIVU.W`, and the cap is controlled by the source
`CMP.W #7` followed by `BMI`; the conversion now preserves both the low-word
input and that wrapped sign-bit predicate. The normal Matrix range remains
0..6 steps, with malformed high-word/high-quotient cases covered explicitly.

## Primary world-effect pending roll

## Matrix transition initialization and title phase cycle: 0x1B1C4..0x1B270

The old export label for `0x1B1C4` was misleading. Its caller is the Matrix
program action service at `0x19976`, and the routine writes the transition
record used by `0x1B282`: selected program slot (`FFE0E4`), requested HUD mode
(`FFE0E0`), pending request (`FFE0DE`), twelve-frame stage timer (`FFE0DA`),
zero stage (`FFE0DC`), and a selected resource position from the five-word
table at `0x1B278` (`0x00D0, 0x0100, 0x0130, 0x0160, 0x0190`). It then tail-calls
`0x1A908 initialize_world_effect_mode`. The typed conversion is now
`initialize_matrix_program_transition()` and resolves that table through a
host-data callback; it does not carry the ROM address into runtime.

`0x1B1FC reset_title_transition` was also over-described. The ROM clears the
world-effect mode, arms the title-phase activity word, and clears the phase
countdown and phase. It does not set a dirty byte. `0x1B218` decrements the
four-frame countdown, wraps phase `0..5`, appends sprite key `0x6E0`, selects
the six-entry pointer table at `0x1DB92`, adds eight bytes to the selected
sprite record, and calls the attribute-offset builder with `(D1,D2,D6,D4) =
(0x10D,0x12E,0x2000,0x6E0)`. The host request exposes the table, phase,
resolved data token, key, placement, and builder flags as typed fields.

The adjacent `0x1B282 advance_matrix_program_transition` now exposes its
other sprite boundary as well. For stages `0..0x0B`, the ROM selects the
twelve-entry pointer table at `0x1DB1C`, passes `FFE0D8` as D1, `0x147` as D2,
and `0x2000` as D6 to `build_title_sprite_records`. Stage 6 still clears the
selected Matrix slot and refreshes the program grid; stage `0x0C` and later
refresh the mode glyph, clear the pending request, redraw the HUD rows, and
return through `0x1A908`. The typed conversion now records the source table,
stage, resource position, selected sprite data, and builder arguments instead
of reducing the call to an opaque stage callback.

The `0x1B302 advance_title_jitter` pass also corrected a host-only clear. The
ROM increments `FFE086`, runs exactly `0x58` PRNG samples into the four-word
`FFCB40` jitter prefix, then enters `0x1B56E` to write the full word stream and
set FFEF14 bits 2 and 3. At the `0x3C` terminal count it clears `FFE084` and
calls `0x1B5A6 reset_word_stream_area`; it does not clear the four jitter words
in this routine. The typed action now distinguishes updated, completed, and
PRNG-unresolved outcomes.

The adjacent `0x1B364 advance_title_figure_animation` pass retains four
independent sprite boundaries. It advances the support-layer service, updates
the five-phase counter, and submits two `0x780` records from `0x1DAF4` and
`0x1DB08` at `(0xF8,0xE4)` and `(0x148,0xE4)`, followed by two `0x7C0` records
from `0xEA490` and `0xEA4A4` at `(0xE2,0x114)` and `(0x15E,0x114)`. Every
selected pointer is advanced by eight bytes and built with `D6=0x6000`; the
host resolver now contains the extracted five-entry table data instead of
collapsing these into one generic title sprite.

The `0x1BA6E advance_title_burst_animation` pass now uses the actual four-entry
pointer table at ROM `0x1D970`: [`0x1D980, 0x1D9BA, 0x1D9E4, 0x1DA26`]. The
source advances the selected pointer by eight bytes before the attribute-offset
sprite builder, so the phase-one request resolves to `0x1D9C2` rather than a
synthetic table-plus-phase token. The four-frame countdown still clears
`FFE0AC` on phase four without appending a sprite on that terminal tick.

The `0x1B45C advance_title_noise_stream` audit preserves the source byte
semantics that matter to the waveform. `FFE16A` is decremented as a byte; the
first limit lookup uses its signed value plus one, while the second lookup
clamps only signed values `>= 3` to table index 2. The conversion updates 176
two-word-stride entries from `FFCB42` and 201 from `FFCB40`, using the rotated
sign stream from two PRNG words per wave, then delegates the full upload to
the `0x1B56E` word-stream boundary. A malformed signed table index is reported
as unresolved rather than silently becoming an unsigned frame count.
Each waveform sample also wraps as a 16-bit word before the signed difference
comparison, including the `0x7FFF` to `0x8000` transition.

The `0x1B87A dispatch_title_world_phase_service` boundary is now typed as two
source tables. FFE04E selects one of five handlers from `0x1BAE4`; the known
handlers are now named for their source behavior: gated +1 movement to 0xD0,
gated -1 movement back below 0xB0, unconditional +3 movement to 0x120 with
audio command 0x2A, unconditional -3 movement below 0xB0, and a gated
FFE050 sprite-bank advance that wraps at four and selects the -3 handler.
After that service, FFE050 selects one of four build-data pointers from
`0x1BAD4`, and FFE04A, FFE04C, and `0x6000` supply the common builder request.
Unresolved handler or sprite-table indexes remain explicit conversion outcomes
rather than being silently mapped to a generic sprite.

`0x1AB6E advance_primary_world_effect_overlay` calls `0x563CC` when the
primary effect has not yet crossed its threshold. The call is not merely an
intro transition: `0x563CC` obtains a trial count from the `0x19BF6`
status-adjusted auxiliary selection, clamps the pending amount to a minimum
difficulty of two, and calls `0x0DBA`. If the resulting roll count exceeds the
comparison byte, the excess is subtracted from the pending byte. A zero roll
count exits before that subtraction, and the comparison uses the source
overflow-aware `SUB.B`/`BLE` flags. Signed-byte underflow clears both the
pending amount and depletion counter; otherwise a nonzero depletion counter
is decremented. Both successful reduction paths reset the title transition
through `0x1B1FC`.

The typed conversion is `advance_primary_world_effect_pending_roll()`. The
overlay conversion's callback is named `advance_pending_roll` to preserve
that source boundary instead of mislabeling it as a generic intro request.

## World-effect transition resolver

When `0x1A5A0 advance_world_effect_mode_animation` finishes its mode-2
countdown, it calls `0x562EC`. The resolver first calculates a trial count
with `0x55F44`, adds the active profile byte, and rolls against the selected
record's `+0x06` timing plus twice `FFE16A`. It compares the new
`FFF0DD` result with `FFF0A4`:

- equal results redraw the HUD rows;
- a larger new result sets `FFE07E` to mode 1 and redraws the rows;
- a smaller result continues only when the difference exceeds the profile's
  comparison byte;
- the continuation advances HUD mode 8 directly, or runs the menu countdown
  and footer services before querying the mode again;
- modes 3, 12, 15, 23, and 24 enter the `0x563CC` pending-roll boundary,
  while all other paths redraw the HUD rows.

`resolve_world_effect_transition()` preserves that order through typed
callbacks for action delay, random rolls, HUD mode, countdown/footer work,
pending-roll advancement, and row redraw. It is still reference/conversion
code and has no dependency on the emulator's memory or VDP paths.

The importer has a separate ROM-only reader at
`src/graphics_import.cpp::read_genesis_field_entity_template()`. That reader
feeds extracted host assets/state; it is not the game runtime and is not a
replacement for the source-function conversion above.

## Post-startup field bootstrap audit

The common post-menu field path in `0x075E run_game_loop` is now composed in
the separate conversion-runtime source
`ghidra_conversion/integration/field_session_bootstrap_conversion`. It is not
part of `NativePortShell`, `shadowrun_qt`, `shadowrun_native`, or the emulator.
The central layer consumes typed source-table inputs and requires every still-
unresolved service as a callback, so a missing conversion fails explicitly.

The manually traced source order is:

| ROM range | Source behavior | Conversion boundary |
| --- | --- | --- |
| `0x07F0..0x07F6` | Run the six fixed `0x14` audio transactions with the `0x0C24` source-counter value `2`, then flush and service the zero-command audio tick; clear both 35-entry audio/menu arrays. | `prepare_audio_mode_entry()` plus `reset_audio_menu_arrays()` |
| `0x07FC..0x0890` | Select field mode `2`, clear the entry latches, initialize entity runtime and world-effect globals, copy the five action-profile rows, and copy one 23-word field/mode selection row from `0x14C1F4`. | `bootstrap_field_session()` typed state |
| `0x0896..0x08BA` | Enter `0x0281E`; the load branch still delegates save restoration and `0x028C2` re-entry to their separate converted boundaries. | `initialize_field_scene_entry()`; load restoration remains explicit in `startup_integration` |
| `0x08C0..0x0918` | Load `D7=0x1E`, call entity behavior service, camera update, and entity render pass 31 times, setting display dirty bits 1, 2, and 3 after each iteration. | `bootstrap_field_session()` warm-up loop |
| `0x0920..0x094C` | Wait for vblank, advance the exported title-palette fade helper, clear the controller/display markers, and sample the controller twice. | `bootstrap_field_session()` warm-up tail |

The conversion test checks the exact six-command audio cadence, one-row
23-word copy, alternate field profile selection, fixed `0xE10` terrain layer
copies, 31 warm-up iterations, dirty mask `0x0E`, and two final controller
samples. The `0x0A8EA` entity-behavior sweep is now converted as a typed
sixteen-slot control-flow service. Its action, transition, collision,
movement, proximity, effect, and dynamic-continuation callees remain named
callbacks until those downstream consumers are fully connected. The field
camera service remains an explicit callback; the central layer does not
replace either boundary with guessed behavior.

## Wrapped encoded text transfer: 0xC160 / 0xC168

The text renderer is a shared display-text boundary, not a field-menu-specific
memory helper. `0xC160` sets `FFF0B2` to select the alternate normalized buffer
and falls through to `0xC168`; `0xC168` chooses `FFEF96` when that flag is set or
`FFB740` otherwise, calls `0x0E9C` for the source transfer command, and writes
the encoded stream into successive destination lines. The native conversion
selects the already-normalized buffer as a typed `std::vector<uint8_t>` and
keeps the source flag out of runtime state.

The source scan preserves the last space whose count is below the requested
line width. If the scan reaches a marker before the width, the marker is part
of the consumed span but is not a glyph. Printable bytes map through the source
glyph table as `byte - 0x20`, with four omitted table entries after index
`0x3C`; the conversion exposes that mapping as
`wrapped_encoded_text_glyph_index()` before a host glyph callback runs. Marker
bytes `0x80..0xBF` consume the current line and begin another transfer, while
`0xC0..0xFF` terminate the stream. The renderer reports line/column placement
through callbacks rather than writing a VDP register or a ROM/RAM address.

The two important callers identify the actual game behavior. The Matrix runner
detail path around `0x188E8..0x18930` normalizes the selected program's
description and invokes the `0xC160` normal-buffer wrapper; the mode text-prompt
path around `0x1F3E6..0x1F3FE` normalizes a prompt into the alternate buffer and
invokes `0xC168` directly. Both paths now share the same rendering conversion
and the source trace is recorded in the display-text module.

## Grouped decimal glyph stream: 0xC216

`0xC216` is the shared numeric renderer used by Matrix runner detail values,
menu counters, magic-force values, equipment/action summaries, and other
status panels. It calls `0x0E9C` for each destination word, extracts the
least-significant decimal digit from `D1`, looks up that digit through the
source glyph table, writes it, and subtracts one word from `A2`. A four-state
counter emits three digits, then the separator glyph at source table byte
offset `0x18` (word index `0x0C`), before resuming digit extraction. The loop
ends when the quotient in `D1.w` is zero; zero itself emits one zero glyph.

The conversion returns typed source glyph-table indices rather than raw digit
bytes or a fake `0xFF` sentinel: digit `n` is table index `0x10 + n`, and the
separator is `0x0C`. Native presentation can map those indices to its host
font or authored text layout without reproducing the Genesis VDP destination
address or the original division helper. The source's overflow-safe 68000
division path is represented by ordinary bounded `uint32_t` division because
the host conversion needs the same decimal quotient/remainder result, not the
processor-specific divide sequence.

## Field-mode exit transition finalizer: 0xC64C

`0xC64C` is the common field-mode exit finalizer, not an unclassified graphics
stub. It first clears the three transient effect/prompt markers. The mode gate
then has three source-specific cleanup families: mode `1` clears the mode-one
high-effect slots when `FFFC1C` bit 0 is set; modes `2` and `4` clear the
mode-two/four slots and clear `FFFC22` bit 9 under the same gate; mode `5`
clears its own marker and high-effect slots only when bit 0 is set and bit 7 is
clear. Each successful family resets world-effect globals and runs the first
mode-exit service pass.

All paths then clear the two recruited-runner transition bytes, refresh the
runner state/assets, clear `FF0110` bits 6/4 while asserting bit 0, clear
`FF0111` bit 6, install field anchor `0xB45C`, clamp the action/resource
counters to `6` and `8`, and subtract `min(resource_pool >> 3, 5000)` from the
resource pool. The final service order is the common mode-exit service,
three-pass entity animation, mode-action event handling, then profile A when
`FFFC1E` bit 0 is set or profile B otherwise.

The conversion exposes the three high-effect families and the two service
phases as typed enums. `0x1F246`, `0xC802`, `0x38FA`, `0x391A`, and the profile
initializers remain explicit callbacks because their own source hills are
separate; no source pointer or RAM address is used as runtime state.

## Field runner state and asset refresh: 0xC802

`0xC802` is the field-mode refresh immediately adjacent to the exit finalizer.
It first serializes the default compact roster record and the selected primary
and secondary records through `0x59798`. It then evaluates the primary runner
at `FF0200` and the secondary runner at `FF0300` in that order. The direct
disassembly requires all three conditions: the live record active bit at
`+0x10` is set, its effect gate at `+0x4C` is zero, and its animation gate at
`+0x9C` is zero. The generated C used `gate_a == 0 || gate_b == 0`; that was
wrong and is recorded here as a source-audit correction.

For an eligible runner, the routine saves the 1-based compact roster marker,
publishes `0xFF` as the absent marker, calls `0xC8B0` with the source delta
`-2` against the corresponding typed effect-counter entry, and clears the
runner's effect slot. It dispatches `0x391A` only when that runner's stable
host identity matches the active runner identity. After both slots, it always
runs the field asset setup (`0x3404`) and the three-pass animation service
(`0x38FA`). The conversion uses a two-entry typed runner array and a bounded
counter vector; an invalid marker returns an explicit status instead of
indexing arbitrary host memory.

The roster serialization, effect clearing, action dispatch, asset upload, and
animation calls are explicit callbacks so this source conversion can be wired
to their already converted modules without duplicating a second state machine.

## Hospital Options menu and leave-hospital route: 0xC928 / 0xCE7C

`0xC928` is the hospital's three-entry `Hospital Options` menu, not a generic
field-exit selector. Its source table at `0xC1558` dispatches:

* `Medical Attn.` -> `0xCA2E`, the medical-attention detail/purchase routine;
* `Cyberware` -> `0xCA16`, which writes mode result `1`, runs the field-mode
  service, and returns to the mode-exit caller;
* `Exit` -> `0xCE7C`, the leave-hospital confirmation.

The outer conversion preserves the source choreography: reset display buffers
once, set the hospital-service result mode, rebuild metadata/palette/grid/font
and option text, initialize the three-entry cursor, then service each frame in
the order vblank, input latch, input sample, and random grid effect. Up has
priority over Down; movement still permits the same frame's confirm test;
Cyberware's action route wins over Start/cancel. Medical Attn. is an explicit
callback because its own `0xCA2E..0xCDE6` hill contains the medical percentage,
cost, affordability, and runner-condition logic.

`0xCE7C` defaults its two-entry Yes/No prompt to No. An accepted Yes does not
merely set a flag: the ROM deliberately pops the handler and Hospital Options
return frames before returning to the caller. The native routine exposes this
as `leave_hospital`, so the host controller can perform the same route without
reproducing unsafe stack manipulation.

The previously skipped leaf at `0xCDF6` is now converted separately as
`calculate_hospital_medical_attention_cost()`. Its direct ROM operations are
specific: `CMP.B #2,(record+0x9B)` adds two to the caller's service tier for
the under-two class gate; `+0x77` Body and `+0x7C` Willpower each contribute
`ceil(attribute/3)-1`; the tier is clamped to two; `D4-7` selects either
`abs(delta)*20` or the fixed factor ten; Body is scaled by that factor over
100, rounded upward for a remainder of at least 50, and clamped to a minimum
charge of one. The source caller supplies base tiers eight, six, and four for
the Hospital Options medical rows. This is a rules calculation, not a VDP or
runtime-ROM lookup.

## Hotel Options and Check Out route: 0xCF38 / 0xDA6A

The generated name `run_hotel_options_menu` for `0xCF38` is the source-backed
label; the older `run_field_exit_prompt_menu` name was incorrect.
The title pointer at `0xC14D0` is `Hotel Options`; the option text immediately
following it is `Karma Screen`, `Check Out`, and `Hospital Options`. The handler
table at `0xC14C8` contains `0xD024` for Karma Screen and `0xDA6A` for Check
Out. This is therefore a two-entry Hotel menu, not a generic field-exit
prompt.

The source sequence is now recorded in the native conversion as a typed loop:
clear the `0xFFF10D` entity-status marker, reset the mode display, run
`0xCFFE` once, service the three active entity-status records through `0xEF08`,
then rebuild the Hotel display. Each frame is vblank, controller latch,
controller sample, and `0xE660` random-grid servicing. Up takes priority over
Down but the same frame can still confirm; B/action calls `0x391A` and does not
create a separate Hotel cancel path.

The Karma handler target `0xD024` is an entry into the existing
Attributes/Skills/Cyberware routine beginning at `0xD01C`; it is kept as an
explicit callback until that full screen controller is converted. The Check
Out target `0xDA6A` is not a stub: it writes the prompt grid, renders
`Do you wish to check out?`, starts on No, wraps Yes/No through `0xC376`, and
accepts source controller bits `0x0E`, `0x0C`, or `0x0D`. Selecting Yes causes
the original routine to unwind both its handler and Hotel-menu return frames;
the native conversion returns `checked_out`/`returned_to_field` instead of
reproducing that stack manipulation.

The native names are `run_hotel_options_menu()`,
`prepare_hotel_options_display()`, and `run_hotel_checkout_prompt()`. Their
source-specific tests are in `tests/menus/hotel_options_menu_conversion_test.cpp`.

## Reviewed function map

The native owner names below are the target C++ vocabulary. A `partial` entry
means the source behavior is understood enough to structure the host state but
still has unresolved downstream calls or hardware presentation work.

| ROM entry | Ghidra name | Behavior recovered | Native C++ owner | Status |
| --- | --- | --- | --- | --- |
| `0x0236` | `reset_and_enter_boot` | Reset path: initializes VDP/Z80 state, clears display state, selects the region/title path, and transfers to the main coordinator. | `ShadowrunBoot` / `NativePortShell::update_boot` | partial |
| `0x0330` | reset display block | Emits the five initial VDP register commands, clears the first VRAM word, and establishes the text-write command. | `make_startup_display_plan()` | partial |
| `0x0436` | `write_startup_text_to_vdp` | Builds the destination command and converts a zero-terminated source string into host glyph indices. | `encode_startup_text()` / `ShadowrunBoot::write_startup_text()` adapter | partial |
| `0x073A` | `wait_for_vdp_ready` | Coalesces the source's 11 × 4096 startup delay, waits at a typed ready boundary, then applies the 256-iteration settle delay. | `ghidra_conversion::wait_for_vdp_ready()`; no host VDP polling loop | partial |
| `0x075E` | `run_game_loop` | Main coordinator: initializes session state, dispatches boot/title/field work, samples input, and performs indirect state dispatch. The common post-startup field bootstrap is now separately composed from `0x07F0..0x094C`; title/menu dispatch and save/new-game branches remain separate. | `ghidra_conversion::integration::bootstrap_field_session()` for the traced field-entry slice | partial |
| `0x0BB6` | `wait_for_even_frame` | Waits for a changed frame sample after clearing its low bit. | `ghidra_conversion::wait_for_even_frame()`; host frame boundary remains a callback | partial |
| `0x0BD2` | `wait_for_vblank` | Advances the frame counter and requests the next interrupt-side sample through a host frame boundary. | `ghidra_conversion::wait_for_vblank()`; presentation timing remains a callback | partial |
| `0x0C24` | `wait_for_vblank_frames` | Repeats the vblank boundary `source_counter + 1` times, preserving the source DBF behavior. | `ghidra_conversion::wait_for_vblank_frames()` | partial |
| `0x0D94` | `wait_for_controller_event` | Waits, latches, and samples until the controller event word has a nonzero high byte, then samples once more. | `ghidra_conversion::wait_for_controller_event()`; input delivery remains callbacks | partial |
| `0x0BF2` | `latch_controller_events` | Combines the sampled high-byte event words and latches activity and Start state. | `ghidra_conversion::latch_controller_events()`; host input delivery remains separate | partial |
| `0x0D44` | `advance_source_prng` | Advances the source 32-bit linear-congruential state with multiplier `0x20E5` and increment `1`. | `ghidra_conversion::advance_source_prng()` | partial |
| `0x0D3A` | `sample_source_random_bound` | Advances the source PRNG and scales its low word by the caller bound using the source multiply/shift operation. | `ghidra_conversion::sample_source_random_bound()` | partial |
| `0x0DBA` | `count_thresholded_random_rolls` | Performs the source repeated six-sided roll, accumulates face-6 retries, compares against the threshold, and counts qualifying trials. | `ghidra_conversion::count_thresholded_random_rolls()` | partial |
| `0x0C2C` | `clear_runtime_memory_and_vdp` | Clears or folds work RAM based on the startup sentinel, resets display state, and invokes low-level setup. | Native session initialization; bus transcription remains in `ShadowrunRuntime` | partial |
| `0x1088` | `step_palette_toward_black` | Moves each of the 64 CRAM words one Genesis color component toward black. | `ghidra_conversion::palette_toward_black()` | partial |
| `0x10D6` | `step_palette_toward_target` | Moves each of the 64 CRAM words one component toward the target palette. | `ghidra_conversion::palette_toward_target()` | partial |
| `0x131C` | `step_palette_transition` | Runs seventeen palette/update steps, each synchronized to frame/input service. | `ghidra_conversion::run_palette_transition()` | partial |
| `0x1342` | `wait_for_palette_frame` | Performs source-counter-plus-one vblank/latch iterations, preserving the DBF semantics used by palette steps. | `ghidra_conversion::wait_for_palette_frame()`; vblank and input latching remain callbacks | partial |
| `0x1358` | `copy_source_word_block` | Copies 32 longwords into the source work block. | `ghidra_conversion::copy_source_word_block()` | partial |
| `0x1368` | `copy_target_palette` | Copies 32 longwords into the 64-word target palette. | `ghidra_conversion::copy_target_palette()` | partial |
| `0x14CE` | `extract_even_bytes` | Extracts bytes from every second byte of the source table. | `ghidra_conversion::extract_even_bytes()` | partial |
| `0x1502` | `copy_even_bytes` | Copies bytes between every-second-byte positions in the source table. | `ghidra_conversion::copy_even_bytes()` | partial |
| `0x1610` | `decode_source_byte_stream` | Decodes literal bytes, repeated runs, and sliding-window copies from the source stream header. | `ghidra_conversion::decode_source_byte_stream()`; destination presentation remains separate | partial |
| `0x167C` | `decode_source_coordinate_stream` | Expands signed coordinate runs from the source base word until the zero terminator. | `ghidra_conversion::decode_source_coordinate_stream()` | partial |
| `0x3E86` | `expand_packed_nibbles` | Maps each packed byte through the source 16-entry nibble table. | `ghidra_conversion::expand_packed_nibbles()` | partial |
| `0x41B6` | `fold_field_coordinate` | Folds the high-nibble field coordinate into the eight-value source band. | `ghidra_conversion::fold_field_coordinate()` | partial |
| `0x4216` | `offset_field_coordinate` | Applies the source eight-unit high-nibble offset. | `ghidra_conversion::offset_field_coordinate()` | partial |
| `0x423C` | `quantize_field_delta` | Converts a wrapped coordinate delta into 128-unit, doubled steps. | `ghidra_conversion::quantize_field_delta()` | partial |
| `0x4E38` | `advance_entity_primary_timer` | On frame phase zero, decrements the primary timer and clears the source `0x80` flag at expiry; source `0x40` bypasses it. | `ghidra_conversion::advance_entity_primary_timer()` | partial |
| `0x4E64` | `advance_entity_secondary_timer` | On frame phase zero, decrements the secondary timer and clears the source `0x20` flag at expiry. | `ghidra_conversion::advance_entity_secondary_timer()` | partial |
| `0x5A56` | `project_entity_screen_position` | Converts fixed-point entity coordinates to screen words and selects the normal field or wider full-screen visibility window. | `ghidra_conversion::project_entity_screen_position()` | partial |
| `0x5A62` | `project_entity_screen_position_standard` | Converts fixed-point entity coordinates to screen words and applies the normal field visibility window. | `ghidra_conversion::project_entity_screen_position_standard()` | partial |
| `0x5B1E` | `sort_entity_slots_by_depth` | Stably sorts the sixteen entity slots by descending effective depth, treating source `0x10`/`0x40` entities as depth zero. | `ghidra_conversion::sort_entity_slots_by_depth()` | partial |
| `0x5B7E` | `mark_entity_slots_for_render` | Sets the render-update bit on all sixteen entity slots. | `ghidra_conversion::mark_entity_slots_for_render()` | partial |
| `0x5FDC` | `advance_entity_animation_accumulator` | Adds one animation increment, advances one frame on a period crossing, and marks the entity for redraw. | `ghidra_conversion::advance_entity_animation_accumulator()` | partial |
| `0x61A0` | `append_entity_sprite_record` | Appends the source pointer, attribute word, and frame-derived tile offset as one typed sprite record. | `ghidra_conversion::append_entity_sprite_record()` | partial |
| `0x61CC` | `append_sprite_record` | Appends the source pointer, attribute word, and caller-derived tile offset as one typed sprite record. | `ghidra_conversion::append_sprite_record()` | partial |
| `0x622C` | `expand_entity_sprite_descriptors` | Handles the optional `0x8002` (signed source `-0x7FFE`) alternate-tile marker before expanding one entity's descriptor list. | `ghidra_conversion::expand_entity_sprite_descriptors()` | partial |
| `0x624C` | `expand_entity_sprite_descriptor_list` | Projects bounded four-word sprite descriptors into ordered screen-space output records until `0x8000`. | `ghidra_conversion::expand_entity_sprite_descriptors()` | partial |
| `0xB530` | `approach_entity_fixed_point_target` | Moves a fixed-point entity coordinate by one `0x800` step or snaps it when the remaining distance is at most one step. | `ghidra_conversion::approach_entity_fixed_point_target()` | partial |
| `0xB6E4` | `select_entity_animation_table` | Disassembly-derived selector for the `<0x100`, `0x100`–`0x1FF`, `0x200`–`0x35F`, `0x360`–`0x45F`, and `>=0x460` animation table banks; the latter uses the special bank when entity flag bit 3 is set. | `ghidra_conversion::select_entity_animation_table()` | partial |
| `0xB39A` | `set_entity_heading_with_field_animation` | Stores the requested heading, selects the C042 directional animation entry after the source rotate-and-correct step, chooses delay mode `0` for behavior values `0x0B`/`0x0C` and `1` otherwise, then resets animation. | `ghidra_conversion::set_entity_heading_with_field_animation()` | partial |
| `0xB3FE` | `set_entity_heading_with_alternate_animation` | Stores the requested heading, selects the C092 directional animation entry, resets animation first, and schedules fixed delay mode `0`. | `ghidra_conversion::set_entity_heading_with_alternate_animation()` | partial |
| `0xAF6C` | `reset_entity_animation_state` | Clears the animation timer and phase, then sets the entity redraw/update bit. This shared helper is used by both heading-selection paths and other behavior transitions. | `ghidra_conversion::reset_entity_animation_state()` | partial |
| `0xA8EA` | `service_entity_behavior_frames` | Dispatches action candidates once, scans all sixteen active entity slots, decrements the signed animation/transition timer, routes the +0x10 behavior bits through action/collision/transition branches, applies mode-5 boundary and proximity work, temporarily installs fallback movement for +0x11 bit 1, calls the dynamic continuation, clears forced effects, and marks animation changes. | `ghidra_conversion::service_entity_behavior_frames()` | partial |
| `0xAB02` | `complete_entity_behavior_transition` | Completes the entity transition after `0xA8EA`: starts the route-flag activity, invokes the weighted scene-entity spawn path when behavior bit 3 is clear, decrements the unsigned spawn countdown when the spawned record carries +0x9E bit 6, resets the ten service records and hands off a deferred activity when that countdown reaches zero, marks the six linked records whose +0xA8 owner matches the current entity, and arms the 0x3C-frame activity delay only when FF0110/FF0210/FF0310 pass their source gates. | `ghidra_conversion::complete_entity_behavior_transition()` | partial |
| `0xACAE` | `advance_entity_behavior_action` | Routes one entity through the field-service action gates: selects a world target on the 0x1000 event, redirects movement/activity locks, handles the signed transition gate, prepares target animation, spawns the 0x0D..0x0F effect family, configures the type-zero target transition, or processes the ordinary spawn cycle. A successful player action emits audio event 0x4B and updates delay mode 0; failed action dispatch schedules collision recheck. | `ghidra_conversion::advance_entity_behavior_action()` | partial |
| `0xABFC` | `update_entity_animation_delay` | Source-traced D0 mode scaling around `0x6A`, nested `0xD7EA` factor call, no-op A0+0x10 bit test, minimum `0x0F`, and A0+0xDC animation-delay write. | `ghidra_conversion::update_entity_animation_delay()` | partial |
| `0xD7EA` | `compute_entity_delay_factor` | Source-traced A0+0x90 capability scan for the first active bit 12..14, 4/8/12 modifier selection, `0xD81C` call, subtract-one factor, and zero floor. | `ghidra_conversion::compute_entity_delay_factor()` | partial |
| `0xD81C` | `average_entity_delay_attributes` | Source-traced A0+0x78/+0x7B byte addition with wrap and floor average returned in D1; generated C is return-only. | `ghidra_conversion::average_entity_delay_attributes()` | partial |
| `0xAF7C` | `prepare_entity_delayed_animation_primary` | Applies normal delay mode, selects the C082 direction entry, installs the primary delayed-collision continuation, and resets animation state. | `ghidra_conversion::prepare_entity_delayed_animation_primary()` | partial |
| `0xAFAE` | `prepare_entity_delayed_animation_alternate` | Applies normal delay mode, selects the C082 direction entry, installs the alternate delayed-collision continuation, and resets animation state. | `ghidra_conversion::prepare_entity_delayed_animation_alternate()` | partial |
| `0xAFF2` | `prepare_entity_auxiliary_behavior` | Ignores an already-active behavior; otherwise either starts the C072 countdown branch at 8 or brackets the external controller/behavior effects, sets flag `0x10`, and selects C0B2 for immediate collision. | `ghidra_conversion::prepare_entity_auxiliary_behavior()` | partial |
| `0xB330` | `enter_entity_behavior_transition` | Brackets the controller effect with flag `0x40`, dispatches the selected behavior action, sets flag `0x10`, selects C0B2, and resets animation state. | `ghidra_conversion::enter_entity_behavior_transition()` | partial |
| `0xB11C` | `synchronize_entity_behavior_spawn` | Compares a refreshed related-entity signature, prepares related behavior and allocates/initializes a child when it changes, then selects C042 and enters the spawn-collision continuation. | `ghidra_conversion::synchronize_entity_behavior_spawn()` | partial |
| `0xB1DC` | `configure_entity_target_transition` | Dispatches the initial action, resolves a linked heading, selects C0A2, resets animation, and conditionally performs the follow-up action and related-child path. | `ghidra_conversion::configure_entity_target_transition()` | partial |
| `0xB49C` | `resolve_entity_fallback_movement` | Selects the source heading, probes base/quarter-turn/half-turn direction slots through the movement-result policy, commits a failed candidate position when needed, and restores the saved continuation. | `ghidra_conversion::resolve_entity_fallback_movement()` | partial |
| `0xB7B8` | `advance_entity_movement_and_mode_transition` | Consumes a blocked movement probe once per pending record, requires behavior bit 1 and at least two negative directional probe bytes, rotates the selected 0xBC36 direction slot into the entity heading, quantizes the candidate field coordinate, routes event codes 0x1A/0x18/0x19/0x1B-0x1F to typed terrain, dialogue, activity, and mode-exit continuations, and preserves the source's world-interaction retry when the projection callback returns carry. | `ghidra_conversion::advance_entity_movement_and_mode_transition()` | partial |
| `0xBB86` | `evaluate_entity_movement_route` | Derives the even heading route key, reads one of four extracted `0xBBF6` descriptors, treats two positive sampled terrain bytes as the source blocked-route carry, or selects and probes the descriptor's fallback direction slot through the movement-result policy. | `ghidra_conversion::evaluate_entity_movement_route()` | partial |
| `0xBADE` | `advance_entity_movement_step` | Transfers linked collision context, probes the requested direction, handles negative recovery, branches through aligned or diagonal movement checks, updates animation state, and commits or resets the step. | `ghidra_conversion::advance_entity_movement_step()` | partial |
| `0xB442` | `reset_entity_movement_state` | Clears movement flag bit 1, restores the desired heading, replaces the raw `0xB45C` continuation with `ReevaluateCollision`, marks animation refresh, selects the rotated heading's animation entry, and rechecks the current position. | `ghidra_conversion::reset_entity_movement_state()` | partial |
| `0xB48C` | `evaluate_entity_collision` | Sends the candidate coordinates through the field-collision boundary, copies the returned four-byte collision state to the entity record, and returns the collision code. | `ghidra_conversion::evaluate_entity_collision()` | partial |
| `0xB732` | `apply_movement_result_policy` | Normalizes a negative directional probe result to `0x7F` for an active field transition, high result bits, or scene `0x14`/collision mode `2` result bands `2`–`5`; non-negative values pass through. | `ghidra_conversion::apply_movement_result_policy()` | partial |
| `0xB77E` | `probe_entity_directional_collision` | Selects one of eight signed movement vectors, adds it to the entity position, queries the collision boundary, records the source two-byte direction slot, and maps negative results to `0x7F` unless entity flag bit 1 is set. | `ghidra_conversion::probe_entity_directional_collision()` | partial |
| `0xBBE4` | `commit_entity_position` | Writes an entity position and clears its pending transition bit. | `ghidra_conversion::commit_entity_position()` | partial |
| `0xBC36` | `build_collision_probe_points` | Selects one of four three-point collision-offset banks from the signed probe bytes, expands it, and snapshots the camera words unless the source mode gate is active. | `ghidra_conversion::build_collision_probe_points()` | partial |
| `0xBCAE` | `apply_mode_five_boundary_motion` | Applies the mode-5 vertical adjustment in the source's strict coordinate windows at the two field edges. | `ghidra_conversion::apply_mode_five_boundary_motion()` | partial |
| `0xE8A0` | `sample_terrain_collision_tiles` | Samples the center tile or four ±12/±7 corner tiles, retains the highest unsigned code with later ties winning, and overrides it with `0x7F` when any corner is blocked. | `ghidra_conversion::sample_terrain_collision_tiles()` | partial |
| `0xE9EE` | `contains_terrain_object_bounds` | Uses `terrain_code * 10` to select the same five-word field collision record used by `0xEE10`, then tests normalized X against record left/right expanded by ±12 and Y against top/bottom expanded by ±7. | `ghidra_conversion::contains_terrain_object_bounds()` | partial |
| `0xEAD2` | `evaluate_terrain_direction_boundary` | Applies the terrain-code `0x38` direction relation to unsigned word deltas within the decoded tile window. | `ghidra_conversion::evaluate_terrain_direction_boundary()` | partial |
| `0xEDFA` | `decode_terrain_tile_window` | Expands the packed terrain tile index into its horizontal origin, row origin, and 32-unit row end. | `ghidra_conversion::decode_terrain_tile_window()` | partial |
| `0xEE10` | `rebuild_terrain_collision_map` | Clears the bounded 64x64 terrain map, rasterizes inclusive 32-unit rectangle records, and invokes the explicit post-processing boundary. | `ghidra_conversion::rebuild_terrain_collision_map()` | partial |
| `0xEE86` | `replace_nth_terrain_marker` | Replaces the requested zero-based occurrence of a terrain marker, matching the source's first-marker behavior for ordinal zero. | `ghidra_conversion::replace_nth_terrain_marker()` | partial |
| `0xFEBE` | `move_selection_column` | Moves the active column by a signed step and wraps within the source row's available range. | `ghidra_conversion::move_selection_column()` | partial |
| `0xFF06` | `move_selection_row` | Changes the two-row selection mode only when at least seven entries exist and clears it when the active column is unavailable. | `ghidra_conversion::move_selection_row()` | partial |
| `0xC376` | `move_wrapped_menu_cursor` | Adds the signed menu step with source `ADD.B` flags, uses the overflow-corrected BGE result to wrap negative sums to the supplied upper cursor value, then uses the raw `D3 - cursor` N bit to wrap above-range values to zero; paired cursor-word writes remain a presentation boundary. | `ghidra_conversion::move_wrapped_menu_cursor()` | partial |
| `0xC802` | `refresh_field_runner_state_and_assets` | Serializes compact roster records, scans primary then secondary field runners through the direct AND gate, consumes the 1-based roster marker with the existing negative-floor counter service, clears effects, dispatches the active-runner action by stable identity, then uploads field assets and runs the animation triplet. | `ghidra_conversion::refresh_field_runner_state_and_assets()` | partial |
| `0xC928` | `run_hospital_options_menu` | Runs the Hospital Options entries Medical Attn., Cyberware, and Exit with source cursor/input priority; Cyberware routes to field-mode service and Yes on the leave prompt returns an explicit leave-hospital route. | `ghidra_conversion::run_hospital_options_menu()` |
| `0xCDF6` | `calculate_hospital_medical_attention_cost` | Calculates a Hospital Medical Attn. charge from the active runner's class, Body, Willpower, and one of the three caller-selected service tiers, including source rounding and minimum-charge behavior. | `ghidra_conversion::calculate_hospital_medical_attention_cost()` |
| `0xCE7C` | `run_leave_hospital_prompt` | Runs the Yes/No leave-hospital prompt, defaulting to No; the accepted Yes path is modeled as a typed non-local return rather than source stack pops. | `ghidra_conversion::run_leave_hospital_prompt()` |
| `0xC8B0` | `apply_byte_counter_delta_with_negative_floor` | Subtracts one from the caller's one-based counter selector, adds the signed byte delta to the selected counter, and replaces a negative result with the source `0xFF` sentinel. | `ghidra_conversion::apply_byte_counter_delta_with_negative_floor()` | partial |
| `0xC134` | `normalize_source_text_line` | Maps source high-bit glyph markers, stops on the line terminator range, and pads the remaining line width. | `ghidra_conversion::normalize_source_text_line()` | partial |
| `0x15554` | `service_nearby_entity_auxiliary_behavior` | Scans the ten entity records with active/non-excluded filtering, applies the normal `0x280` radius/`0x500` exclusive window or the type-`0x0E` `0x140`/`0x280` window, clamps lower bounds at zero, and calls the existing `0x1562E` auxiliary service for each match. | `ghidra_conversion::service_nearby_entity_auxiliary_behavior()` | partial |
| `0x17C3C` | `read_menu_entry_profile_eligibility` | Indexes the source `FFFB30` byte table with `FFF102`, sign-extends the selected byte, then performs the source logical four-bit word shift. The generated Ghidra C body was an empty stub; direct disassembly is authoritative here. | `ghidra_conversion::read_menu_entry_profile_eligibility()` | partial |
| `0x1A16A` | `render_matrix_runner_detail_panel` | The runner-specific child of `0x1882E`: writes two common detail grids, renders the no-active-run message when `FFF0B0` is nonnegative, gives program 4 its direct status-row path, gives programs 5..7 their special text path, and otherwise computes the delay-scaled primary/eligibility-gated secondary/final row passes. The primary/final counts follow the source `CMP.W #$0E`/`BMI`, `SUBQ.W #1`, and `DBF` sequence (zero still emits one row, ordinary values cap at 13); the secondary destination subtracts the compact-record `+0x05` value from the restored primary loop counter after the primary `0x11C0` call, not from the unclamped delay/status sum. | `ghidra_conversion::render_matrix_runner_details()` | partial |
| `0x1A370` | `advance_world_effect_controller` | Per-frame world-effect coordinator: gates the active `FF163E` record, advances the descriptor transition's byte-sized noise cadence, starts descriptor loading when the current compact runner has no active effect, decrements the effect countdown, selects the next world-effect mode, and publishes the mode-2 background, mode-3 motion, or default-effect setup. The VDP noise loop is an explicit host presentation boundary. | `ghidra_conversion::advance_world_effect_controller()` / `advance_world_effect_transition()` | partial |
| `0x1A5A0` | `advance_world_effect_mode_animation` | Runs the mode-2/3/4/5/6/default world-effect animation state machine: selects the individual sprite-table entry, applies the source fixed-point position deltas, falls through into the first tick of a newly selected mode, arms title refresh after audio, rebuilds title sprites on the negative transition result, and installs descriptor D/E or the inline mode-5 descriptor pair at the source countdown boundaries. | `ghidra_conversion::advance_world_effect_mode_animation()` | partial |
| `0x55F44` | `calculate_runner_action_delay` | Saves D1 across `0xD81C`, combines active runner bytes `+0x78`, `+0x7B`, `+0x88` and `FFFBBE+0x07` with source byte wrap, then applies the signed `FFF0B7` normal/positive/negative branches and request-dependent profile-floor behavior. The generated Ghidra body retains only the `0xD81C` call; direct disassembly is authoritative. | `ghidra_conversion::calculate_runner_action_delay()` | partial |
| `0x55FD8` | `calculate_and_publish_runner_action_result` | Clears the prior action result, forces the shared delay calculation's request to zero, combines the action delay with the selected Matrix-program availability byte, subtracts the selected compact record's `+0x05` gate from the low byte, and exits with `-1` when the 68000 `SUB.B`/`BLE` flags report a nonpositive result, including signed-overflow cases. The inline continuation at `0x55FFC` then adds twice compact `+0x06` and the world-effect phase, clamps difficulty to two, calls `0x0DBA`, and writes the resulting success count or `-1` to the active live record's `+0x9F`. The nominal exported span is only 38 bytes; the canonical ROM bytes through `0x5604E` are required to recover the actual routine. | `ghidra_conversion::calculate_and_publish_runner_action_result()` | partial |
| `0x15CA4` | `compute_diagonal_step_cost` | Subtracts and normalizes the two low words, selects the major axis with the source unsigned `CMP.W`/`BCC`, then applies `ASR.W` plus carry-based rounding and `ADD.W`. The typed helper preserves this wrapped word metric; the record-based `0x15CC0` entry reaches the same leaf after loading its point pair. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15C58` | `is_entity_target_within_range` | Requires target flag bit 0, rejects bit 6, measures the linked target, and reports distance below `0x200`. | `ghidra_conversion::is_entity_target_within_range()` | partial |
| `0x15C84` | `update_entity_heading_from_linked_target` | Checks the linked record through `0x15C58`, writes the measured heading to record `+0x34` on success, and leaves the carry boundary explicit for the `0x14C7C` caller. | `ghidra_conversion::update_entity_heading_from_linked_target()` | partial |
| `0x15CC0` | `compute_diagonal_step_cost` | Loads two record coordinate pairs, performs the same low-word absolute-difference, unsigned-major-axis, `ASR.W`/carry-round, and `ADD.W` sequence as `0x15CA4`; the host API exposes deltas only after the record loads have been modeled. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15D3C` | `measure_relative_heading_and_distance` | Subtracts two world positions and derives the source fixed-point heading plus half-major-axis distance. | `ghidra_conversion::measure_relative_heading_and_distance()` | partial |
| `0x14D58` | `measure_entity_target_heading_and_distance` | Loads the current and linked entity positions before applying the shared relative heading/distance metric; the source preserves unrelated caller registers. | `ghidra_conversion::measure_entity_target_heading_and_distance()` | partial |
| `0x14D6A` | `select_entity_motion_action_row` | Selects D4540 for a nonnegative active action state or D4EA0 for a negative one, then converts the one-based action code into a `0x30`-byte row whose `+0x27` motion-distance profile is consumed by `0x13D0C`. | `ghidra_conversion::select_entity_motion_action_row()` | partial |
| `0x14C7C` | `resolve_entity_target_motion` | Dispatches entity types `0xC`, `0xD`, and `0xE`: type `0xC` applies the `<0x300`, `0x300..0x5FF`, and far bands, allocates the close-effect slot when `+0xDC` is negative, and writes its D0/D1 slot record fields; types `0xD/E` temporarily project `+0x2E`, use `0x15C84`'s `+0x34` carry, restore the old current heading, and only commit both heading fields when the mode gate allows it. | `ghidra_conversion::resolve_entity_target_motion()` | partial |
| `0x14F26` | `probe_entity_world_position` | Combines a record's fixed-point world coordinates with signed `0x100` tile offsets before invoking the terrain-probe boundary. | `ghidra_conversion::probe_entity_world_position()` | partial |
| `0x14F50` | `advance_entity_collision_recovery` | Retries an empty/boundary terrain result through `+0x89`, returns carry on terrain rejection or retry exhaustion, commits D5/D6 only for a live retry, and forwards the `0x15ABA` collision carry. | `ghidra_conversion::advance_entity_collision_recovery()` | partial |
| `0x150E8` | `derive_entity_heading_offsets` | Uses the extracted `0x15E16`/`0x15E56` direction tables, signed `0xC` scale, and arithmetic four-bit shift to produce the record's horizontal and vertical offsets; the shared host helper also exposes the constructor-specific scale/shift pair. | `ghidra_conversion::derive_entity_heading_offsets()` / `derive_scaled_entity_heading_offsets()` | partial |
| `0x14FF8` | `initialize_entity_spawn_record` | Clears and seeds a child record, resolves an active linked target heading, applies the direct positive word values from `0x15A4C`/`0x15A6C` as host coordinate deltas, and stores heading offsets. | `ghidra_conversion::initialize_entity_spawn_record()` | partial |
| `0x1514E` | `process_entity_spawn_cycle` | Applies the global-mode and `+0x10` bit-1 late-window gate, services the ammo/slot boundaries, initializes the primary record, selects resource table `0x15318` or alternate `0x15336` from the indexed `+0x6E` status byte, optionally starts the mode-2 world transition, then clones the 80-word record and applies the exact `+0x3E/+0x11/+0x0E/+0x3A/+0x64/+0x41/+0x16` follow-up writes. | `ghidra_conversion::process_entity_spawn_cycle()` | partial |
| `0x15118` | `initialize_entity_behavior_record` | Clears the typed behavior-record fields and installs the source `-12` vertical offset, `0x1000` timer, `0x4000` step interval, active flag, and update continuation. | `ghidra_conversion::initialize_entity_behavior_record()` | partial |
| `0x15646` | `spawn_entity_effect_record` | Builds the short-lived effect/projectile entity during the parent's `0x3C`-frame window: advances the asset meter, claims an entity and one of eighteen effect slots, copies the source record fields, places the effect with the direct `0x15A4C`/`0x15A6C` direction words, then either derives the marker/heading offsets locally or resolves a linked target through the explicit `0x55848` boundary and clamps its travel distance to `0x0BE0`. | `ghidra_conversion::spawn_entity_effect_record()` | partial |
| `0x15830` | `dispatch_entity_effect` | Handles the linked effect path after the direct `0x07/0x0E` spawn branch: schedules the world timeout, claims an effect slot, scans ten live records around the linked target, chooses the linked record or the first nearby `0x06/0x0D` candidate, writes `+0xB2/+0x32/+0x2A/+0x02/+0x03`, and preserves the `0x4946` audio table plus `0x4964` indirect handler as typed callbacks. | `ghidra_conversion::dispatch_entity_effect()` | partial |
| `0x1597E` | `scan_entity_effect_targets` | For source types `0x03/0x05/0x0A`, marks every active, non-excluded candidate within the `0x500` diagonal-cost band; for `0x06/0x0D`, also sets the source flag/link and selects the first compatible target. | `ghidra_conversion::scan_entity_effect_targets()` | partial |
| `0x15930` | `apply_entity_secondary_effect` | Claims a local effect slot, initializes a type-2 target effect with phase zero, then crosses the source audio event `0x34` and indirect handler boundary. | `ghidra_conversion::apply_entity_secondary_effect()` | partial |
| `0x152FA` | `prepare_special_spawn_record` | Moves the primary spawn into the special follow-up state, saving its placed coordinates and setting the `0x20`/`1` transition markers. | `ghidra_conversion::prepare_special_spawn_record()` | partial |
| `0x15ABA` | `find_colliding_entity` | Scans the source ten-record collision range, applies active/occupancy/window filters, copies the candidate position only on the ordinary compatible path, and preserves collision-gate and source-override carry-only exits. | `ghidra_conversion::find_colliding_entity()` | partial |
| `0x15D4E` | `measure_heading_and_distance` | Converts a signed delta pair into clockwise heading units and the source half-major-axis distance using the extracted 1024-entry correction table as a typed host constant; the distance is narrowed to the source D3.w result before host comparisons. | `ghidra_conversion::measure_heading_and_distance()` | partial |
| `0x148D2` | `choose_dominant_axis_directions` | Chooses horizontal slots `6/2` and vertical slots `0/4` from signed deltas, then places the dominant axis first while preserving horizontal priority on ties. | `ghidra_conversion::choose_dominant_axis_directions()` | partial |
| `0x14756` | `clear_entity_route_flags` | Clears source movement flag bits `1` and `3` after route selection finishes or fails. | `ghidra_conversion::clear_entity_route_flags()` | partial |
| `0x14682` | `select_obstruction_route` | Sets +0xB0 bit 1, rejects an occupied target, writes a direct or two-tile bypass route, or carries the heading-driven 0x14AF4 retry into the 0x149C0 queue. | `ghidra_conversion::select_obstruction_route()` | partial |
| `0x14764` | `find_alternate_route_tile` | Accumulates a primary grid step until the candidate's side opens, or returns the previous candidate at an occupied/bounded stop. | `ghidra_conversion::find_alternate_route_tile()` | partial |
| `0x14AF4` | `find_rotating_route_strip_target` | Converts the entity position to an obstruction tile, tries five cardinal strip orientations, and returns the first non-current candidate. | `ghidra_conversion::find_rotating_route_strip_target()` | partial |
| `0x148AA` | `find_first_occupied_obstruction_tile` | Advances by the selected cardinal obstruction-map delta until a nonzero tile is reached and returns the last open tile in the source D1 role; the host bounds the search at the map edge. | `ghidra_conversion::find_first_occupied_obstruction_tile()` | partial |
| `0x14800` | `probe_obstruction_strip` | Advances through open tiles along a primary direction while the adjacent secondary tile remains occupied and returns the last open candidate, with explicit host edge reporting. | `ghidra_conversion::probe_obstruction_strip()` | partial |
| `0x14854` | `find_bypass_route` | Produces the primary and secondary open route tiles written by the source; `can_bypass_obstruction` remains the boolean compatibility wrapper. | `ghidra_conversion::find_bypass_route()` | partial |
| `0x1494C` | `is_obstruction_path_blocked` | Walks the packed six-bit-column/0x40-row obstruction line and reports the first occupied tile encountered. | `ghidra_conversion::is_obstruction_path_blocked()` | partial |
| `0x14914` | `is_entity_path_blocked` | Converts entity and target world positions to packed grid indices, then checks the obstruction line in both source directions. | `ghidra_conversion::is_entity_path_blocked()` | partial |
| `0x145A2` | `advance_entity_animation_target_route` | Applies the frame-counter cutoff, clears/rebuilds +0xB1 axis relation flags, enters the +0xB0 route queue when requested, publishes the direct/queued heading-distance pair, and installs the B574 continuation. | `ghidra_conversion::advance_entity_animation_target_route()` | partial |
| `0x149C0` | `advance_obstruction_route_queue` | Tests +0xB0 bit 3 for a blocked path, consumes the packed route cursor at +0xBA/+0xBC, publishes the decoded obstruction target/cost, and returns to the 0x14A36 completion or 0x14A54 replan branches. The host uses the direct low-byte flag tests from disassembly, not the Ghidra C export's 0x0800 word mask. | `ghidra_conversion::advance_obstruction_route_queue()` | partial |
| `0x14BA6` | `update_entity_axis_relation_state` | Updates signed X/Y relation bits, preserves the source movement-mode gate, and reports the unresolved route-advance handoff at `0x14682`. | `ghidra_conversion::update_entity_axis_relation_state()` | partial |
| `0xE00A` | `wait_for_controller_event` | Services frame timing, controller latch/sample, and caller-provided update work until the high event byte is nonzero, then takes one final sample. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0xF312` | `wait_for_controller_event` | Shares the same event-wait loop as `0xE00A`; its differing source update path is supplied by the host callback boundary. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0x118C6` | `cycle_inventory_action_selector` | Applies the horizontal action delta across selectors `2..5`, wraps `2 - 1` to `5` with the source redraw sentinel, and wraps `5 + 1` to `2`. | `ghidra_conversion::cycle_inventory_action_selector()` | partial |
| `0xA7E4` | `initialize_entity_slot_order` | Replaces the source sixteen-pointer, `0x80`-stride initialization with stable host slot indices `0..15`. | `ghidra_conversion::initialize_entity_slot_order()` | partial |
| `0xAE5E` | `schedule_entity_collision_recheck` | Sets the entity pending-reaction bit and records the collision-recheck continuation as typed state. | `ghidra_conversion::schedule_entity_collision_recheck()` | partial |
| `0x1A2CC` | `allocate_first_free_effect_slot` | Scans eighteen source effect bytes, marks the first zero slot occupied, returns D0 as the slot, D1 as `0x49 + slot*0x18`, and A2.w as `0x0920 + slot*0x0300`, and sets the table-size sentinel when full. The previous conversion dropped A2 and allowed the spawned-effect `+0x26` field to be injected; the typed allocator now exposes both slot-derived outputs and the spawn conversion derives that field from the allocation. | `ghidra_conversion::allocate_first_free_effect_slot_record()` / `ghidra_conversion::allocate_first_free_effect_slot()` | partial |
| `0x1A342` | `find_first_available_world_effect_entity_slot` | Scans seven world-effect entity slots and returns slot `3..9`, or the full-table sentinel `10`. | `ghidra_conversion::find_first_available_world_effect_entity_slot()` | partial |
| `0x1DBAA` | `schedule_world_activity_timeout` | Applies the guarded-scene early exit, arms the source timeout marker, and selects the short `0x78` or long `0x258` timeout. | `ghidra_conversion::schedule_world_activity_timeout()` | partial |
| `0x1DF00` | `clear_entity_effect_pending_flags` | Clears bit 0 across the six world-record effect flag bytes. | `ghidra_conversion::clear_entity_effect_pending_flags()` | partial |
| `0x1DFC4` | `advance_world_effect_trigger` | Finishes the latched transition countdown or applies the source cadence, PRNG, bounded-region, and transition gates. | `ghidra_conversion::advance_world_effect_trigger()` | partial |
| `0x1E04A` | `begin_world_effect_transition` | Schedules the scene-14 timeout or starts the mode-2 transition with the source `0x960` timer and sound/flag/effect boundaries. | `ghidra_conversion::begin_world_effect_transition()` | partial |
| `0x1E0B8` | `promote_entity_effect_flags` | Promotes bit-1 effect records to the source bit-2 state across all six world records. | `ghidra_conversion::promote_entity_effect_flags()` | partial |
| `0x1E0DC` | `spawn_world_effect_entities` | Walks the typed effect descriptors, allocates entity/effect slots, applies grouped vertical origins, marks effect state, projects entities, and hands off visible descriptor coordinates. | `ghidra_conversion::spawn_world_effect_entities()` | partial |
| `0x1E25A` | `spawn_world_effect_from_coordinate_sequence` | Allocates a coordinate-sequence effect, applies the zero-first-coordinate fallback, updates the active count, and schedules the timeout. | `ghidra_conversion::spawn_world_effect_from_coordinate_sequence()` | partial |
| `0x1E4AC` | `spawn_world_effect_at_player_position` | Allocates the flagged player-relative effect and places it at the player X coordinate plus `0x80`. | `ghidra_conversion::spawn_world_effect_at_player_position()` | partial |
| `0x44BC` | `initialize_entity_record_from_template` | Seeds the shared entity record from the template, installs slot/state/position fields, selects the low-nibble or scene-`0x37` variant, raises the special effect flag, and exposes the source override list as a host boundary. | `ghidra_conversion::initialize_entity_record_from_template()` | partial |
| `0x4466` | `spawn_entity_template_list` | Consumes the terminated template list, allocates paired slots, resets or advances the `0xC0` vertical origin, marks root records, and links child records to the current root. | `ghidra_conversion::spawn_entity_template_list()` | partial |
| `0x43B6` | `process_pending_entity_template_lists` | Services the scene-2 boundary, prepares pending data, drains the generated list, applies selected-list timeout behavior, and drains the default list in source order. | `ghidra_conversion::process_pending_entity_template_lists()` | partial |
| `0x4388` | `finalize_pending_entity_template_update` | Runs the pending-list coordinator and installs the mode-2 alternate or mode-1 primary state marker. | `ghidra_conversion::finalize_pending_entity_template_update()` | partial |
| `0xA800` | `prepare_pending_entity_template_list` | Applies the source gate order, increments the one-shot selection counter, selects the generated list, or installs the appropriate static fallback list. | `ghidra_conversion::prepare_pending_entity_template_list()` | partial |
| `0x5BA98` | `select_pending_entity_template_list` | Selects the scene-relative list row and one-based counter entry from typed host tables. | `ghidra_conversion::select_pending_entity_template_list()` | partial |
| `0x5BA50` | `load_scene_resources` | Decodes the scene resource, exposes it to the presentation/import boundary, installs the three source index entries at the typed stride, and publishes readiness. | `ghidra_conversion::load_scene_resources()` | partial |
| `0x5BA66` | `install_scene_resource_index_entries` | Installs the three source index pairs at offsets `0`, `stride`, and `stride * 2`, then sets the resource-ready state. | `ghidra_conversion::install_scene_resource_index_entries()` | partial |
| `0x1530` | `decode_and_upload_source_stream` | Decodes the source stream and performs the original VDP upload; native conversion keeps only the decoded host bytes and an explicit upload boundary. | `ghidra_conversion::decode_source_byte_stream()` plus presentation boundary | partial |
| `0x14902` | `compute_obstruction_grid_index` | Converts world X/Y into the source obstruction-grid index using a `0x200` horizontal unit and `0x40`-byte row stride. | `ghidra_conversion::compute_obstruction_grid_index()` | partial |
| `0x14B4E` | `decode_obstruction_tile_projection` | Decodes packed tile coordinates into host X/Y positions offset by `0x100`, then selects horizontal `2/6` or vertical `4/0` from the reference tile relation. | `ghidra_conversion::decode_obstruction_tile_projection()` | partial |
| `0x14C56` | `project_heading_position` | Applies the extracted eight-entry signed vector table at `0xB2F6` to the current world position using the heading word's top three bits. | `ghidra_conversion::project_heading_position()` | partial |
| `0x14C2E` | `derive_entity_axis_relation_flags` | Clears the relation byte, sets bit 0 when entity X is no greater than target X, and sets bit 1 for the equivalent Y comparison. | `ghidra_conversion::derive_entity_axis_relation_flags()` | partial |
| `0x15A8C` | `find_first_available_entity_slot` | Scans six `0x100`-stride source records for the first clear active bit and returns slot `9..14`, or `15` when full. | `ghidra_conversion::find_first_available_entity_slot()` | partial |
| `0x195C` | `sample_controller_input` | Samples controller words, derives newly asserted bits, and updates decoded directional state. | `ghidra_conversion::derive_controller_sample()` | partial |
| `0x19F2` | `read_controller_port` | Acquires the controller bus, reads phase `0x00` and `0x40`, combines the six-button byte, and releases the bus. | `ghidra_conversion::read_controller_port()`; host input and bus operations remain callbacks | partial |
| `0x17708` | `render_field_hud` | Sets the field display mode, renders the HUD sprite stream, uploads five tile blocks, refreshes HUD state, and conditionally renders the mode-8 panel. | `ghidra_conversion::render_field_hud()`; presentation operations remain callbacks | partial |
| `0x530DE` | `service_audio_if_enabled` | Gates the source audio queue service on the enable flag, preserving bus acquire, command flush, append, and release order. | `ghidra_conversion::service_audio_if_enabled()`; host audio backend remains callbacks | partial |
| `0x1BAC` | `finalize_title_input_sequence` | Recognizes the seven-byte title input sequence or clears the bounded history. | `ghidra_conversion::finalize_title_input_sequence()` | partial |
| `0x1BEA` | `record_title_input_edge` | Stores up to eight nonzero title-loop edge bytes in the source event buffer. | `ghidra_conversion::record_title_input_edge()` | partial |
| `0x1C1E` | `advance_title_text_state` | Counts down the completed-logo text delay, steps the target palette every 32 counts, then emits the completed title text setup. | `ghidra_conversion::advance_title_text_state()`; palette/copyright presentation remain boundaries | partial |
| `0x1C6A` | `advance_title_sprite_state` | Advances the title sprite cadence/phase and requests a SAT projection rebuild. | `ghidra_conversion::advance_title_sprite_state()`; sprite projection remains a presentation boundary | partial |
| `0x1D7A` | `setup_title_state` | Clears four title words, runs the resource/font setup order, publishes palette bits `0xD`, and advances the palette for 17 frames. | `ghidra_conversion::setup_title_state()`; resource operations and palette presentation remain callbacks | partial |
| `0x1DCA` | `draw_title_prompt_variant_a` | Submits the primary title prompt record to the common text writer at tile base `0xCA84` with palette flags `0x2000`. | `ghidra_conversion::draw_title_prompt_variant_a()`; source text writer remains a callback | partial |
| `0x1DE0` | `draw_title_prompt_variant_b` | Submits the alternate title prompt record to the common text writer at tile base `0xCA84` with palette flags `0x2000`. | `ghidra_conversion::draw_title_prompt_variant_b()`; source text writer remains a callback | partial |
| `0x1DF6` | `draw_title_copyright` | Submits four copyright records at tile bases `0xCB04`, `0xCB84`, `0xCC04`, and `0xCC84`. | `ghidra_conversion::draw_title_copyright()`; source text writer remains a callback | partial |
| `0x03032` | `scene_uses_alternate_profile` | Accepts scene IDs `6..9` and `0x2C..0x31`, rejecting the gaps and all other signed values. | `ghidra_conversion::scene_uses_alternate_profile()` | partial |
| `0x02A32` | `initialize_field_mode_profile_a` | Selects profile A, runs the common mode-entry service, clears seven transient fields and the render-pending marker, then initializes scene-mode state. | `ghidra_conversion::initialize_field_mode(..., FieldModeProfile::profile_a, ...)` | partial |
| `0x02A44` | `initialize_field_mode_profile_b` | Performs the same typed initialization sequence for profile B. | `ghidra_conversion::initialize_field_mode(..., FieldModeProfile::profile_b, ...)` | partial |
| `0x02A94` | `initialize_mode_nine` | Runs the common mode-entry service, selects the mode resource, publishes mode 9, resets the field marker to `0xFFFF`, and establishes the source HUD/input/scene flags. | `ghidra_conversion::initialize_mode_nine()` | partial |
| `0x10918` | `prepare_menu_graphics` | Sets the menu transfer display command, runs the common buffer preparation, copies eight source metadata words, marks the resource ready, and enters the decoded-resource upload boundary. | `ghidra_conversion::prepare_menu_graphics()` | partial |
| `0x10984` | `transfer_menu_asset_batch_nine_optional` | Runs the common preparation, uploads nine tile blocks plus a tenth block when the source flag is set, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the nine-plus-optional plan | partial |
| `0x10A7E` | `transfer_menu_asset_batch_four_post_prepare` | Runs preparation, repeats the transfer command, uploads four tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the four-block post-preparation plan | partial |
| `0x10AF0` / `0x10B5A` | `transfer_menu_asset_batch_four` | Runs preparation, uploads four tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` | partial |
| `0x10BC4` | `transfer_menu_asset_batch_eight_after_copy` | Runs preparation, performs the source buffer-copy hook, uploads eight tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the eight-block copy plan | partial |
| `0x10C96` | `transfer_menu_asset_batch_eight` | Runs preparation, uploads eight tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` | partial |
| `0x1A304` | `clear_effect_slot` | Clears one of the eighteen effect-slot occupancy bytes. | `ghidra_conversion::clear_effect_slot()` | partial |
| `0x1A310` | `clear_effect_slots` | Clears all eighteen effect-slot occupancy bytes. | `ghidra_conversion::clear_effect_slots()` | partial |
| `0x1A322` | `clear_entity_effect_pending_records` | Clears the global effect marker and bit 0 across thirteen source entity records. | `ghidra_conversion::clear_entity_effect_pending_records()` | partial |
| `0x13AF2` | `find_first_entity_action_below_threshold` | Scans eight entity action/item bytes for the first value below `0x0D`, returning zero when none qualifies. | `ghidra_conversion::find_first_entity_action_below_threshold()` | partial |
| `0x13A40` | `copy_entity_anchor_position` | Copies the source entity's two-word anchor position into the destination record. | `ghidra_conversion::copy_entity_anchor_position()` | partial |
| `0x05622` | `clear_entity_runtime_fields` | Clears one transient field across ten entity records and masks the paired flags with the caller value. | `ghidra_conversion::clear_entity_runtime_fields()` | partial |
| `0x0E9C` | `make_vdp_transfer_command` | Splits a source transfer address into the two display-control words used by the Genesis transfer boundary. | `ghidra_conversion::make_vdp_transfer_command()` | partial |
| `0x0DFC` | `write_repeated_words` | Emits one word for an explicit host count through the transfer boundary. | `ghidra_conversion::write_repeated_words()` | partial |
| `0x0E18` | `write_word_stream` | Emits a bounded ordered source word stream through the transfer boundary. | `ghidra_conversion::write_word_stream()` | partial |
| `0x1056` | `clear_vdp_words` | Emits sixteen zero words through the transfer boundary. | `ghidra_conversion::clear_vdp_words()` | partial |
| `0x164DC` | `clear_scene_record_transition_words` | Clears the leading transition word in ten scene records. | `ghidra_conversion::clear_scene_record_transition_words()` | partial |
| `0x16DEE` | `clear_scene_record_pending_flags` | Clears bit 0 in the ten scene-record pending flags. | `ghidra_conversion::clear_scene_record_pending_flags()` | partial |
| `0x16E08` | `all_scene_record_slots_active` | Tests bit 0 across all ten scene-record slots. | `ghidra_conversion::all_scene_record_slots_active()` | partial |
| `0x16EA0` | `fill_terrain_work_region` | Fills a bounded rectangular terrain work-map region with value `2`. | `ghidra_conversion::fill_terrain_work_region()` | partial |
| `0x176EA` | `render_menu_footer` | Copies the footer source block, marks it dirty, refreshes menu metadata, and draws the footer in source order. | `ghidra_conversion::render_menu_footer()` | partial |
| `0x177A4` | `initialize_menu_slot_states` | Sets status `6` and timer `1` for all five menu slots. | `ghidra_conversion::initialize_menu_slot_states()` | partial |
| `0x111D0` | `upload_menu_tile_block` | Uploads one menu tile block through the presentation boundary. | `ghidra_conversion::upload_menu_tile_block()` | partial |
| `0x11454` | `reset_menu_upload_cursor_and_upload` | Clears the menu upload cursor and uploads one tile block. | `ghidra_conversion::reset_menu_upload_cursor_and_upload()` | partial |
| `0x17EE4` | `reset_menu_overlay_and_upload_two` | Clears the menu overlay state and uploads two tile blocks. | `ghidra_conversion::reset_menu_overlay_and_upload_two()` | partial |
| `0x1E4E` | `reveal_title_logo_column` | Consumes one two-byte logo descriptor, requests its column render, and enters the completed-logo state at column `0x25`. | `ghidra_conversion::advance_title_logo_column()`; source blitters and 16-word palette copy remain events | partial |
| `0x2634` | `cycle_title_circuit_lights` | Every fifth vblank, selects one of seven eight-word title light palettes. | `ghidra_conversion::advance_title_circuit_lights()`; palette write boundary remains explicit | partial |
| `0x26D0` | `advance_title_animation_timer` | Advances the title animation timer and sets the source update flag on wrap. | `ghidra_conversion::advance_title_animation_timer()` | partial |
| `0x1A9E` | `run_title_loop` | Source-order title frame: vblank, completed-logo text state, seven-phase circuit cadence, five-frame animation timer, controller sample/edge/latch, delayed logo-column reveal, six-phase sprite projection, prompt variant selection, and post-loop palette/input finalization. | `ghidra_conversion::advance_title_loop_frame()` composes the converted leaf routines; Qt consumes typed presentation requests | partial |
| `0x3084` | `run_field_loop` | Field update loop around movement, entity state, camera, and object output. | `NativePortShell::update()` field branch | partial |
| `0x5D746` | `run_pre_title_sequence` | Runs the source startup loop: typed resource setup, vblank cadence, logo strobe, 224-line scroll generation, sprite projection, audio gates, palette/input checkpoints, final cleanup, and the settling wait. | `ghidra_conversion::run_pre_title_sequence()`; unresolved child operations remain explicit callbacks | partial |
| `0x5DA50` | `setup_pre_title_resources` | Performs the recovered clear/prepare/zeroed-stream/field-refresh/decode/palette/grid child calls in source order and marks the typed dirty flags. | `ghidra_conversion::setup_pre_title_resources()`; host transfer callbacks remain explicit | partial |
| `0x5DAC4` | `fill_boot_horizontal_scroll` | Advances the converted PRNG 224 times, stores `(random >> 5) & 0x1f` per line, and invokes the typed horizontal-scroll stream boundary. | `ghidra_conversion::fill_pre_title_horizontal_scroll()` | partial |
| `0x5D8BA` | `step_pre_title_logo_strobe` | Decrements the five-tick strobe cadence, wraps the eight-phase palette selection, marks the dirty flag, or steps the entity color words at cadence two. | `ghidra_conversion::advance_pre_title_logo_strobe()` | partial |
| `0x5D938` | `advance_pre_title_animation` | Sets the source sprite markers, builds the two ordinary title records, delegates the six-phase projection gate, and sets the projection marker. | `ghidra_conversion::advance_pre_title_animation()` | partial |
| `0x5D996` | `advance_pre_title_logo_phase` | Checks the `FFDD30`/`FFDD3C` gates, advances typed `FFE066`/`FFE064` phase state, and builds two projection records on every open gate. | `ghidra_conversion::advance_pre_title_animation()`; source phase state remains caller-owned | partial |

### Command-overview refresh correction: `0x11592`

The Ghidra C export for `0x11592` is not behaviorally reliable at its
branch. The ROM disassembly is authoritative here. The routine:

1. derives the cursor sprite offsets from the command-overview selectors:
   `primary * 0x28` in `D1` and `secondary * 0x18` in `D2`;
2. initializes the sprite-record count to one, expands the `0xD29F6`
   cursor records with first sequence `2` and attribute flags `0x2000`, backs
   up to the final record and clears its sequence byte, then publishes the
   sprite-list-ready marker and `FFEF14` bit 1;
3. prepares the menu grid and reads the selected entity action through the
   typed `0x12148` boundary;
4. for a nonzero action, renders item action details, calls the entity-action
   service when the `CMP.B #0x10` result is negative, and prepares the menu
   dialog; and
5. for action zero, renders the table at source `0xD1CA5` to tile base
   `0xC520` with flags `0x6000`.

The current conversion is
`ghidra_conversion::refresh_command_overview_action_details()`. It carries
the two selector-derived offsets and all branch callbacks as typed values.
The host extraction alias for the `0xD1CA5` action-title table is still an
explicit data-extraction gap; no runtime ROM lookup or invented asset is
being used to hide it. This also corrects the stale Ghidra-C interpretation
that branches on the prior `FFEF14` bit instead of the action-code read.

The adjacent page drivers are now traced as well. `0x11442` preserves the
caller's signed `D0` through the upload leaf; that value is the navigation
delta consumed by `0x114A0` and `0x11506`. The primary driver wraps the
seven-page action selector, marks page two for the panel path, refreshes
details only for pages zero and one, and otherwise resets the VDP/menu upload
cursor. The secondary driver uses `FFDD22` bits 1, 3, and 2 to suppress the
one-shot action dialog on navigation edges, arms/consumes `FFF115`, wraps a
negative secondary-page result to zero, and maps page four back to page three.
These are typed state transitions and callbacks in
`ghidra_conversion::advance_command_overview_primary_page()` and
`advance_command_overview_secondary_page()`; they do not emulate the VDP or
read Genesis memory.

### Selected item/action detail renderer: `0x12A8C`

The ROM trace for `0x12A8C` identifies this as the equipment command-overview
selected-item renderer, not a generic panel copy. It receives the one-based
item code in `D0`, calls `0x12A54` to select the resource variant at the
threshold of thirteen, and restores the incoming `D0` before returning. The
selected row is a `0x30`-byte record from the `0xD4540` item table: `+0x1E`
is the optional item-art resource and `+0x24` is the ordinary detail value.

When art is present, the routine writes the item art at source tile base
`0xC1A4` with source loop values `D1=4`, `D7=8`, then always renders the item
name at `0xC420`/`0x6000`. It reads the selected entity action through
`0x12148`. For action codes below `0x0D`, it checks the active record byte at
`+0x6E + D7` in the exact order `1, 2, 0, 3, 4, 5`; the paired overlay
destinations are `0xC19C`, `0xC19C`, `0xC1B6`, `0xC1B6`, `0xC336`, and
`0xC336`, using `D1=1`, `D7=3`, and palette flags `0x2000`.

The remaining action bands are behaviorally meaningful: `0x0D..0x13` use
the extended label table, `0x14` and `0x18..0x28` use the action-dependent
label table and repeat the `0x12148` lookup, `0x15..0x17` and `0x29..0x2B`
return before the final label/value pair, and `0x2C+` set the armor-selection
marker and call `0x11352`. The completed label path renders at `0xC31C` with
`0x4000`; ordinary actions use the item row's `+0x24` value, while extended
and action-dependent paths use `+0x6E + D7` for the detail-value grid at
`0xC39E`/`0x2000` with source loop values `D1=0`, `D7=1`.

`ghidra_conversion::render_item_action_details()` now expresses this as
typed item-art, text, phase-overlay, label, detail-value, and armor-selection
plans in `equipment_combat_bar_conversion`. Its source-data-bound overload
resolves the one-based code through `UsItemCatalog`: `has_detail_art` is the
semantic form of the signed `+0x1E` pointer test, and the ordinary detail grid
uses the row's typed `+0x24` factor. Rows 41..43 retain the ROM's `-1`
no-art state, and an invalid or identity-mismatched item code reports a
missing source row without invoking presentation callbacks. The original
`0xD25DC..0xD262C`,
`0xD29C4..0xD29D0`, and `0x6AC46` tables remain explicit extraction/data
boundaries; no runtime ROM address lookup or invented asset alias has been
added. If the `0x12148` record lookup is unavailable, the conversion reports
that missing source boundary instead of assuming action zero.

### Equipment action-row selector and renderer: `0x13000` / `0x12EAE`

The disassembly corrects another misleadingly generic family. `0x13000`
always renders the action-detail heading, then selects a thirteen-entry label
row. In source modes below two, the selected action comes from `0x12148`; a
zero action falls back to active-record `+0x79` plus the first set bit in
`+0x90` bits 5..8, contributing values 1..4. A nonzero action is not itself
the label row: the routine reads that action's record `+0x26`, subtracts one,
and clamps the result to 0..12. Higher modes use `+0x9D` directly when
`+0x56` is negative; otherwise they use the active `+0x57` action and the
same `+0x26` detail selector. Missing `+0x26` data is now an explicit source
boundary in `select_entity_action_detail()` rather than an invented reuse of
the action code.

`0x12EAE` is the equipment detail-row controller around that selector. With
no active action (`+0x57 == 0`), cyberware flags bit 3 wins over bit 2 for the
initial detail asset; with neither set it renders `0xD1AA5` at `0xC538` and
uses asset index one. For a higher-mode negative display state it renders the
same header only for active action five, advances `0x12FA4`, then reads the
alternate `0xD4EA0` row `+0x25`; the ordinary path reads `0xD4540` `+0x25`.
A signed-negative row value returns before selection/text/grid output.

The alternate branch now consumes a matching typed `MagicSpellDefinition`
instead of an arbitrary signed byte. The source derives the spell code as
active record `+0x57 - 1`, reads that row's `D4EA0 +0x25` availability value,
and uses it as the alternate detail asset index; a missing or mismatched typed
row is reported as an unresolved source-data boundary. Ordinary item detail
rows continue to receive their signed `D4540 +0x25` value separately.
The source-bound overload now resolves ordinary active action `+0x57` codes
through the typed `UsItemCatalog` (one-based code to zero-based row) and uses
that row's signed `+0x25` value. Out-of-range or identity-mismatched rows stop
with an explicit missing-detail result; the generic signed-byte input remains
an audit adapter.

After the `0x13000` selection, the routine renders the detail text from the
`0xD19B0` family at `0xC51C`/`0x6000`, then writes six rows from the selected
detail asset at `0xC52A`/`0x2000` (`D1=0`, `D7=6`). The conversion is now
`ghidra_conversion::render_equipment_action_detail_row()` in the equipment
menu module, with explicit source-table enums and typed callbacks for the
header, action-menu advance, selected label, detail text, and detail grid.

The higher-mode negative-display branch in `0x12CEE` now consumes the same
typed spell row as `0x12FA4`: active item code six selects D4EA0 row five
(`Stink`), and the residual calculation validates that relationship before
entering `0x10660`. The entity record assembled for the attack-strip math is
named separately from the spell definition, so a missing or mismatched spell
row is visible as a source-data boundary instead of being silently treated as
an arbitrary action record. The remaining attack/defense-bar branches still
need their own full source audit.

### Action-menu residual advance: `0x12FA4` / `0x10660`

The source `0x12FA4` path is not an available-count test. It renders the
action-menu heading from `0xD19B7` at `0xC49C`/`0x6000`, loads active-record
`+0x9D` into `D4`, converts active `+0x57` to the zero-based `D4EA0` row by
subtracting one, and calls `0x10660`. A positive residual selects the
`0xCF8F0 + (residual - 1) * 0x0E` effect row and writes six rows at
`0xC4AA`/`0x2000`; zero or negative residuals stop after the heading.

This also corrected the shared residual conversion: zero is a valid first
`D4EA0` row for `0x10660`, so `calculate_action_residual()` no longer rejects
row index zero. `advance_entity_action_menu()` now takes a typed entity,
current force, and the matching `MagicSpellDefinition` from the US D4EA0
catalog. The active action code must equal `spell_code + 1`; this prevents a
caller from silently pairing the residual math with the wrong spell row. The
typed spell row exposes the decoded difficulty modifier and 166AD4 effect
directly, while the generic action-record overload remains for other audited
action paths. A missing or mismatched spell row is reported as an unresolved
source-data boundary rather than treated as “one action remains.”

### Equipment-details parent dispatcher: `0x111E8`

`0x111E8` is the page-level equipment renderer. When `FFF112` is clear it
does only the existing `0x11928` menu-row render and returns. When set, it
clears the marker, prepares menu grid variant B, then renders the heading at
`0xC2A0`, the active runner race/state row from `0xD19C7` at `0xC1A0`, and
the class row from `0xD19E5` at `0xC1AC`.

Its action-name branch is also explicit: active action zero selects the
cyberware-bit-3 text at `0xC422`, then bit 2 at the same destination, or the
no-action text at `0xC420`; a nonzero action selects the `0xD4EA0` name row
when `+0x56` is negative and the `0xD4540` row otherwise. The wrapped
`CMP.B #0x10` gate invokes `0x12EAE` only for the low-action range. The
dispatcher then builds the physical meter at `0xC2B4`, mental meter at
`0xC334`, calls armor selection, and prepares the menu dialog in that order.

`ghidra_conversion::render_equipment_details()` now captures this parent
sequence, returns the nested `0x12EAE` result, and keeps text/meter source
rows and presentation callbacks typed. Its source-data-bound overload passes
the `UsItemCatalog` into the nested renderer, so ordinary low-action detail
rows resolve their signed `D4540 +0x25` value at the page boundary instead of
relying on a stale `EquipmentDetailsInput` field. It is still a
conversion/reference module; none of these callbacks are native-port runtime
plumbing.

The combat conversion now binds the extracted seven-row `D4D50` armor catalog
at the entity-action service boundary. `0x559BC` reads the target's typed
`+0x76` armor ordinal for primary/secondary subtraction, while `0x55C76`
uses the reaction target record for its secondary pass. The binding preserves
the source low-byte subtraction and keeps the catalog dependency in core data,
separate from menu presentation code.

## Initial coherent C++ conversions

`0x0330` and `0x0436` are no longer represented only as bus traces. The
native layer now exposes `StartupDisplayPlan` and `StartupTextTransfer` in
`../include/shadowrun/boot.hpp`; `boot.cpp` implements the transformations as
ordinary C++ data construction. `ShadowrunBoot` retains a narrow bus adapter
for source-verification tests, while the host result is independent of Genesis
addressed memory.

## Mode resource copy family: 0x566C2 / 0x566D8..0x5670E

The direct ROM sweep corrected several misleading labels in this adjacent
family. `0x566C2` copies 24 longwords from the fixed source table at
`0x18AC56` into the mode-selection working table at `FFD91E`; it is not a
metadata refresh. `0x566D8`, `0x566E2`, `0x566EC`, and `0x566F6` are four
entry points into one eight-longword copy loop at `0x566FC`, targeting
`FFD87E`, `FFD89E`, `FFD8BE`, and `FFD8DE` respectively and setting `FFEF14`
bit 0. `0x5670E` copies the same 24-longword source block into the latter
three destinations in one pass. The typed conversion is therefore one
`ModeResourcePaletteBanks` model plus `copy_mode_resource_palette_bank()` and
`copy_mode_palette_block()`, with the entry-point bank selected explicitly;
there are no four duplicated host routines or source-address lookups.

## Field mode service conversion: 0x56AC0 / 0x566EC

The per-frame display service at `0x5672C` is now separate from the outer
`0x56AC0` input loop. It first honors the `FFE18A` suppression flag and the
status-`0x14` early exit, then selects display mode 1, display mode 2, or the
status-specific mode-0 cadence. The typed conversion preserves the source
counter resets and wraps: status 0 uses six one-word frames; statuses 6..9
use three three-word frames; statuses 4 and 5 use their own three-frame
tables; statuses 44..49 use four overlapping four-word windows; mode 1 uses
four overlapping four-word windows; and mode 2 advances an eight-position
direction/index value only when the low nibble of `FFDF88` is `0xF`. The
result is a named presentation-write record, not a VDP emulator or a random
address write.

`0x56AC0` is the outer field-mode service controller, not an intro routine.
Its source-derived flow is now represented by
`ghidra_conversion::run_field_mode_service()` in the separate conversion
library. The conversion preserves the initial resource/palette setup, the
mode-result-specific resource branches, the numeric-menu setup, result-1
sprite-panel path, normal grid/text path, and the vblank/latch/input
precedence. The old “panel snapshot” label for `0x566EC` was wrong:

* `0x566EC` copies eight longwords (sixteen CRAM colors) into `FFD8BE`, which
  is palette bank 2, colors 32..47 of the 64-word palette upload buffer. It
  then sets `FFEF14` bit 0, publishing that a subsequent palette service must
  upload the changed full palette.
* `0x56AC0` derives the bank selector from the active runner/entity record at
  `FFFC04`: it reads record `+0x2C`, divides by `0x2000`, subtracts one, and
  indexes the ROM palette-source table at `0x18ACB6` before calling `0x566EC`.
  The exact physical interpretation of record `+0x2C` is still a data-table
  investigation, but its role in this call is proven: it chooses the active
  field/HUD palette bank.
* `0x17F8E` (`draw_hud_footer`) reaches the same helper after selecting a
  bank from the active record. Thus this is shared palette publication for
  field/HUD presentation, not a copy of panel geometry or menu state.

The service's source-derived flow is:

1. primary decrement (`0x04`), panel decrement (`0x01`), primary increment
   (`0x08`), and panel increment (`0x02`) are processed in source order;
2. confirm (`0x40` or `0x10`) is handled after navigation;
3. restart (`0x20`) re-enters setup through the graphics-service boundary;
4. return (`0x80`) restores the field asset sequence, animation assets, and
   field setup before returning.

`0x56DC4` remains an explicit typed action-handler callback. `0x587DA` is now
converted under the explicit field-action presentation names in
`shadowrun_ghidra_field_action_selection_result_presentation` and is no longer
described as an anonymous renderer:

* It first saves the action-row cursor (`FFF0D0`) into the selected-action
  field (`FFF0B8`).
* A negative service result dispatches action cursor 3 to a six-grid/two-grid
  action-detail batch, cursors 4+ to eight active-runner inventory-item glyph
  submissions (with the selected-action-4 text pass), and lower cursors to
  the eight three-byte action-detail records at the selected source table.
  Each record uses a glyph triple unless its leading marker is exactly `E`,
  in which case it writes one detail grid.
* The equipment-upgrade result writes eight grids. Other positive results use
  action cursor 3 for field-resource-record purchase rows, cursor 5 for the
  field-quantity purchase split, and other cursors for the field-resource
  purchase split. Cursors below 3 first use `0x5763C`: `FFF0B6 == 6` selects
  one menu-resource record table and every other menu mode selects the other,
  then `FFF0B8` chooses a record at a `0x10` stride. The related `0x57628`
  helper adds the action cursor times two bytes to that record for action-path
  word access; it is also used by the field-action and Matrix/program-storage
  code, so the native abstraction is a shared menu-resource selector rather
  than a field-only offset helper.
* A zero result uses the active-runner inventory-item glyph row for action
  cursor 3; otherwise it selects one of the ordinary field-action record
  tables and runs the alternating glyph/detail-grid loop.

The adjacent navigation leaves at `0x58C54` and `0x58CDE` were audited
against their byte instructions as well. Their deltas are `ADD.B`, so the
conversion wraps the low byte before testing the source sign and bounds. A
negative result or a value beyond the panel's upper bound is clamped and
returns without a redraw; only an in-range primary/panel write invokes the
normal grid/text or alternate grid/sprite refresh. This distinction matters
for the menu's visual update cadence and is covered by the navigation test.

The focused tests `shadowrun_ghidra_field_mode_service_loop` and
`shadowrun_ghidra_field_action_selection_result_presentation` cover the palette dirty
publication, service-loop input precedence, and every presentation branch.

## Command-overview and shared active-runner HUD panels: 0x110BC / 0x110E8 / 0x110EC

`0x110BC` is the command-overview panel entry, not a generic wrapper. It reads
the active record's `+0x3E` and `+0x2C`, computes `D5 = (+0x3E * 0x50) +
0x8001` (word arithmetic), sets `D4=0xE104`, `D7=7`, and `D1=9`, then calls
`0x03648`. Because `0x03648` uses DBF counters, this is ten rows of eight
words. The native conversion exposes that as `CommandOverviewHudPanelPlan`.

`0x110EC` is not meaningfully “menu HUD panel wrapper B.” Its body prepares
the common active-runner HUD panel draw and then calls `0x03648`:

* The active record's `+0x3E` value selects one of three tile origins (`-0x796C`,
  `-0x7956`, or `-0x7940`).
* The preceding `0x110E8` shared-entry prelude loads `+0x2C` into D2. Direct
  callers of `0x110EC` prepare D2 themselves.
* The caller's D4 is preserved as the panel destination; `0x110EC` does not
  initialize it.
* The draw submits four rows of four words through the shared HUD-panel leaf.

The callers identify where the resulting panel is used: `0x0D01C` is the
attributes-and-skills menu, `0x56AC0` is the field-mode service menu, and the
`0x110E8` entry is reached from the inventory/action controller at `0x11740`.
So the useful native abstraction is a shared active-runner HUD panel layout,
with the caller supplying the surrounding menu state; it is not three
different “variant B” menus.

`0x1111A` is the active-runner name presentation: it uses the active record's
`+0x58` encoded text source, destination `0xC0A0`, and `0x6000` attributes.
The text decoder remains the separate `0xC0DA` conversion boundary.

`0x1170C` supplies the command-label table at `0xD1D3A` and enters `0x11712`.
`0x11712` selects a caller-supplied table at 0x12-byte records when
`FFF0D0 >= 2`; selectors 0 and 1 instead use the first Inventory record at
`0xD1D28`. The conversion now exposes the table alias, record index, byte
offset, destination `0xEC9C`, and `0x6000` attributes instead of a callback
with no source/layout information.

## Field action cost conversion: 0x5787C / 0x57A44 / 0x57A6E / 0x57AF6 / 0x57B04 / 0x57B42

The routines around `0x5787C` were mislabeled as an intro family. They are
field-menu cost logic called by the selected-action presentation/handler
cluster around `0x5794E` and `0x57C7A`. They are now represented by the
separate `shadowrun_ghidra_field_action_cost` conversion library in
`field_action_cost_conversion.*`:

* `0x5787C` calculates a quantity cost from the primary action category:
  category 0 is `quantity^3 * 25`, categories 1 and 2 are
  `quantity^2 * 3 * 100`, and category 3+ is `quantity^2 * 2 * 100`.
* `0x57A44` reads a selected three-byte action record. Its first byte is a
  zero-based row index into the fifty `0x30`-byte US records at `0xD4540`,
  where the base rate is the word at record offset `+0x22`. The conversion now
  exposes that complete confirmed item slice as `UsItemCatalog`; ordinary
  one-based item lookup remains a separate helper. A nonzero third byte scales
  the selected rate by `quantity / 10`, with the source low-word truncation
  preserved explicitly.
* `0x57A6E` applies the mode-specific feature flags before charging/display:
  mode 4 plus `FF018F` bit `0x40` removes 10%, mode 5 plus bit `0x80` removes
  25%, mode 2 plus bit `0x04` halves the amount, and mode 8 plus bit `0x02`
  halves it. It then subtracts the runner adjustment and publishes `FFF0BE`.
* `0x57B04` is the shared runner adjustment that Ghidra failed to recover
  because its result stays in D3. It subtracts `1AAC6E[FFF0B6]` from active-
  runner record `+0x8C` as a byte, preserves the `SUB.B`/`BLE` N/V/Z flags,
  caps the surviving cleared-word margin at ten, and calculates
  `(cost >> 5) * margin`. Its overflow case is not equivalent to a host
  `int8_t` comparison.
* `0x57AF6` is the companion display path that adds that adjustment instead
  of subtracting it.
* `0x57B42` is selected by the large-cost callers after their amount reaches
  `0x10000`. It calculates `(cost >> 8) * margin` once and subtracts that
  same value ten times; it does not recompute the shifting amount each pass.

The native code takes typed mode/feature/runner values and the conversion-owned
US item catalog. The callback overload remains only as an explicit audit
adapter for alternate tables. It has no ROM-address reads, emulated registers,
or dependency on the runtime/emulator. The exact game-facing names of the four
mode categories and the active-runner `+0x8C` stat remain data-table work; the
cost behavior and its confirmed D4540 source data are now explicit and tested.

## Field resource-record purchase conversion: 0x573FC

`0x573FC` is the positive-result saved-selection-3 resource-record purchase
leaf. It selects the `0x1AB196` table for menu mode 6 and the other 0x70-byte
table otherwise, then indexes an 0x0E-byte record by the current category. A
high-bit record marker sets the service-result failure before presentation.
The requirement and funds failure messages both enter the shared controller
wait. The two capacity fields preserve the source word `CMP`/`BMI` result
instead of becoming host unsigned `>=` tests; the typed record copy updates
item/flags, capacities, limits, progress, and the availability requirement in
source order after confirmation.

## Selected field action application: 0x5794E / 0x57B8C / 0x57C5E

`0x5794E` is the mutation half of the field-menu action path, not another
intro helper. Its caller has already selected a record group; the routine
adds `group * 0x18` and `primary_cursor * 3` to reach one three-byte record.
The native conversion is `apply_selected_field_action()` in the separate
`shadowrun_ghidra_field_action_application` library:

* A high-bit first byte is the disabled/invalid marker and sets the source
  failure result `FFF112 = -1` before any state mutation.
* The detail panel refresh (`0x57C5E`) happens before pricing. The ordinary
  record path scans active-runner `+0x66..+0x6D` in ascending slot order and
  renders its no-slot message and waits if all eight slots are occupied.
* Marker `E` is a separate path: its price starts from record byte 2 and it
  increments active-runner `+0x4F` after the confirmation prompt. The source caps
  the visible count at `0x14` and rejects the transaction when the increment
  would reach `0x15`.
* Ordinary records use the `0xD4540`-backed cost calculation, then store the
  selected marker as marker+1 in the empty action slot and copy record byte 1
  into the companion `+0x6E` slot. This marker+1 write is intentional source
  behavior, not a normalization invented by the host layer.
  The source-data-bound overload validates the zero-based record type against
  the typed fifty-row catalog and resolves `+0x22` directly. An out-of-range
  or identity-mismatched row returns `missing_source_data` before the detail
  refresh, cost check, or runner mutation; it cannot become a free purchase.
* Funds are checked before the confirmation prompt; insufficient funds renders
  its message and waits, and funds are deducted only after the prompt accepts.
  Cancel, no-slot, insufficient-funds, and special capacity outcomes leave
  the runner record and funds unchanged. A special capacity rejection first
  restores the visible count to `0x14`, refreshes the detail panel again,
  renders its limit message, and waits through the shared controller-wait
  path before returning.

`0x57B8C` is now labeled `run_menu_confirmation_prompt`: it draws the
two-choice prompt, services vblank/controller input, uses the shared wrapped
0/1 cursor for movement bits 8/9, and exits on the source bits 12/14. On a
movement, the source plays its move sound before calling the wrapped cursor
writer; the typed callback sequence now preserves that order. Option 0 is the
accepting choice; option 1 leaves the action unapplied. It remains
an explicit presentation/input callback in the native application module;
the conversion does not emulate its VDP writes. The grouped field-action grid
conversion now models `0x57C3A/46/52/5E` as the `0xC604/3`, `0xC184/12`,
`0xC904/2`, and `0xC484/6` presets entering shared `0x57C66`; the source row
is 31 words from `0xC1320`, and the detail panel is six rows because `D1 = 5`
is consumed by `DBF`.

## Field-service resource purchase conversion: 0x5766A

`0x5766A` is a field-service purchase/threshold routine reached from the
action handler at `0x56DC4` when the selected service is outside the separate
record-action branches. It was incorrectly classified as an intro routine.
The new `shadowrun_ghidra_field_resource_purchase` library represents its
state in `field_resource_purchase_conversion.*`:

* The valid-category path writes grid variant B before any threshold decision;
  category 4 and malformed high-bit values take the `FFF112 = -1` error exit
  without presentation.
* Categories 0, 1, and 2 compare their progress counters at `FFFBC0`,
  `FFFBC2`, and `FFFBC4` against requested quantity at `FFFBBE` scaled by
  `0x28`, `0x50`, and `0x0A`. Category 3 has a fixed satisfied/limit boundary
  at three units in `FFFBC5`.
  The category-3 test is the source `CMP.B #3` followed by `BMI`: it observes
  the wrapped subtraction's N flag, not a host signed-byte comparison. Thus
  progress `0x80` produces `0x7D` after subtracting three and takes the
  category-limit path.
* A satisfied category takes the category-specific message path, waits, and
  does not charge the player. Category 3 uses a distinct limit message.
* An unsatisfied category reads its word cost from `0x1AC8E0[cursor]`, rejects
  insufficient funds after its message/wait path, publishes the preview cost,
  renders the purchase title and grouped cost in order, and enters the shared
  `0x57B8C` confirmation prompt.
* An accepted purchase deducts the table cost and increments the selected
  progress counter by 10, 25, 5, or 1. Cancellation leaves funds and
  progress unchanged while preserving the published preview cost.

The native model deliberately leaves the exact UI text/category labels at the
data/UI boundary. The ROM proves the four distinct counter/cost behaviors;
it does not justify inventing user-facing names from the generic Ghidra output.

## Recovered field quantity branch: 0x577BA

The field-service dispatcher at `0x56DC4` has a separate `beq 0x577BA`
branch when the secondary service selector is five. The original exported
function list skipped this branch target because it was folded into the
preceding `0x5766A` range, even though `0x577BA` has its own control flow and
returns at `0x5787A`. It is now recorded as `apply_field_quantity_purchase`.

This path is distinct from the progress-counter purchase above:

* It accepts primary categories 0 through 4 and rejects category 5 or above.
* A negative `FFF0B3` availability marker exits immediately and clears the
  marker. Otherwise it writes grid variant B, the purchase title, quantity
  `FFF0D6`, and already-computed cost `FFF0BE` in that source order.
* Insufficient funds renders its message, waits for the controller event, and
  leaves funds and the quantity slots unchanged before clearing `FFF0B3`.
  Confirmation cancellation also leaves funds and the quantity slots
  unchanged before clearing the marker.
* The invalid-category exit writes `-1` only to the source service-result
  marker `FFF112`; it does not clear the armed `FFF0B3` availability marker.
  The native conversion preserves that early-exit mutation boundary.
* An accepted purchase deducts `FFF0BE`, writes `FFF0D6` to `FFFBBE` for
  category 0 or `FFFBC6..FFFBC9` for categories 1 through 4, and clears the
  marker.

Because this target was missing from the generated function inventory, the
Ghidra export plugin's address-name map, function table, call graph, call-site
annotations, and status ledger were updated together with the native module.

The signed-byte audit of the two field-service purchase leaves is now explicit:
the ROM's `0x5766A` and `0x577BA` category gates use `BMI`, not an unsigned
host comparison. Valid categories still map to the same typed slots, while a
malformed high-bit category is rejected at the host boundary instead of being
used as an unchecked array index. Category 2's `0x5766A` threshold also keeps
the ROM's low-byte wrap (`requested_quantity * 10` is compared as a signed
byte), which matters at the 0x19/0x1A boundary. The category-3 threshold is a
separate N-only `CMP.B #3/BMI` predicate; its `0x80` progress case is therefore
source-satisfied even though a normal signed C++ comparison would not be.

## Call annotation policy

Direct calls are annotated with the target address and reviewed symbol.
Indirect calls remain explicit and are not guessed. Current unresolved sites
are recorded in `call-sites.tsv` at `0x0B42`, `0x4658`, `0xC444`, `0x11884`,
`0x15928`, `0x1640A`, `0x19A1C`, `0x1B892`, `0x1E728`, `0x1EF28`, `0x1F26E`,
`0x1F58A`, `0x24E32`, `0x53AA4`, `0x54960`, and `0x5AF48`.

Resolve these only when ROM/data-flow evidence identifies the target. Do not
replace them with guessed aliases.

## World-effect scene driver: 0x1ADE6

`0x1ADE6` is the title/world scene driver, not an unexplained generic world
routine. Its ROM entry performs the scene-owned reset, uploads the decoded
scene source, initializes the selected world-effect mode, attaches descriptor
E, and then enters the frame loop at `0x1AEAC`.

The native reference conversion is
`ghidra_conversion::run_world_effect_scene_loop()` in
`world_effect_scene_loop_conversion.*`. It preserves the source decisions in
typed state and leaves presentation/input work at named callbacks:

* each tick samples vblank/controller/menu-glitch services first;
* an active world sequence (`FFDF8C`) advances until the carry/result from
  `0x1D2E6` hands control to runner selection (`FFF0A9 != 0`) or the mode
  action loop, with audio command `0x05` and the second mode-entry prepare;
* a completed scene (`FFE084`) runs the bounded jitter service and then the HUD
  fade/display-transition return path;
* the ordinary title path gates world-menu, title-resource, directional-input,
  and `0x1A370` world-effect services, then follows the `FF1592` title-service
  bits and the independently active burst/noise/profile/motion/matrix effects;
* world-effect animation is ticked only while `FFE080` is clear, followed by
  the common world-sprite append;
* controller bit 15 takes the Matrix/field refresh branch through `0x1822C`,
  then uses audio commands `0x04` and `0x03`; the resource refresh result is
  an explicit exit condition.

`source_audio_track` is only copied to `FFE189` during the prologue. The
runner-selection branch is controlled separately by `FFF0A9`; it must not be
inferred from the audio track. The conversion has a host-only frame bound for
tests, but the unbounded form follows the source return paths. It does not
enter the emulator or native Qt runtime.

## Title/world interaction service: 0x1C754

`0x1C754` is the input/behavior dispatcher reached once per active scene
frame. Its source-specific behavior is now
`ghidra_conversion::advance_title_world_interaction_service()`:

* `FFE084` installs behavior E as draw `0x1C2A8` / update `0x1C2AE` after a
  fresh behavior-record initialization;
* title-service flag bit 0 blocks the interaction path while the sweep effect
  owns the frame;
* primary behavior descriptor `0x1C36A` advances world-effect activation only
  at stage/phase `3/3`, and otherwise waits unless its stage is the terminal
  value `5`;
* secondary descriptor `0x1C350` uses the same `3/3` gate for the interaction
  update and the same terminal-stage fallthrough;
* only world-effect modes below 3, controller event bits `0x40/0x10/0x20`,
  and an inactive descriptor transition reach the `0x19976` Matrix
  program-action service.

The descriptor values and stage/phase tests are source evidence. The native
conversion exposes behavior installation, world activation, interaction, and
Matrix action as typed boundaries; it does not reproduce the behavior record
or VDP implementation.

## Secondary title/world effect service: 0x1CD5A

`0x1CD5A` is the per-frame service behind `FF1592` bit `0x40`, seeded by
`0x1CB20` when the secondary effect path is selected. It is now represented by
`ghidra_conversion::advance_title_secondary_effect_service()` rather than the
generic former export label.

The source behavior is a three-phase effect with three source-variant groups:

* variant 0 selects source sequences `0x1D906`, `0x1D918`, `0x1D922`;
* variant 1 selects `0x1D934`, `0x1D93E`, `0x1D948`;
* variant 2 selects `0x1D952`, `0x1D95C`, `0x1D966`.

Each frame decrements the lifetime, 20-tick source-variant timer, and 3-tick
phase timer. The current fixed-point position is converted to host sprite
coordinates before the named asset is submitted. A zero frame timer seeds the
paired `FFE088..FFE08E` positions for the title-effect motion; the next source
variant seeds the six `FFE090..FFE0A6` positions, and variant 2 advances both
the six-sprite motion (`0x1CF4A`) and paired motion (`0x1CEFA`).

When the lifetime crosses below zero, the service clears the `FF1592` bit,
resets `FFE0D6`, optionally arms `FFE0AA=0x0A` for the alternate motion, and
hands control to `0x19F18` menu progression after the final sprite build. The
conversion exposes that handoff as a callback and resolves the nine source
sequences to host assets; it does not perform ROM/VDP lookups at runtime.

## Title/world reveal transfer service: 0x1D1F4

`0x1D1F4` is the service reached by `FF1592` bit `0x10`, not an unidentified
world routine. It progressively reveals the title/world resource by writing
`0x47` masked longwords per active frame. The random source index is bounded by
`FFE006`, aligned to an even four-byte source offset, and added to the
`FFDF76` destination base. The mask is the source `0x1D3F4` expansion with
selector `5`; the host conversion emits each result as a
`TitleRevealWordWrite` record rather than recreating a VDP command/data port.

When `FFE008` crosses below zero, the service clears behavior-record flag
`+0x12` bit `0x10`, refreshes the source block once, and then follows the
actual scene branch: `FFE16B != 0` installs the fixed world-sequence state used
by `0x1D280/0x1D2E6`, otherwise control is handed to `0x1A01E` menu-selection
and text setup. The native boundary exposes both outcomes as typed actions and
keeps the decoded source asset behind callbacks.

## Generic export label pass and ROM verification baseline: 2026-09-26

The canonical export had 119 family/address labels left at
`convert_*_routine_0x...`. Those labels have now been replaced in the exporter
name table and synchronized export snapshots, including every generated C
function header, direct-call annotation, call-site row, and call-graph edge.
The review ledger is
`decompilation/docs/ghidra-function-label-review.tsv`; it records the behavior
claim, confidence, and disassembly/Ghidra-C evidence for every one of the 119
entries. Interrupt-vector STOP stubs and empty callback targets are explicitly
classified as reserved/no-op boundaries rather than being promoted into game
systems.

This checkout does not contain a Ghidra `analyzeHeadless` installation, so the
snapshot synchronization changed names/comments only; it did not regenerate
the ROM disassembly or function bytes. A fresh headless export remains a
separate environment-dependent check before treating the snapshot as newly
generated evidence.

Important source checks from this pass:

* `0x104C8` computes its action-table byte offset with `MULU.W #6` followed
  by `ADD.B` for the menu column, then enters the shared `0x104F6` phase/item
  path. The typed lookup now preserves the product's upper byte while wrapping
  only the byte-sized addition; it scans possession slots 6 through 0 for the
  selected action item and can now pass the decoded C8AE0/D4540 tables into
  the source-bound slot-7 continuation. A high-row test covers the index
  distinction, while a typed menu test covers the actual item-41 word path.
* `0x104F6` is `apply_action_item_phase_modifiers`. The generated Ghidra C
  stops after the slot-6..0 scan, but the ROM disassembly continues into the
  slot-7 item `0x29..0x2B` lookup at `0x1050E..0x1055A`. The typed conversion
  already includes that continuation; generated C alone is not sufficient
  evidence.
* `0x12C74` is `merge_equipment_action_skill_value`. The generated Ghidra C
  is a return-only body even though the ROM selects the maximum of `+0x81` and
  the action-class bucket `+0x82/+0x83/+0x84`, then performs the lower-value
  merge plus zero-to-one correction. The typed equipment conversion now takes
  the action code and all four source skill bytes, so the selector is not
  hidden in a precomputed generic skill. This is recorded as a decompiler
  omission, not treated as a no-op. The source trace is complete: `BPL` after
  `CMP.B #8` and `CMP.B #11` selects the action bands, `CMP.B` against `+0x81`
  selects the candidate only when its wrapped byte difference is negative,
  and `CMP.W`/`ADD.W` merges the selected skill with the incoming D6 word.
  The conversion tests cover the band boundaries, signed-byte wrap, word
  overflow, and the zero-to-one result correction.
* `0x14D6A` is not an animation-frame-table selector. `0x13D0C` calls it while
  advancing entity motion, then reads the selected D4540 equipment-item or
  D4EA0 magic-spell row's `+0x27` byte and uses it to choose a pair of movement
  distance thresholds from `0x13E7C`. The typed conversion therefore names
  the result as an action row selection and records the source table family;
  it does not pretend that a Genesis ROM pointer is a runtime animation
  object.
* `0x12148` uses the same source shape with `MULU.W #2` and `ADD.B`: the row
  product's high byte survives while the column wraps the low byte. The typed
  action-code lookup now preserves that word index and tests the `0x80` row
  boundary instead of narrowing the entire selector to eight bits.
* The status ledger is now one row per ROM entry. Eight duplicate rows were
  removed, including the old catalog duplicates for the magic-force functions
  and the duplicate HUD-mode/runner-record entries.
* The callgraph snapshot had two stale aggregate counts for
  `apply_field_quantity_purchase` (`0x577BA`); they were reconciled against
  the 2,986 resolved rows in `call-sites.tsv`, and the audit now checks every
  caller/target pair rather than trusting the aggregate blindly.

`decompilation/tools/verify_rom_conversion.ps1` is the structural gate. The
current baseline is 910 exported entries, 0 generic export labels, 119 reviewed
labels, 932 unique status rows, 2,986 resolved direct call sites, 16 explicit
indirect/unresolved call boundaries, 229 conversion implementation files, 28
behavioral modules, and 230 CTest cases. The gate passes the structural checks,
but behavioral fidelity remains OPEN: 928 status rows are partial and only 4
are defended. The audit therefore does not authorize a true-to-source claim
until the 16 indirect boundaries and the partial hills have been traced and
defended against ROM behavior.

## Instruction-level ROM conversion audit: 2026-09-26

The naming pass exposed a more important problem: a generated function can
have a plausible name and still be a return-only decompiler stub while the
ROM contains the actual behavior. `decompilation/tools/audit_rom_conversion.ps1`
now performs the next audit layer. It indexes the instruction spans in
`disassembly.s`, checks every resolved direct call against the generated C
body, finds explicit `ROM 0x...` evidence in the typed conversion sources,
and joins those results to the one-row-per-entry status ledger. The complete
row-level result is `decompilation/docs/rom-conversion-audit.tsv`.

The current indexed result is 910 entries, 29,141 disassembly instructions,
zero missing ROM spans, 18 native-substitution review rows, 27 generated-C
return-only risks, 20 linear adjacent-export boundary reviews, and 287 entries
without explicit ROM-address evidence in a conversion source. The boundary
classifier also records adjacent entries reached by an explicit branch or
call, but does not mislabel those as linear fall-through. These numbers
describe audit exposure, not percent converted. The structural gate still
passes, but source fidelity remains open.

### Recovered behavior slice

The following entries were traced from the ROM rather than inferred from the
generic label or the generated C:

* `0x566A2` is the action-profile table loader. It copies 5 rows x 17 words
  (0x22 bytes per row) from ROM `0x166AD8` to the working table at `FFF9FA`.
  `entity_action_profile_conversion.cpp` decodes those rows into typed event
  records and two mutable roll-modifier slots.
* `0x55828` is the action-profile selector. It follows the actor record pointer
  at `+0xD8`, reads the linked record's profile index at `+0xA0`, multiplies by
  the 0x22-byte row stride, and selects that row from `FFF9FA`. The typed
  `EntityActionDynamicModifierContext` now carries the table and linked index;
  if either is unavailable, the conversion reports an unresolved profile and
  does not substitute a fabricated roll count or difficulty.
* `0x3602` computes the HUD glyph attribute word from record fields at `+0x57`,
  `+0x9B`, and signed `+0x56`. It returns `0x2000` for the low/ordinary path
  and `0x6000` for the highlighted path. The branch matrix is now a tested
  `select_hud_glyph_attribute_flags` helper; the renderer wiring remains a
  separate audit item.
* `0x60F4` executes `ORI #1,CCR; RTS`, so its observable result is a carry-set
  return used by the descriptor branch at `0x600E`. It is represented by the
  typed `set_sprite_descriptor_carry()` boundary rather than the former empty
  hook. The caller branch still needs a complete native behavior trace.

* `0x1DC22` is a ten-record selected-bit gate, not a generic circular entity
  iterator. When a selected record exists it enters `0x1DC4C` with the
  `0x678CC` cursor sentinel. The typed service conversion now preserves the
  normal/special candidate starts, descriptor fallbacks, eight-record
  non-wrapping scan, previous-target release, and the normal-only `+0x43` clear
  and `+0xAC` back-link publication. `0x1DEA0` now uses its actual `+0x10`
  bit-6/bit-3 and `+0x0C` predicate before invoking `0x1E514`.

### Caller-sensitive fall-through correction

The audit of the world-effect selector found a real conversion error in the
timing helpers. `0x562D4` contains the timer addition and then falls directly
into `0x562DE`; there is no return at `0x562DC`. Therefore the action-count
caller at `0x56284` receives both `FFE169` and twice `FFE16A`. The transition
caller at `0x56316`, and the intro callers at `0x5614C`/`0x5618A`, enter
`0x562DE` directly and receive phase only.

The old `run_thresholded_roll` conversion treated its non-phase branch as
timer-only, which was wrong for both the action-count path and the transition
path. It now calls the combined native timing helper for the `0x562D4`
entry-path and phase-only for direct `0x562DE` callers. The focused test uses
action timer 3/phase 2 and expects difficulty 7, and delayed transition phase
9/base 4 and expects difficulty 22. This is a source-fidelity correction,
not a new game rule.

The audit now records 20 adjacent-export boundaries whose final instruction is
not an explicit control transfer. Boundaries such as `0x3D4A -> 0x3D80` and
`0x3DDE -> 0x3E14` contain explicit branch/call entries into the adjacent
shared transfer bodies and are therefore not linear fall-through risks. The
two confirmed arithmetic fall-throughs remain `0x1952C -> 0x19538` and
`0x562D4 -> 0x562DE`; `0x52E96 -> 0x52EA2` is a shared inner audio entry with
the same source-level boundary shape, now documented and tested separately.

The field-column trace also corrected a conversion omission. `0x3D4A`
calculates the pre-alignment count, emits that segment through `0x3D80`,
aligns `D4`, and then continues into the common tail for the remainder. The
typed host renderer now preserves both transfer segments and its focused test
checks the two-entry case. Matrix storage similarly treats the ROM `0xE2DF2`
footprint table as immutable source data, and the audio `0x52FD2` inner entry
is callable without a redundant bus-begin operation.

These changes deliberately do not modify the native port. They are conversion
evidence and typed source contracts only. The generated Ghidra C for the first
three routines is still incomplete in places; the ROM disassembly remains the
tie-breaker, and no entry is marked defended merely because this helper now
exists.

### Magic force/detail presentation conversion

The ROM pass for `0x10172` (`refresh_magic_force_display`) is now represented by
`ghidra_conversion::refresh_magic_force_display`. It preserves the five source
operations and both predicates: the first repeated-grid write is unconditional,
the second occurs only when the current force is at least two, the normal grid
write follows, the fourth operation occurs only when maximum and current force
differ, and the final repeated-grid write is unconditional. The grid operations
are callbacks because they are presentation services, not ROM-memory operations.

The ROM pass for `0x102D4` (`prepare_magic_action_details`) is now represented by
`ghidra_conversion::prepare_magic_action_details`. It receives the selected
typed `MagicSpellDefinition` and passes that row to availability rendering. Its
typed callback sequence is grid preparation, encoded spell text three times,
runner posture adjustment, spell availability rendering, and grouped decimal
glyph construction. The three text calls remain explicit to avoid collapsing
source behavior into a guessed repeat parameter. Focused tests assert both the
callback sequence and the spell-row identity. These entries remain `partial`
because the presentation callback implementations still need their own
ROM-backed verification; the conversion itself does not modify the native port.

The adjacent Magic-menu pass also now covers `0x100A8`, `0x1088C`, `0x1098E`,
and `0x10A82`. `adjust_magic_force` is a typed word-state mutation that clamps
the selected force, publishes runner record `+0x9D`, runs the source display and
combat/action refresh sequence, and restores only the current-force word
around the combat callback, matching the source stack save. The refreshed
maximum is retained. Spell availability uses the signed
`+0x25` record byte: unavailable emits one repeated marker, while available
emits the selected typed `MagicSpellDefinition` to the spell-text callback and
then writes one grid. The two grid helpers preserve their exact counts and
display commands (`0x8174`, or `0x8134` then `0x8174`). Their callback order and
branch counts are covered by the Magic-menu test; these remain partial only
because the host presentation callbacks are still separate conversion
boundaries.

The neighboring `0x101FE` selector is now a typed magic-menu operation. It
publishes the selected table word's low-byte force maximum and preserves the
existing current force only when the runner has no inventory slot and still
holds the selected spell item code (`spell code + 1`); otherwise it resets
current force to the selected maximum.

The source table feeding that selector is now typed as well. `0x566A2` copies
five 0x22-byte rows from ROM `0x166AD8` into the working table at `FFF9FA`,
and `0xFBA6` chooses a row with the runner record's big-endian `+0xA0` table
index. The extracted rows contain 3, 2, 4, 5, and 2 selectable entries:
`(5,3),(1,1),(3,1)`; `(0,2),(11,2)`; `(7,1),(9,4),(4,3),(11,2)`;
`(8,3),(2,3),(6,2),(3,3),(4,4)`; and `(7,3),(10,3)`. Each pair is
`(D4EA0 spell code, force/value limit)`. The native conversion now exposes
these as `UsMagicSpellTableCatalog` and selects from a typed row; the raw
FFF9FA byte-table helper remains only as an explicit audit boundary.

`0x10276` is also now source-traced as the magic selection display rather than
a generic text batch: its code-6 tile-mask branch, selected typed spell name
and description, shared Success/Force labels, posture/availability calls, and
grouped force value are explicit in one typed callback sequence. The display
and its shared tail receive the same spell definition, rather than passing a
bare code to text and availability services.

The neighboring `0x10260` operation now keeps metadata-grid submission ahead
of selected current/max force publication. `0x102D4` names its three source
text roles explicitly as `Set Force & Posture.`, `Success`, and `Force` before
the posture, availability, and grouped-force services.

The top-level `0xFBA6` Magic controller is now represented as a typed frame
loop. It preserves column-before-row-before-action-before-exit input priority,
resets cursor timing only after navigation, and leaves resource setup and
target/effect services as explicit callbacks.

The `0xFF5A` equipment-selection boundary is now source-traced as a complete
typed orchestration routine. It resolves the selected spell from the US
catalog before detail preparation, then performs the conditional force carry,
installs `spell_code + 1`, routes the force/posture D-C-E edges, clears the
source slot marker, runs the field/HUD meter tail, and publishes `+0x9D`. An
out-of-catalog code returns an explicit missing-spell result; valid source
selections retain the ROM flow, and the ROM contains no availability rejection
at this entry.

### World-effect/menu arithmetic audit continuation

The direct ROM pass over `0x562EC` and `0x563CC` found the existing typed
control flow faithful. `0x562EC` enters `0x562DE` directly, applies the phase
modifier, then uses the source signed-byte difference and comparison-byte
branches to choose HUD-panel, countdown/footer, pending-roll, or row-redraw
services. `0x563CC` clamps the pending amount to two, returns without a
reduction for a signed-byte `BLE` result, and otherwise performs wrapped byte
subtraction before the depletion/reset follow-up. No source-shaped runtime
memory access was added.

The meter caller at `0x56506` did expose conversion mismatches. Its
`CMP.W D6,D7` followed by `BPL` tests the sign bit of wrapped `D7-D6`; it is
not an unsigned maximum. `calculate_menu_hud_row_meter()` now preserves that
predicate, with a high-bit availability regression case, and its action-cost
`SUB.B`/`BLE` gate now preserves overflow. `0x56632` also retains the source
word `7-D4`/`BGT` percentage predicate. The service/variant
timers at `0x56434`/`0x56482` also have explicit wrapped-result tests: their
`BGT`/`BPL` behavior is retained for a zero byte minus `0xFF`.
## Mode-entry audio cadence audit

The source entry at `0x53126` is not an unparameterized generic audio tick.
Its register setup is explicit:

- `D0 = 0x14`, then `0x5315E` runs the fixed audio transaction wrapper.
- `D5 = 2`, then `0x0C24` performs `D5 + 1 = 3` vblank waits.
- `D6 = 5`, so the transaction/wait pair repeats six times through `DBF`.
- `0x52FA0` flushes the audio bus once after the loop.
- `0x53156` runs the zero-command audio service wrapper once before return.

The conversion therefore exposes the command value and vblank counter to
typed host callbacks. It does not model the Genesis bus or substitute a
different timing cadence.

## Random grid resource selection audit

`0xE660` is now represented by a typed three-draw plan rather than an
unexplained random-call count. Its first `bound=2` draw chooses the zero branch
(`C13E8`, then `CE126`/`CE12E`/`CE136`) or the nonzero branch (`C13F8`, then
indexed `CE13E`). The following bounds are `8,3` or `4,4`, respectively, and
both branches submit a two-row grid.

`0xE6F4` similarly records its four source draws (`2,2,3,3-or-4`). The first
two choose the `C13E2`/`C13DC` or `C13D0`/`C13D6` primary table family, including
the source D5 offsets `4/0/0/2`. The zero-first branch uses a fixed three-entry
variant and two rows; the nonzero-first branch uses an indexed four-entry
variant and one row. The converted callbacks receive typed plans and do not
perform ROM or VDP emulation.

## Terrain collision probe audit

The conversion now covers the coupled `0xE7C8` terrain probe family. The
`0xE8A0` sampler keeps the center/four-corner source order, later-equal maximum
selection, blocked `0x7F` override, and selected tile index. `0xEA36` scans
only sampled codes with bit `0x20`, calls `0xEAD2` at the exact signed corner
offsets, and exits on the first carry. `0xE7C8` shifts its incoming coordinate
words by four before bounds checks and sampling, preserves signed negative
codes, sets the directional marker for bit `0x40`, and routes object versus
directional codes through typed callbacks. `TerrainCollisionResult` carries
the result and marker state without exposing source RAM or emulating the VDP.

The blocked-movement continuation also now has the complete source input for
`0xBC36`: ROM `0x0BD34` is a 48-byte table of four banks, each containing
three signed `(x,y)` probe offsets. The first negative byte among
`FFEF10..12` is rotated left three bits and masked to select the bank; the
converted loader exposes those offsets as `CollisionProbeOffsetBank` data and
the runtime must pass the typed terrain samples to `build_collision_probe_points`
before invoking `0xB7B8`. This is a direct extracted record, not a runtime ROM
lookup.

## Terrain collision-map rebuild audit

`0xEE10` now consumes the `FFDF8A` rebuild request, clears the complete
`0x1000`-byte terrain-code map, rasterizes the source's inclusive `0x20`-step
rectangle records, and crosses the `0xEE86` post-process boundary. The latter
now has its actual field/effect gate and signed marker ordinal represented in
`TerrainMarkerReplacementState`, replacing the former generic marker-only
description.

## Bitmask-selected menu text audit

`0xF5F4` now builds the selected field/menu text records from the D0 bitmask,
preserving the seventh-slot special entry, exact-nine slot duplication,
larger-list overflow entry, first/continuation page split, and 0x100 text
destination stride. The C661D/C662B strings and parallel layout values are
decoded host records supplied at the asset boundary.

## Menu cursor-sprite overlay audit

`0xFDFE` now owns a typed six-frame countdown and four-phase C8B3E source
selection for the command/magic/equipment menu cursor overlay. The conversion
preserves the source position offsets, record-list reset/trailing-sequence
clear, FFD980/FFEF14 publication, and unconditional 0xFDAE four-grid write.

## Entity proximity and pair-collision audit

`0xEB92` is now a typed stable-index scan: bit-6 exclusions, player-record
skip, linked +/-`0x100` versus ordinary +/-`0x40` windows, collision-bit
publication, heading/value exchange, `0xECCC` routing, and the masked PRNG
adjustment are all explicit. `0xECCC` now exposes the bit-4 idle-gated special
activity route, the scene/mode variant choice, the bit-3 one-shot activity
flag, and the world-effect/pending-effect/default handler priority through
typed state and callbacks rather than generic source-function pointers.

## Runner ammunition catalog binding audit

The source `0x036FC advance_entity_asset_meter` weapon branch does not receive
an arbitrary maximum. After validating the active/equipped record state, it
uses the one-based action code at `+0x57` to select a `0x30`-byte row from the
US item table at `0xD4540`, then reads that row's `+0x24` byte as the weapon's
ammunition ceiling. The `0x0D..0x0F` action band is the separate inventory-phase
branch and does not read D4540. `0x038AC service_entity_ammo` delegates to this
path only for record types below three; its type-three-and-later branch consumes
one round directly and therefore has no catalog dependency.

`advance_entity_asset_meter(record, delta, item_catalog)` and
`service_entity_ammo(record, delta, item_catalog)` now expose that source
relationship through `UsItemCatalog`. A weapon code outside the catalog or a
row whose one-based identity no longer matches returns
`missing_source_data` before changing ammunition. The scalar-limit overloads
remain available as explicit audit adapters for callers that have already
traced a limit; the conversion does not silently substitute a zero or a
neighboring item's value. Regression coverage exercises the source row 4
ceiling of `0x19`, an identity mismatch, and the type-three clip bypass.

## Equipment action status catalog audit

The source `0x55948 resolve_equipment_action_status` is an ordinary equipment
action path, not a generic wrapper around `0x559BC`. It selects the active
record's one-based `+0x57` code in the US `0xD4540` table, reads `+0x25` as an
index into `0x166AD4`, applies the source `NEG.B`, and uses `+0x26` as the
initial difficulty. The opposing record supplies the `0x55ED6` command
average, the remaining `0x55F20` posture component, `+0x77`, and the first set
bit in cyberware flags 9..11 for the continuation trial count.

The sixteen bytes at `0x166AD4` are now an explicit typed effect catalog. It
retains each source byte and its zero-extended wrapped `NEG.B` result, which
matters for records whose source byte is not one of the ordinary positive
spell-effect entries. The new typed resolver rejects an absent or identity-
mismatched D4540 row and rejects a negative/out-of-range effect index before
mutating resolution state. It then applies the existing typed dynamic-status,
armor, defense, meter, and counter services and enters `0x559BC`; it does not
reimplement those leaves or substitute a guessed action value.

`0x55900 resolve_entity_action_status` is the fixed-parameter sibling that
feeds the same tail. Action code `0x0F` uses value three/difficulty ten and
sets the alternate-meter flag, `0x0E` uses value three/difficulty six, and all
other entries use value six/difficulty eight. It always selects secondary
armor before entering `0x559BC`; the typed conversion shares the continuation
builder with `0x55948` and the regression test verifies the `0x0F` mental-meter
branch without introducing a second combat implementation.

## 0x104F6 special-item modifier audit

The complete `0x104F6` tail is now tied to source data rather than a
caller-invented modifier matrix. The routine scans possession slots 6 through
0 for the selected action item and adds the first matching phase with `ADD.B`.
It then always examines slot 7. Only item codes `0x29`, `0x2A`, and `0x2B`
enter the special branch.

For that branch, `D7` is the original action index selected by `0x104C8` and
`C8AE0[D7]` is a signed selector. The US selector bytes decoded for indices
0 through 15 are:

`1, 1, 1, 0, 1, 0, -1, 1, 1, 1, -1, 2, 0, -1, 0, 12`

A negative selector skips the special word. Selectors 0, 1, and 2 address the
matching one-based `D4540` item record at `+0x22`, `+0x24`, and `+0x26`,
respectively. The `+0x24` and `+0x26` reads are aligned source words, so the
typed rows retain the adjacent bytes (`+0x25` and `+0x27`) instead of reducing
them to a single guessed field. The result is a word addition followed by the
source signed-positive test; values at or below zero become one.

The selector value 12 at index 15 is intentionally not guessed. The traced
instruction would read another word in the same `0x30`-byte record, but no
semantic D4540 field has been established for that offset. The source-bound
conversion returns `source_data_missing` before random rolls or action state
mutation when that path is reached. The old matrix overload remains only as an
explicit audit adapter for tests and callers that have already supplied decoded
values; it is not the source-of-truth path.

The source-bound path is threaded through both direct conversion callers:
`resolve_menu_metric_action` and the special attack-value branch of
`compute_equipment_combat_bars`. Both require the decoded C8AE0 selector and
D4540 item catalog together. A half-supplied pair preserves the phase-only
value but reports `source_data_missing`; it does not reuse the caller-owned
matrix as an implicit compatibility fallback.

## World-effect controller audit: 0x1A370..0x1AC50

The direct ROM pass over `0x1A370` confirmed two separate per-frame paths.
When `FF163E+0x0C` is clear the routine returns immediately. When `FFE082`
is set it services the byte cadence at `FF163E+0x04` and `+0x06`, waits for
vblank, and streams the transition-noise frame; an expired `+0x06` reaches
`0x1A8F4`. When `FFE082` is clear it calls `0x1A538`, starts `0x1A818` when
the selected compact runner has no active effect, and otherwise uses
`0x17E78` as the world-effect countdown/divisor before selecting mode `2`,
mode `3`, or the default-effect setup.

The generated Ghidra C for `0x1A370` is not behaviorally reliable here: it
tests `FF164A` instead of `FF163E+0x0C`, loses the return condition from
`0x1A538`, and omits the meaningful phase input to the `0x1D3F4` noise-mask
loop. The typed conversion keeps those decisions explicit and leaves the
VDP/noise writes at a presentation callback boundary.

This audit also corrected the adjacent state boundaries. `0x1A54E` writes
only `FFE076=1`, `FFE078=0x1200`, `FFE07A=0x0D00`, and `FFE07C=0x1E`.
`0x1AC50` independently publishes `FFE084=1` and clears `FFE086`; it does
not initialize a velocity, lifetime, or palette. `0x1A8F4` clears `FFE082`,
sets `FFE080`, and clears `FF163E+0x0C`. These now use
`WorldEffectDefaultState` and `WorldEffectResetState` in the controller, plus
the source-owned overlay state in `world_effect_overlay_conversion.hpp`.

Missing converted dependencies in the controller now return explicit
unresolved actions instead of silently treating absent callbacks as a false
runner marker or a completed default initialization. Zero from `0x17E78` is
retained as the source countdown value; the mode-2 divisor path reports an
unresolved/invalid boundary rather than dividing by zero in host C++.

## Title/world sprite call audit: 0x1A570, 0x1A5A0, and 0x1A7BE

The first pass over `0x1A570` was too generic: the generated C showed two
calls but dropped all of the source arguments. The ROM emits two different
lists through `0x1378 build_title_sprite_records`:

| source list | base X | base Y | builder flags |
| --- | ---: | ---: | ---: |
| `0xF1B1E` | `0x012A` | `0x0100` | `0x2000` |
| `0xF1B28` | `0x0115` | `0x0100` | `0x2000` |

`TitleSpriteBatchBuildRequest` preserves these as two ordered typed requests.
The world mode conversion now sends those requests when `0x1A5A0` reaches its
negative `FFE0FC` transition result; it no longer hides the identity of the
two title groups behind a no-argument “rebuild” callback.

The `0x1A7BE` leaf also carried more behavior than its generated C exposed.
It selects `((FFDF88 >> 1) & 3) + 8` from the `0xF1AEE` pointer table, appends
sprite key `0x06E0`, advances `FFE07A` by `0x60`, and only builds while the
unsigned word remains below `0x1280`. The build uses `FFE078 >> 4`,
`FFE07A >> 4`, attribute offset/key `0x06E0`, and the 16-bit build flags
`0xA000` (`D6=-0x6000`). `TitleVerticalSpriteBuildRequest` now carries those
fields, including the selected table index, and tests cover the fixed-point
coordinate conversion and strict unsigned cutoff.

The mode-6 completion call at `0x1A770` is now named for its actual callee,
`advance_menu_selection_service_and_refresh` (`0x5668C`), rather than being
reported as a generic “finish mode” hook. These changes are conversion/source
audit work only; the sprite builder remains a host asset/presentation boundary.

## World/title scene-loop audit: 0x1ADE6

The scene driver comparison found that the central loop had been hiding
several source boundaries. The call at `0x19788` is
`advance_matrix_program_loading`, not a generic world-menu service. The
persistent title/world services at `0x1CCDC` and `0x1CC24` are called on every
non-world-sequence presentation tick; their own routines inspect their source
state. The scene loop must not invent outer “persistent active” gates.

`FFF0B1` has two different source tests: the main active-scene path uses
`BMI` immediately after `0x1A370`, while the field-refresh path uses `BNE` at
`0x1B16A`. The host now carries the signed status byte and applies those two
conditions independently. The prologue also preserves the external
`FFF0A1` noise flag, `FFE0DE` Matrix-transition request, and existing `FFEF14`
dirty bits because the ROM does not clear them there. It continues to clear
the scene-owned title/world latches in the exact source order.

## World-effect animation/overlay audit: 0x1AA40..0x1AD42

The direct listing exposed a branch distinction that the first typed pass had
collapsed. `FFE0F2` is the selector for the loading/transition path: only
that flag selects the fixed `0xF1C46` table, uses the six-phase cadence, and
skips the three overlay calls. `FFE082` is a separate descriptor/completion
gate. When `FFE0F2` is clear and `FFE082` is set, the normal descriptor table
still renders and all three overlays still advance; only the normal timing
update and `0x0694` append are skipped. The conversion now models those
branches separately.

The overlay requests are now source-specific. Primary, secondary, and
background use tables `0xF1BDE`, `0xF1BCE`, and `0xF1BBE`, respectively, and
the typed build request carries the selected table, phase, shifted X origin,
and fixed Y origin. The old `attribute`/`projected_position` names obscured
that these values are the builder's D2/D1 coordinate origins (`0x00B9` for
the moving overlays and `0x00D0` for the background), not palette metadata.

The primary phase compare is `CMP.W #0x1A` followed by `BCS`, so the valid
phase range is `0..0x19`; a post-increment value of `0x1A` resets to zero.
Secondary and background wrap at four phases. The conversion and regression
tests now retain these exact table identities, origins, branch gates, and
phase boundaries.
