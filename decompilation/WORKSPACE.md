# Workspace ownership map

This file defines where new Shadowrun reconstruction work belongs. The native
port, the source-conversion workbench, and its Qt host are separate products
with separate runtime state. They share source evidence and named assets, not
game-state implementations.

## Repository root

| Path | Responsibility |
| --- | --- |
| `AGENTS.md` | Project constraints and reconstruction rules |
| `CMakeLists.txt`, `CMakePresets.json` | Visual Studio IDE umbrella only |
| `decompilation/` | ROM research, extracted host assets, native port, and conversion workbench |
| `video analysis/` | Independent gameplay capture and numbered reference frames |
| `build/` | All generated builds and build-derived captures; ignored by Git |

## `decompilation/`

| Path | Responsibility |
| --- | --- |
| `input/` | Canonical binary inputs, including the US Genesis ROM |
| `export/` | Read-only Ghidra export and conversion scratch evidence |
| `analysis/` | Ghidra project-local analysis data and notes |
| `analysis/retired/` | Retired capture analyses kept only as historical evidence |
| `assets/` | Individually named host resources generated from source assets |
| `include/shadowrun/<domain>/` | Native-port public contracts |
| `src/<domain>/` | Native-port implementation, organized by behavior owner |
| `src/app/` | Native and Qt executable entry/presentation adapters |
| `tests/<domain>/` | Native-port checks grouped by the behavior they defend |
| `tools/` | Offline extractors, visual dumpers, ROM analyzer, and asset scripts |
| `ghidra_conversion/` | Independent source-derived conversion workbench and Qt host |
| `docs/` | Screen maps, ROM audits, visual review, hill status, and source evidence |
| `ShadowRunGenesis.gpr`, `ShadowRunGenesis.rep/` | Local Ghidra project state; ignored and not runtime input |

Native domains use the subsystem boundaries already present in the conversion
workbench: `audio`, `core`, `entities`, `field`, `rendering`, `session`, and
`title_intro`. Menu behavior remains within the native field/menu controller;
menu-focused evidence lives in `tests/menus/`.

## Ghidra conversion workbench

`ghidra_conversion/include/shadowrun_ghidra/<domain>/`, `src/<domain>/`, and
`tests/<domain>/` own the typed behavior contracts recovered from the ROM.
`src/integration/` composes those contracts across subsystems and is not a new
domain owner.

`ghidra_conversion/qt_runtime/` is the Qt presentation and executable layer:

| Path | Responsibility |
| --- | --- |
| `src/app/` | Qt host entry point and process-level scene routing |
| `src/audio/` | Audio output adapter |
| `src/dialogue/` | Generic dialogue presentation, text catalog, and event-fed dialogue state |
| `src/field/` | Field screen, command, interaction, and roster adapters |
| `src/menus/` | Attributes/skills, hotel, shop, and menu presentation state |
| `src/rendering/` | Qt image compositing helpers |
| `tests/<domain>/` | Qt adapter checks grouped by subsystem |

Generic dialogue panels consume speaker/event properties and trigger state;
they do not own the world event that supplies the text. Qt controls present
typed conversion/native state and must not create a competing game-state
machine.

## Build output layout

Every configure preset writes under the ignored repository-level `build/`
directory. Debug, Release, Qt, IDE umbrella, and MSVC output have distinct
paths:

```text
build/
  ide/{debug,release,msvc/debug}/
  native/{debug,release,qt/debug,qt/release,msvc/debug,msvc/release}/
  conversion/{workbench/{debug,release},qt/{debug,release}}/
  archive/       # earlier builds and their captures, retained for reference
```

Do not add a `build-*` directory beneath a source domain. Add a preset under
the owning CMake project and route it to the matching `build/` subtree. Keep
generated screenshots/logs with their build or under `build/archive/`; source
evidence and independent reference frames belong under `docs/` and
`video analysis/`.

`build/archive/` preserves old build outputs and captures after relocation.
Their CMake caches still name the previous output locations, so treat those
trees as artifact storage; configure and build through the current presets.

Two older generated trees still sit under their former project folders:
`decompilation/build-debug/` and
`decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/`. Windows
denied moving these directories, so they remain ignored in place until they
can be archived. All new configure presets point to the centralized layout.

## Ownership rule

Put implementation, public contract, and focused check beside their domain
counterparts. Use `app/` only for executable startup and cross-screen Qt
composition, `integration/` only for multi-domain conversion composition, and
`tools/` only for offline/import/inspection executables or scripts. A new
directory needs a single written ownership rule before files are added to it.
