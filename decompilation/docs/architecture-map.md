# Native architecture map

This is the architecture implied by the ROM, the regenerated Ghidra callgraph,
the typed conversion work, and the existing native source. It is the shape the
port should converge on. The 906 exported functions are not 906 original
source modules: they are mostly leaf routines, table-dispatch targets, transfer
wrappers, animation variants, and small record helpers. The export currently
has 2,978 resolved direct call sites and 16 indirect/unresolved boundaries.

## The short version

```text
Windows / Qt frontend
        |
        v
NativeGameController ---- InputState / FrameClock
        |
        +--> GameSession (typed runner, world, inventory, story, save state)
        |        |
        |        +--> SceneState: Boot -> Title -> Startup -> Field -> Menu
        |        |                         \-> Dialog / Combat / Matrix / Shop
        |        |
        |        +--> FieldWorld (area map, terrain, collision, entities)
        |        +--> MenuSystem (command focus, inventory, equipment, dialogs)
        |        +--> IntroDirector (sprite choreography and timed text)
        |        +--> AudioService (complete Windows-playable tracks/effects)
        |
        +--> NativeRenderer (background layers, individually named sprites,
        |                     object ordering, text, RGBA presentation)
        +--> ResourceManifest / AssetStore (direct assets only)

Offline only: ROM/resource extractors, audio conversion, manifest generation
Reference only: one Ghidra project/export, ROM traces, scratchpad, captures
```

The arrow direction is ownership. The renderer and Qt widgets present typed
state; they do not own a second game state machine. The extractor creates host
assets; the runtime never decodes ROM pointers or reads Genesis RAM addresses.

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
  -> Field area loop (0x3084)
       Start -> command overview and submenus
       raw event -> area transition / dialog / combat / special state
```

Indirect ROM dispatches remain explicit in the native model as typed state
transitions. They are not generic function-pointer tables copied from the
ROM. The known state-pair and handler-table families around `0x1F580` and
`0x214B0` should become named transition policies as each hill is traced.

### Authoritative session model

`GameSession` owns the data that changes during play:

- current scene/mode and scene-local cursor/focus;
- the active runner and roster eligibility;
- the 0x100-byte-equivalent typed runner record, including the distinction
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

The field loop combines movement, four-corner terrain probing, camera motion,
entity projection, animation, and mode transitions. It should not be split into
one class per decompiled function. The source families around `0x2B34`,
`0x2CE0`, `0x2F32`, `0x3084`, `0x3A1C`, `0x3BAC`, `0xE7C8`, `0xB7B8`, and
`0x167AE` are natural methods/services on this subsystem.

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

The source audio functions around `0x52D..0x53` are a command/bus producer,
not the Windows playback backend. Native `AudioService` should receive named
music/effect events and play complete files in a Windows-supported format
(for example WAV for effects and an appropriate complete-track format for
music). It must not expose or consume the old broken fragments.

The emulator/bus transcription may remain as a museum/reference path. It is
not a dependency of native gameplay.

## 3. Presentation and resources

### AssetStore and resource manifest

The runtime uses one flat, human-maintained resource list with aliases:

```cpp
assets["title.logo"];
assets["intro.helicopter"];
assets["field.entity.walk.0.1"];
assets["audio.title"];
```

The manifest is the complete runtime allowlist. It should point directly to
individual extracted images, maps, sprites, text tables, and complete audio
files. A runtime load loop iterates manifest rows; it does not contain one
assert or one hard-coded loader branch per asset.

The current direct-assets boundary is already represented by
`assets/manifest.tsv`, `assets/catalog.tsv`, `assets/resources.tsv`, and the
runtime manifest reader. The next cleanup is data quality: late-ammo assets,
all moving intro sprites, and complete Windows-playable audio must be corrected
at the extractor/source-data boundary, not hidden by runtime fallbacks.

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
decompilation/ghidra_conversion/
  typed source-understanding tests and conversion contracts only
decompilation/src/graphics_import.cpp
decompilation/src/graphics_extract.cpp
decompilation/src/audio_extract.cpp
  ROM -> direct host assets; no native game state
decompilation/assets/
  manifest and bare runtime assets only
decompilation/webm_ffmpeg_microframes/
  isolated visual reference breakdown; never a runtime source
```

The Ghidra conversion layer is not linked into `shadowrun_port`. It records
what the ROM does and supplies tests/evidence for the native rewrite. The
emulator/bus code is likewise a verification museum, not a second gameplay
implementation.

## 5. What the 906 entries become

The current export/ledger pass is complete in the narrow source-coverage
sense: all 906 exported functions are named and represented, with the ledger
also retaining nine non-function ROM anchors used by the native audit. The
conversion tests now group the remaining leaf entries into typed subsystem
contracts. That does **not** mean the game is fully playable or every hill is
defended: the ledger still has 908 `partial`, four `defended`, one `ported`,
and two `pending` rows.

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

The existing 124 conversion test targets are evidence fixtures, not a proposed
runtime file layout. As a hill becomes native and defended, its conversion
helpers should be folded into the relevant native subsystem or retained only
as a source-trace test; they should not be linked wholesale into the game.

## 6. Migration order

1. Keep the current native startup/field slice stable and preserve the source
   audit tests.
2. Finish the direct asset regeneration and manifest cleanup, especially audio
   and individually extracted intro sprites.
3. Turn `SceneState` and `GameSession` into the single authoritative native
   state owner; move any remaining screen logic out of frontend code.
4. Complete one hill at a time: trace, typed state, focused CTest, independent
   image dump, then fold the corresponding conversion contract into native.
5. Add the remaining field interactions, menus, save/load, combat, Matrix,
   and audio only when their ROM entry, data tables, exit paths, and captures
   are traced. Do not fill pending hills with invented content.
6. Keep Ghidra and the emulator buildable as reference tools, but prevent both
   from becoming runtime dependencies.
