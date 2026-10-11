# Workspace ownership

## Active project

Current implementation work belongs to `ghidra_conversion/`. This is the
active CMake project and the source of the Release Qt executable:

| Path | Responsibility |
| --- | --- |
| `ghidra_conversion/include/`, `src/`, `tests/` | Typed source-derived C++ model and checks |
| `ghidra_conversion/qt_runtime/` | Qt application, presentation adapters, and runtime checks |
| `ghidra_conversion/tools/map_entity_viewer/` | Standalone Qt map/entity authoring application |

The active project reads shared source data from sibling folders. These paths
are data/evidence inputs; they do not make the old native-port project active:

| Path | Responsibility |
| --- | --- |
| `input/` | Canonical ROM input |
| `export/` | Ghidra C export and reverse-engineering evidence |
| `assets/` | Individually named extracted host assets |
| `gameplay/scripts/` | Lua content loaded by the Ghidra Qt runtime |
| `docs/` | ROM maps, screen audits, source traces, and visual evidence |
| `../video analysis/` | Independent gameplay video and reference frames |

## Inactive historical project

The older native-port CMake project and its source remain under
`CMakeLists.txt`, `include/`, `src/`, and `tests/` at this directory level.
They are retained for reference, but current changes, builds, and launch
configurations belong to `ghidra_conversion/`. Do not select the parent CMake
project for ordinary work.

## Build outputs

Configure and build from `ghidra_conversion/`. Its presets keep active products
separate and centralized at the repository root:

```text
build/
  conversion/
    workbench/{debug,release}/
    qt/{debug,release}/
  archive/
    source-local-builds/   # former build trees moved out of source folders
    visual-review-runs/    # earlier menu/field screenshot review sessions
    ghidra-investigations/ # one-off source-trace and entity experiments
    workspace-root-generated/ # stray root CMake/test/packaging output
    ...                    # earlier binaries and QA evidence, retained
  visual-audits/           # current visual-review artifacts
```

The normal app is the Release Qt executable at
`build/conversion/qt/release/shadowrun_ghidra_conversion_qt.exe`. Debug builds
are for diagnosis. Do not create build directories under source folders.
Historical source-local build trees were moved under
`build/archive/source-local-builds/`. Archived captures and one-off analysis
outputs remain available under their named archive folders; use only the
centralized `build/conversion/` paths for current Ghidra executables.

## Ownership rules

- Keep conversion behavior in `ghidra_conversion/src/<domain>/` and its public
  types in `ghidra_conversion/include/shadowrun_ghidra/<domain>/`.
- Put Qt presentation adapters in `ghidra_conversion/qt_runtime/`; they must
  present the conversion/Lua state instead of introducing a competing game
  state machine.
- Put multi-domain composition in `ghidra_conversion/src/integration/`.
- Keep extracted assets and Lua content as named files in their shared data
  folders; the runtime consumes those files directly.
- Keep generated binaries and visual captures under `build/`, not next to
  source files.
