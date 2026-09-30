# Native architecture map

This is the architecture implied by the ROM, the canonical Ghidra callgraph
snapshot,
the typed conversion work, and the existing native source. It is the shape the
port should converge on; it is not a claim that every named native layer exists
yet. The 910 exported functions are not 910 original source modules: they are
mostly leaf routines, table-dispatch targets, transfer wrappers, animation
variants, and small record helpers. The export currently has 2,986 resolved
direct call sites and 16 indirect/unresolved boundaries.

## The short version

```text
Windows / Qt frontend
        |
        v
NativeGameController ---- InputState / FrameClock
        |
        +--> GameSession (mutable runner, world, story, save state)
        |        |
        |        +--> SceneState: Boot -> Title -> Startup -> Field -> Menu
        |        |                         \-> Dialog / Combat / Matrix / Shop
        |        |
        |        +--> RunnerRoster / CharacterRecords
        |        +--> RulesDatabase / ExtractedGameData (read-only)
        |        +--> FieldWorld (area map, terrain, collision, entities)
        |        +--> MenuSystem (command focus, inventory, equipment, dialogs)
        |        +--> IntroDirector (sprite choreography and timed text)
        |        +--> AudioService (complete Windows-playable tracks/effects)
        |
        +--> NativeRenderer (background layers, individually named sprites,
        |                     object ordering, text, RGBA presentation)
        +--> ResourceManifest / AssetStore (direct assets only)

Offline only: ROM/data extractors, image/palette decoding, audio conversion,
              manifest generation
Reference only: one Ghidra project/export, ROM traces, scratchpad, captures
```

The arrow direction is ownership. The renderer and Qt widgets present typed
state; they do not own a second game state machine. The extractor creates host
assets and typed game data; the runtime never decodes ROM pointers or reads
Genesis RAM addresses.

## 1. Runtime layers

### Frontend and process shell

The Windows executable and optional Qt frontend should own only process/window
concerns:

- create the window and select the presentation scale;
- translate Windows/Qt events into `InputState` (`held` and `pressed` bits);
- drive a 60 FPS `FrameClock`;
- hand frames to the renderer and present them.

The frontend must not contain title/menu/field transitions. Those belong to
`NativeGameController` and `GameSession` so a headless test and Qt display use
the same behavior.

### Game session and state graph

The source reset path at `0x0236` enters the main coordinator at `0x075E`.
The native equivalent is one controller update:

```text
sample input edge
  -> update the active SceneState
  -> update world/entity timers
  -> build a presentation frame
  -> submit audio commands
  -> present
```

The state graph recovered from the ROM is:

```text
Boot / region setup
  -> pre-title logos (0x5D746)
  -> title loop (0x1A9E)
       Start edge
  -> Start Options (0x59092)
       New Game                  Load Game
       -> Archetype (0x59248)    -> Save/Load (0x59172)
       -> timed opening              valid load / empty-slot stay
  -> Field runtime loop (0x075E)
       scene descriptor load (0x3074 -> 0x3084 -> 0x313A)
       one-pass frame coordinator (0x0952..0x0B9C)
       Start -> command overview and submenus
       raw event -> area transition / dialog / combat / special state
```

Indirect ROM dispatches remain explicit in the native model as typed state
transitions. They are not generic function-pointer tables copied from the
ROM. The known state-pair and handler-table families around `0x1F580` and
`0x214B0` should become named transition policies as each hill is traced.

### Extracted game data and rules boundary

The ROM is not only an image and audio source. It also contains the static game
database: fixed-size records, pointer tables, text catalogs, archetype
templates, item/equipment definitions, spell definitions, field/event data,
and other lookup tables consumed by the original routines. These must be
extracted and typed before the native systems can be considered complete.

The offline extraction side should produce an `ExtractedGameData` package,
separate from the runtime image manifest. It is read-only at runtime and is
the native equivalent of the source tables, not a ROM address map:

```text
ExtractedGameData / RulesDatabase
  +-- runner/archetype templates
  +-- roster and character-record defaults
  +-- inventory, weapon, armor, and equipment definitions
  +-- cyberware definitions and installed-effect metadata
  +-- spell/magic definitions and costs/effects
  +-- attribute and skill definitions
  +-- field maps, entity templates, event cells, and transition records
  +-- dialogue, menu, notebook, and other text catalogs
  +-- source-derived animation/audio event definitions
```

Each table needs a source address/stride/pointer-table note in the extraction
catalog and a typed native representation. The runtime may use ordinary IDs,
enums, vectors, and records; it must not retain source pointers or perform
ROM-address arithmetic. `assets/native/state.bin` remains the small mutable
startup-state payload; static rules/data should have an explicit separate
generated data boundary rather than being hidden in graphics assets or the
runtime manifest.

The already-confirmed extraction families include:

- runner/archetype templates at `0x7291A`, with `0x100`-byte records, and the
  ten named recruited templates at `0x72C1A` (terminated by a zero-kind record);
- the live runner record fields consumed by the inventory, posture, HUD,
  attributes, skills, cyberware, and combat-bar routines;
- 50 inventory/equipment records at `0xD4540`, with a `0x30`-byte stride;
- armor/detail and attack/defense lookup data at `0xD4D50`, `0xCF3D4`, and
  `0xCF610`, plus the associated timing/animation lookup families;
- fifteen cyberware names/effect slots through the pointer table at `0xD2072`;
- twenty attribute/skill labels through the pointer table at `0xC1A2A`;
- field mode, terrain, entity-template, event, and transition tables behind
  the field-mode and entity dispatch pointers;
- save/roster compact records and the projections used by `0x59172` through
  `0x598B0`;
- the spell/magic records and availability/effect rules reached by `0xFBA6`,
  which remain an explicit extraction task rather than being inferred from the
  menu image.

This list is a starting inventory, not permission to treat every constant in
`rom_layout.hpp` as understood. Every family still needs a source consumer,
record shape, pointer/stride rule, and native use documented before it is
called complete.

The distinction is:

```text
ExtractedGameData / RulesDatabase = immutable definitions and lookup tables
GameSession / RunnerRoster         = mutable play state and record instances
SaveStore                          = persistence of the mutable state
```

### Authoritative session model

`GameSession` owns the data that changes during play and refers to the
read-only `RulesDatabase` for definitions:

- current scene/mode and scene-local cursor/focus;
- the active runner and roster eligibility;
- typed runner records, including the distinction
  between possession, equipped weapon/gear, and equipped armor;
- area/world identity, camera, terrain/event cells, and collision state;
- pending interactions, dialog/combat state, timers, and animation phases;
- save/load state and the source-derived random state where behavior requires
  it.

The source live record is the important boundary, not the individual Ghidra
function. Existing evidence identifies fields such as `+0x10` flags,
`+0x3E` record type, `+0x4D` posture, `+0x4E/+0x4F` ammunition/clip state,
`+0x56/+0x57` equipment, `+0x66..+0x6D` possession, `+0x76` armor, and
`+0x77..+0x9D` attributes/status. Native types should grow around these
confirmed meanings instead of reproducing a byte-addressed RAM struct.

## 2. Gameplay subsystems

### FieldWorld

This is the largest shared gameplay subsystem:

```text
AreaResource
  -> terrain layers / event cells / object descriptors
  -> camera and viewport
  -> collision probe and boundary transition
  -> EntityWorld update
  -> world effects / raw event reaction
```

The field runtime loop at `0x075E` combines movement, four-corner terrain probing, camera motion,
entity projection, animation, and mode transitions. It should not be split into
one class per decompiled function. The source families around `0x2B34`,
`0x2CE0`, `0x2F32`, `0x3074`/`0x3084`, `0x3A1C`, `0x3BAC`, `0xE7C8`, `0xB7B8`, and
`0x167AE` are natural methods/services on this subsystem.

The recurring coordinator is a separate integration seam rather than a second
game-state machine: `advance_field_runtime_frame()` owns the source order for
VBlank/request priority, the `FFDF3E` field-transition decision tree, the
`FFDF40/FFDF42` world-activity re-entry, normal entity/camera/render/world
services, controller-bit precedence, command overview, and pending
scene-handler outcomes. Its transition callbacks are named for the actual
converted leaves: mode-entry preparation, scene-variant carry, field-exit
orchestration, anchor reset/snapshot, field-mode transition, world-activity
initialization, world-activity request handling, and field-scene
initialization. Missing leaves return explicit pending results, so connecting
the seam to Qt cannot silently turn an unresolved source branch into a fake
screen. The runtime contains no ROM function pointer or 68000 loop.

### EntityWorld and effects

An entity is a typed record plus behavior/animation state, not a pointer into
the old RAM image. `EntityWorld` owns:

- active roster/NPC/effect slots;
- fixed-point world position and screen projection;
- depth order and object-list priority;
- movement/collision behavior;
- animation phase/timer and sprite descriptor selection;
- temporary world effects and their lifetime.

The many small routines in the `0x4E..0x62` and `0xA8..0x15` regions collapse
into these services: timing, projection, sprite-record construction, heading,
route/collision evaluation, and behavior transitions. The source helper count
does not imply separate runtime objects.

### MenuSystem

`MenuSystem` owns command focus, row/column selection, inventory actions,
equipment mutation, text/dialog layout, and return-to-field state. It receives
the active runner record from `GameSession` and emits typed presentation data.

The important source distinction is preserved:

```text
inventory possession  !=  equipped weapon/gear/armor
```

The menu may use Qt Widgets or Qt Quick for static controls, but the cursor
bounds, action dispatch, mutations, and exit path remain in this native
controller. The current source groups around `0x10D30..0x12EAE`,
`0x17..0x19`, and `0x1F3C4` are menu services, not hundreds of screens.

### IntroDirector

The intro is a timed composition of independently extracted sprites, text,
background layers, camera motion, and palette/visibility transitions. It is not
a bitmap flipbook.

`IntroDirector` should hold typed choreography records such as:

```text
frame range -> sprite alias -> position/path -> layer/order -> palette/alpha
```

It renders moving sprites individually and synchronizes their source-derived
cadence. The WebM/FFmpeg microframe breakdown belongs only in
`webm_ffmpeg_microframes/` as a reference-analysis artifact. It is never part
of the runtime asset manifest and never replaces sprite extraction.

### AudioService

The source audio functions around `0x52D..0x53` are a command/bus producer and
the source sound driver is an extraction reference, not the Windows playback
backend. Offline audio extraction must decode the source resources into
complete named music/effect files in a Windows-supported format. Native
`AudioService` receives named music/effect events and plays those files; it
does not reproduce Z80 command sequencing, waveform uploads, driver state, or
fragment catalogs. It must not expose or consume the old broken fragments.

The emulator/bus transcription may remain as a museum/reference path. It is
not a dependency of native gameplay.

## 3. Presentation and resources

### AssetStore and resource manifest

The runtime uses one flat, human-maintained resource list with aliases:

```cpp
assets["title.logo"];
assets["intro.helicopter"];
assets["field.entity.walk.0.1"];
assets.audio_path("audio.intro.title.stage-1");
```

The manifest is the complete runtime allowlist for host resources. It should
point directly to individual extracted RGBA images, maps, sprites, text/data
resources, and complete audio files. A runtime load loop iterates manifest
rows; it does not contain one assert or one hard-coded loader branch per
asset. Static rules/data are a separate generated data package and are not
smuggled into this image/audio manifest.

`assets/resources.tsv` is the runtime manifest and alias table. `manifest.tsv`
and `catalog.tsv` are extraction/audit metadata and must not become additional
runtime authorities. The next cleanup is data quality: late-ammo assets and
all moving intro sprites must be corrected at the extractor/source-data
boundary, not hidden by runtime fallbacks. Complete audio now follows the same
boundary: WAV files are direct runtime assets, while Z80/VGM intermediates
remain offline-only.

### Native image and effect model

Genesis graphics routines often combine tile decoding, palette selection,
CRAM updates, VDP transfers, sprite-table construction, and timing effects.
Those are source-decoding concerns. The offline image importer resolves the
source tiles and palette into ordinary host RGBA images. The native renderer
loads those images directly:

```text
source tiles + source palette
        -> offline decoder
        -> individually named RGBA image/sprite files
        -> runtime image loader
        -> ordered sprite instances / background images
```

The runtime should not have a palette animator, VDP upload queue, tile-plane
reconstructor, Genesis address arithmetic, or source palette mutation path.
When a scene needs a flash, fade, tint, glow, or other special effect, use a
Qt/native compositor effect or an authored alternate image. When a scene needs
animation, use a named sequence of individual images with source-derived
timing and placement. Do not recreate a palette-shift mechanism merely because
the Genesis used one to produce the original pixels.

### NativeRenderer

The renderer consumes a `RenderFrame` made of:

- background/map layers;
- ordered sprite instances with alias, position, frame, flip, palette and
  visibility;
- text/glyph runs at source-derived positions;
- HUD/menu primitives;
- palette/fade parameters.

It may use a software RGBA surface for exact composition or Qt painting for
static interfaces. It must preserve source object order, transparency,
projection windows, animation cadence, and layer ownership. It must never
simulate VRAM, a VDP, or Genesis address arithmetic.

## 4. Offline-only source and extraction tree

These pieces are intentionally outside the game runtime:

```text
decompilation/tools/ghidra/
  ExportShadowrun.java       one canonical naming/export script
decompilation/export/
  functions.tsv, callgraph.tsv, call-sites.tsv, ghidra-c/, SCRATCHPAD.md
  ghidra-c/completed/ (only fully converted and independently verified exports)
decompilation/ghidra_conversion/
  typed source-understanding tests and conversion contracts only
  include/shadowrun_ghidra/{core,entities,field,world,menus,cyberspace,
    title_intro,session,audio,rendering}/
  src/<domain>/ and tests/<domain>/ follow the same ownership boundaries
  one conversion archive; focused tests remain individually runnable
decompilation/src/graphics_import.cpp
decompilation/src/graphics_extract.cpp
decompilation/src/audio_extract.cpp
decompilation/src/game_data_extract.cpp (or equivalent split)
  ROM -> host RGBA assets, complete audio, and typed ExtractedGameData
  no native gameplay state or runtime ROM access
decompilation/assets/
  resources.tsv plus bare runtime assets organized by semantic ownership:
    boot/ title/ intro/ movies/ menu/ field/ items/ audio/ native/
  item art is further split into weapons/first-aid/miscellaneous/magic/armor
decompilation/webm_ffmpeg_microframes/
  isolated visual reference breakdown; never a runtime source
```

The Ghidra conversion layer is not linked into the native runtime. It records
what the ROM does and supplies tests/evidence for the native rewrite. The
emulator/bus code is likewise a verification museum, not a second gameplay
implementation. The current `shadowrun_port` still contains bus-shaped code;
that is an implementation gap to remove, not part of this target architecture.
The conversion tree is grouped by these ownership domains rather than one
library or source directory per exported address; its detailed layout is in
`docs/ghidra-conversion-source-layout.md`.

## 5. What the 910 entries become

The current export/ledger pass is complete only in the narrow source-coverage
sense: all 910 exported functions are represented, with the ledger also
retaining non-function ROM anchors used by the native audit. The conversion
tests now group some leaf entries into typed subsystem contracts. That does
**not** mean the game is fully playable or every hill is defended: the ledger
currently has 928 `partial` and four `defended` rows, with no `pending` rows in
the current ledger snapshot. The 119 formerly generic exported names now have
reviewed semantic labels; that naming pass does not close their behavioral
verification hills.

The realistic native source count is therefore roughly a dozen ownership
areas, not 900 files:

```text
app / frontend
session / scene controller
field world / terrain / collision
entities / behavior / effects
menus / dialogs / inventory
intro choreography
rendering / text / asset store
audio
persistence
offline extraction tools
tests / visual dumps
```

The existing 229 CTest cases are evidence fixtures, not a proposed runtime file
layout. As a hill becomes native and defended, its conversion helpers should
be folded into the relevant native subsystem or retained only as a source-trace
test; they should not be linked wholesale into the game.

## 6. Migration order

1. Keep the current native startup/field slice stable and preserve the source
   audit tests.
2. Split source-data extraction from graphics extraction and enumerate every
   ROM table consumed by gameplay: records, rules, equipment, spells, events,
   text, and persistence data.
3. Define the generated `ExtractedGameData`/`RulesDatabase` boundary and the
   mutable `RunnerRoster`/`GameSession` boundary. Do not route native gameplay
   through ROM offsets or conversion helpers.
4. Finish direct asset regeneration and manifest cleanup, especially audio and
   individually extracted intro sprites. Resolve Genesis palettes during
   extraction and emit ordinary host images; do not carry palette/VDP logic
   into the runtime.
5. Turn `SceneState` and `GameSession` into the single authoritative native
   state owner; move remaining screen logic out of frontend code.
6. Complete one hill at a time: trace, typed state, focused CTest, independent
   image dump, then fold the corresponding conversion contract into native.
7. Add remaining field interactions, menus, save/load, combat, Matrix, and
   audio only when their ROM entry, data tables, exit paths, and captures are
   traced. Do not fill pending hills with invented content.
8. Keep Ghidra and the emulator buildable as reference tools, but prevent both
   from becoming runtime dependencies.
