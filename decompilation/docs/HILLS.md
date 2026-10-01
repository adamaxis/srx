# Hills

Standing order is in `AGENTS.md`. This file is the progress index. A hill is
defended only with a named CTest plus a framebuffer dump. Same-source visual
composites do not count.

| Cluster | Hill | ROM | Status | Defending test | Dump |
|---|---|---|---|---|---|
| 0 Process | Visual dump CLI | — | defended | `shadowrun_visual_dump` writes 320x224 BMPs | `build-debug/visual-regression/dump/` |
| 0 Process | Leftover-clear (startup + field menu) | `0x59092` / `0x59172` / `0x10F50` | defended | `shadowrun_leftover_clear` | leftover-clear/*.bmp |
| 1 Boot | Pre-title logos | `0x5D746` after reset `0x0236` | partial | `shadowrun_boot_sequence` | `--scene boot` (guy fade, lightning hides, SEGA shine stops, FASA fade-in then hold then fade, 0x131C) |
| 2 Title | Layered title loop | `0x1A9E` / `0x1E4E` / `0x1C6A` | partial | `shadowrun_menu_system`; `shadowrun_graphics` SAT box | `--scene title` (black, 4px L-R explode, baked 7-frame circuit lights stepping every 3 ticks, one Start + 0x131C fade) |
| 3 Startup | Start Options | `0x59092` / `0x10B5A` | partial | `test_startup_behavior_and_rendering` | `--scene start-options` (CRT frame + layout `'['/'\\'` title bar, triangle cursor) |
| 3 Startup | Save/Load empty slot | `0x59172` | partial | leftover-clear + startup test | `--scene save-load-empty` |
| 3 Startup | Archetype | `0x59248` / `0x42B0` | partial | startup test | `--scene archetype` (CRT + 0x7291A+0x50 portraits) |
| 3 Startup | Intro stages | `0x59C5E` / `0x5AF62` / `0x59D7C` / `0x5AA96` / `0x5AD1A` | partial | `shadowrun_ghidra_startup_integration` | Separate Qt conversion reaches field handoff and emits direct Seattle, wilderness, tri-vid timed, three independent ambush map layers, overlays, ambush SATs, airport, and cab-map layers; 30-fps longplay comparison is recorded, but this remains partial until the full opening dump has a defended visual CTest |
| 3 Startup | Ambush source choreography | `0x5A1EA` / `0x5A252` / `0x5A59C` / `0x5A8DE` | defended | `shadowrun_native_port` | `visual-regression/intro-hill/intro-1240.bmp`, `intro-1552.bmp`, `intro-2512.bmp` |
| 3 Startup | Airport helicopter overlays | `0x5B08E` / `0x5B100` / `0x5B1B6` | defended | `shadowrun_native_port` | `visual-regression/intro-hill/intro-3000.bmp`, `intro-3400.bmp` |
| 3 Startup | Cab destination zoom | `0x13726` / `0xD4464` / `0xD4536` | defended | `shadowrun_native_port` | `visual-regression/cab-hill/cab-map-0.bmp` through `cab-map-89.bmp` |
| 4 Field | Walk / collide | `0x075E` / `0x03084` scene load / `0xE7C8` / `0x3BAC` | partial | `shadowrun_native_port` | `--scene field` (272px playfield, 0x334C 6-tile HUD column) |
| 5 Dialog | Field electromagnetic-lock / Security Terminal prompts | mode-2 field event table → `0x20E48` / `0x20EC2` | partial | `shadowrun_ghidra_field_interaction_continuation_test` (source plan only) | Qt prompt dump pending |
| 5 Dialog | Shared NPC dialogue panel projection | `0x211E8` / `0x2401C..0x2410C` / `0x21226` / `0x1F65A` / `0x1F246` / `0x1F3C4` / `0xF4AA0..0xF4B2C` / `0x121FA0` / `0x122420` | partial | `shadowrun_ghidra_dialogue_panel`, `shadowrun_ghidra_field_dialogue_runtime` | `--field-dialogue-dump manager <bmp>`; common panel and A/B/C loop are connected, but real field trigger, exact source tile placement, random arrival, payment/debt continuation, and independent golden remain open |
| 6 Menu | Command overview pixels | `0x10D30` / `0x10F50` / `0x110BC` / `0x114A0` / `0x11592` | partial (same-source composite, not a golden) | `shadowrun_ghidra_field_command_overview_bridge` | `--field-command-dump <bmp>` |
| 6 Menu | Inventory action row/equipment/discard | `0x11740` / `0x118C6` / `0x12148` / `0x12168` / `0x121E0` / `0x12634` / `0x1275C` / `0x12836` | partial (same-source composite, not a golden) | `shadowrun_ghidra_conversion_qt_field_action_qa` | `--field-action-dump <output-dir>`; ten executable checkpoints cover selection/re-selection/un-selection, equipment math, bars, and discard; trade recipient/First Aid/utility branches remain explicit |
| 6 Menu | Clips four-entry selector/reload/trade panel | `0x11B58` / `0x11CCA` / `0x11DD2` / `0x11E7E` / `0x11FDE` / `0x12062` / `0x1209A` | partial (same-source composite, not a golden) | `shadowrun_ghidra_conversion_qt_field_clips_qa`; `shadowrun_ghidra_field_clips_menu_runtime`; `shadowrun_ghidra_field_party_roster_runtime` | `--field-clips-dump <output-dir>`; source four-button row replaces the five-button command row, selector movement/wrap/reselection/cancel, typed new-game/save roster projection, nested recipient filtering, quantity wrap, transfer arithmetic, distribution, and reload arithmetic are checked; runner-name resource, full loaded-save field composition, and independent visual golden remain open |
| 6 Menu | Magic | `0xFBA6` | partial | `shadowrun_native_port` magic open | Magical Defense C0DA 0xC89C0 |
| later | Matrix | `0x1817E` | pending | — | — |
| later | Combat | field combat loop | pending | — | — |
| later | Audio | `0x530DE` | pending | — | — |

Rejected (must not return):

- Extra Joshua archetypes as switchable party members.
- Mode `0x1A` (security interior) as the motel doorway.
- Generic “empty building” interiors.
- Six full-screen title frames as the claimed title sequence.
