# Hills

Standing order is in `AGENTS.md`. This file is the progress index. A hill is
defended only with a named CTest plus a framebuffer dump. Same-source visual
composites do not count.

| Cluster | Hill | ROM | Status | Defending test | Dump |
|---|---|---|---|---|---|
| 0 Process | Visual dump CLI | — | defended | `shadowrun_visual_dump` writes 320x224 BMPs | `build-debug/visual-regression/dump/` |
| 0 Process | Leftover-clear (startup + field menu) | `0x59092` / `0x59172` / `0x10F50` | defended | `shadowrun_leftover_clear` | leftover-clear/*.bmp |
| 1 Boot | Pre-title logos | `0x5D746` after reset `0x0236` | partial | `shadowrun_boot_sequence` | `--scene boot` (SEGA plane B, snow HSCROLL, pal strobe) |
| 2 Title | Layered title loop | `0x1A9E` / `0x1E4E` / `0x1C6A` | partial | `shadowrun_menu_system` (timing only) | `--scene title` |
| 3 Startup | Start Options | `0x59092` | partial | `test_startup_behavior_and_rendering` | `--scene start-options` |
| 3 Startup | Save/Load empty slot | `0x59172` | partial | leftover-clear + startup test | `--scene save-load-empty` |
| 3 Startup | Archetype | `0x59248` / `0x42B0` | partial | startup test | `--scene archetype` |
| 3 Startup | Intro stages | `0x59C5E` / `0x5AF62` / `0x59D7C` / `0x5AA96` / `0x5AD1A` | partial | `shadowrun_menu_system`; `shadowrun_graphics` | `--scene intro` frames 0/0xA0/0x290/0x3A4/0x6EC |
| 4 Field | Walk / collide | `0x3084` / `0xE7C8` | partial | `shadowrun_native_port` | `--scene field` |
| 5 Dialog | Motel doorway | mode 2 `C0`/event 0 → `0x14` + `0x20E48` | partial | `shadowrun_native_port` motel tuple | `--scene motel-dialog` |
| 6 Menu | Command overview pixels | `0x10F50` | partial (same-source composite, not a golden) | `shadowrun_menu_visual_regression` | command-menu actual BMP |
| 6 Menu | Magic | `0xFBA6` | pending | — | — |
| later | Matrix | `0x1817E` | pending | — | — |
| later | Combat | field combat loop | pending | — | — |
| later | Audio | `0x530DE` | pending | — | — |

Rejected (must not return):

- Extra Joshua archetypes as switchable party members.
- Mode `0x1A` (security interior) as the motel doorway.
- Generic “empty building” interiors.
- Six full-screen title frames as the claimed title sequence.
