# ROM-to-native audit

This is the working audit for the native rewrite. The Ghidra export remains
the evidence corpus; none of the entries in this document authorize ROM
address reads, a 68000 interpreter, or a VDP-shaped runtime.

## Current source evidence

| Evidence | Current count | Meaning |
| --- | ---: | --- |
| Exported function entries | 910 | Canonical Ghidra function inventory |
| Resolved direct call sites | 2,986 | Calls with a concrete target in `call-sites.tsv` |
| Indirect/unresolved call sites | 16 | Explicit dispatch boundaries still needing table/handler proof |
| Generic export labels | 119 | Functions still named by address/family rather than behavior |
| Conversion implementation files | 225 | Leaf source-trace contracts, not proposed runtime files |
| Behavioral conversion modules | 28 | The reviewed ownership boundaries in `conversion-source-modules.tsv` |
| Conversion tests | 225 | Focused source-trace tests; all currently pass |

Generic labels are not evenly distributed. The manual naming backlog is:

| Family | Generic labels | Immediate question |
| --- | ---: | --- |
| intro/title | 56 | Which stage, actor, text batch, or effect does each call serve? |
| menu | 21 | Which concrete menu owns the record/table access and mutation? |
| graphics/data presentation | 18 | Is this a sprite record, text/resource reader, HUD panel, or importer boundary? |
| interrupt/timing | 8 | Which host scheduling or input service is actually required? |
| entity | 6 | Which record fields and behavior transition are read or mutated? |
| world/effects | 5 | Which effect record, mode, or event transition owns it? |
| field | 4 | Which field mode, terrain, camera, or exit path owns it? |
| save/session | 1 | Which compact record projection owns it? |

## Indirect dispatches that still need source proof

These are not missing implementations to be filled with guessed handlers. Each
one needs its table selector, table contents, handler contract, and native
owner identified from the ROM and its callers:

| Caller | Call site | Native boundary |
| --- | --- | --- |
| `run_game_loop` | `0x0B42` via `A1` | Main scene/mode dispatch |
| `update_entity_frame` | `0x46F6` via `A1` | Entity kind dispatch |
| `dispatch_menu_handler` | `0xC444` via `A0` | Concrete menu action handler |
| `run_inventory_actions` | `0x11884` via `A0` | Inventory action table |
| `convert_entity_routine_0x15830` | `0x15928` via `A2` | Entity effect/action service |
| `service_active_effect_slots` | `0x1640A` via `A1` | Active world-effect handler |
| `service_matrix_program_action` | `0x19A1C` via `A4` | Selected Matrix program action |
| `dispatch_title_phase_handler` | `0x1B892` via `A1` | Title/world phase handler |
| `initialize_world_effect_selection` | `0x1E728` via `A0` | Effect-selection policy |
| `dispatch_world_effect` | `0x1EF28` via `A1` | World-effect handler |
| `run_mode_exit_services` | `0x1F26E` via `A1` | Mode-exit service list |
| `run_mode_text_prompt` | `0x1F58A` via `A0` | Mode-specific text prompt |
| `dispatch_indexed_handler` | `0x24E32` via `A0` | Indexed world/event handler |
| `run_indexed_title_services` | `0x53AA4` via `A1` | Title service sequence |
| `run_mode_action_loop` | `0x54960` via `A0` | Mode action handler |
| `advance_title_intro_schedule` | `0x5AF48` via `A1` | Intro stage schedule entry |

Known table families to resolve against those calls include `0x12CCB4`,
`0xF3DC6`, `0xF3E1E`, `0x490A`, and the table families around `0x1D751E`,
`0x1DCB0C`, `0x1DD216`, `0x1DD98C`, and `0x1DDE16`. The native result should
be an enum/policy/variant visitor, not an address-indexed function table.

## Data still to pull into typed native data

These are the data families that game behavior consumes. Graphics extraction
alone is not sufficient:

| ROM family | Native data boundary | Status |
| --- | --- | --- |
| `0x7291A`, `0x72C1A` | Archetype defaults and ten recruited runner templates | Confirmed fields; recruitment writers remain to trace |
| live `0x100`-byte runner record | `RunnerRecord` / `RunnerRoster` | Confirmed consumed offsets; opaque bytes remain until a consumer is proven |
| `0xD4540`, stride `0x30` | `ItemDefinition` inventory/equipment records | Confirmed initial fields; complete category/effect mapping still needed |
| `0xD4D50`, `0xCF3D4`, `0xCF610` | Weapon, armor, attack, defense, and timing tables | Consumers identified; full typed extraction still incomplete |
| `0xD2072` | Cyberware names/effect slots | Names extracted; effect semantics still need rule consumers |
| `0xC1A2A` | Attribute/skill labels and definitions | Labels extracted; upgrade/derived-rule data still needs a typed table |
| `0xD4EA0`, reached by `0xFBA6`/`0x1088C` | Spell definitions, availability, cost, force/effect rules | Explicit remaining extraction task |
| field mode/terrain/entity/event tables | `AreaResource`, `TerrainMap`, `EntityTemplate`, event/exit records | Modes and terrain partially typed; all event/transition records not yet pulled |
| `0x59318` and `0x597EE..0x598B0` | `SaveStore` compact records and projections | Projection behavior converted; complete save data package not yet native |
| text pointer/catalog families | `TextCatalog` / dialog/menu/notebook data | Several screens extracted; complete catalog ownership still under audit |

## Native integration order

The executable can now boot from direct assets and reach the field through the
native controller. The next runtime slices should be pulled from the audited
boundaries in this order:

1. Replace the historical `NativePortShell` name with the subsystem-owned
   `NativeGameController` implementation files while preserving one session
   state owner.
2. Introduce `ExtractedGameData`/`RulesDatabase` for item, runner, field,
   text, and spell records. Keep it separate from the image/audio manifest.
3. Integrate the defended field/entity contracts into `FieldWorld` and
   `EntityWorld`, then add independent visual dumps and control tests.
4. Resolve menu dispatch and record mutation by concrete menu ownership;
   inventory, equipped gear/armor, first aid, clips, cyberware, attributes,
   and magic must not share a generic action bucket.
5. Resolve the Matrix tables into `CyberdeckSystem` policies.
6. Finish combat/effect handlers and save/load before calling the native game
   broadly playable.

Every completed item must have a ROM trace, a typed native owner, a focused
test, and (for a screen) an independent image dump. A renamed generic wrapper
does not close an audit item.
