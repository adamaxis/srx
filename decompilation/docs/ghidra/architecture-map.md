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
        |        +--> ConversationController (runner dialogue/service graph)
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

The separate `qt_runtime` frontend follows the same boundary
for the conversion executable: Qt owns the Windows key events, while the
converted field coordinator owns the typed controller state. Its field input
path is `host port byte -> 0x195C held/new-edge sample -> 0x0BF2 activity and
Start latches -> 0x0952..0x0B9C dispatch`; Genesis Start remains port bit 7 and
is not substituted with the startup menu's A action. This adapter is kept out
of the older native-port and emulator targets.

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
- recruited-runner conversation/service state: the runner-specific opening
  prompt at `0x211E8`, the ten-entry information-branch table at
  `0x2401C..0x2410C`, and the short-term single-run/permanent-hire command
  family at `0x21226` through `0x213CA`, with typed information flags,
  runner biography prompts plus Ricky's concrete Glow City/Rat Nest/Johnson
  subject tree at `0xFE848..0xFE90C`, Winston's Halloweeners/Lone Star/troll
  subject tree at `0xFEA8C..0xFEAF4`, and Trent's mages/contacts/Johnsons
  subject tree at `0xFEC78..0xFECE0` including the `0x24964` Jefferson-contact
  flag mutation, fee inputs,
  primary/secondary roster-slot selection, active-entity handoff, and field
  animation/upload callbacks. The ten-entry fee schedule, source cost
  arithmetic, and title-only roster-full prompt are typed;
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

### ConversationController and source-complete service graph

The runner dialogue is now an explicit integration boundary rather than a
collection of unrelated information functions:

```text
ConversationController
  -> common runner prompt (0x211E8)
  -> information opener / knowledge flag (0x2401C..0x2410C)
  -> source-complete subject graph
       Ricky  -> Glow City / Rat Nest / Gunderson
       Winston -> Halloweeners / Lone Star / troll origin
       Trent  -> mages / Jefferson contact / local Johnson
  -> hire offer preparation (0x21226)
  -> typed payment and roster-slot mutation (0x21318..0x21332)
  -> presenter-facing text/page result
```

`integration/recruited_runner_conversation_conversion.*` owns the cursor and
routes typed commands between these already converted domain boundaries. It
does not render Qt controls or duplicate `GameSession` state. The remaining
runner subject families intentionally stop at a named
`source_boundary` until their ROM descriptor chains are converted; that is a
conversion-audit work item, not a placeholder screen. Trent's contacts route
also exposes the proven `FFFC2C` story-flag mutation while leaving the
Pocket-Secretary record consumer explicit. This is the architecture work that
turns leaf conversions into a usable system boundary without importing a ROM
command table into the runtime.

### Shared dialogue panel boundary

The NPC-facing panel is one presentation component, not one source file per
conversation. `qt_runtime/dialogue_panel.*` projects the converted result into
one typed shape:

```text
ConversationController result
  -> actor identity (source name)
  -> optional source-bound portrait alias
  -> text segments / page cursor
  -> subject or payment options
  -> < MORE > / return-to-conversation state
  -> explicit source boundary when a record family is still missing
```

The adapter handles the common prompt, information pages, subject choices,
multi-page responses, service offers/results, and unresolved branches through
the same panel contract. It does not dispatch input or invent labels: input
continues through `dispatch_recruited_runner_conversation()`, and common
choice labels remain empty until their ROM text records are traced. This keeps
the panel reusable for Stoker, runner conversations, package/letter pages,
and other NPCs while leaving NPC-specific text and flags in source-derived
records.

The first field actor binding is the Stoker's Coffin Motel profile selected by
`0x1F65A`. Its profile table at `0x121FA0` points to the `0x1287C6` pattern
stream and `0x122420` 7x9 portrait stream; the offline importer emits the
single direct asset `dialogue.portraits.stoker.motel` (56x72 host pixels).
The portrait is now source-tested. No other field-exit profile is allowed to
borrow that portrait alias. The shared Qt presenter looks up each view's
`portrait_alias`; an empty or unavailable alias remains portraitless rather
than falling back to Stoker. Stoker is still the only extracted field-NPC
portrait currently bound to this route.

The Stoker conversation now runs from
`gameplay/dialogue/conversations/stoker_motel.json`. Its indexed-service graph
is separate from the signed field-exit graph; both use the shared JSON
conditions, speaker/menu catalogs, ROM text catalog, Lua graph runner, and
typed Qt presenter. `michael_package.lua` registers named host effects for the
source wallet and story transactions. The older typed
`StokerMotelPromptView` conversion remains for focused resource/dump QA, not as
the production branch policy. The panel receives stable text and choice IDs;
it does not carry ROM addresses or Genesis command bytes.

After the typed Stoker arrival record is selected, the ordinary `0x249C4`
branch now runs through the shared Qt presenter: rolls 0..79 select the source
`0xF4AB4` two-choice manager record, while 80..99 remain at an explicit
boundary until the `0x1E1FC` effect sequence is connected. The host uses the
source bounded-random algorithm, but its field seed is currently initialized
by Qt. The separate three-choice root at `0xF4AFC` belongs to indexed trigger
0 (`0x53AB6`), whose ROM gate is FFFC1E bit 0 clear, scene/event 2/2, and
alternate result 0. It is the panel shown in independent section 03 frame
000301; it does not replace the two-choice post-arrival record. The
`qt_runtime/field_dialogue_runtime.*` seam connects both source routes to the
shared panel without duplicating field state or putting ROM pointers in the
runtime. Its controller path preserves source A/B/C priority and title-only
wait semantics.

The Hotel handoff is now covered from ordinary New Game field input. Scene-2
record 91 (`0x173138`, `0xC2`) opens Stoker arrival; A resolves the bounded
manager branch, A opens the room offer, and A pays the source 20-nuyen charge
from the live field wallet. ROM `0x21458` writes the reduced balance and sets
`FFF10D` negative; after the paid message, the `0x1F65A` caller enters Hotel
Options at `0xCF38`. `FieldSceneRuntime::advance_frame()` carries the same
typed party records through Hotel status, the separate Hotel Karma upgrade
page, checkout, and the field redraw. The Stoker property has no interior map;
Hotel is presented as its source menus. The Debug/Release Hotel QA follows
this whole route with controller-port input and writes 223 consecutive
320x224 frames, including the initial spawn and a 30-frame field hold after
checkout. A deterministic PRNG seed selects the manager branch and
stabilizes the Hotel animation; the production PRNG seed remains unresolved.
The complete route also confirms the source manager choice and payment feed
the existing generic dialogue and Hotel controllers rather than starting
those systems from diagnostic helpers. Independent Hotel screenshots remain
unavailable, so exact Hotel visual fidelity stays open. The hill remains
`partial` until package-state selection, rare-effect handling, other field
placement, and independent visual defense exist.
No other field-exit profile is allowed to borrow Stoker's portrait or text
aliases.

The typed room-payment command `0x21458` uses the field wallet and source
20-nuyen charge. It opens the corresponding insufficient-funds or paid
message, preserves the wallet on failure, and deducts the charge on success.
Restored-save wallet state is unavailable, so the Qt controller stops at a
source boundary instead of guessing a balance for a loaded game.

The manager-package C option now dispatches source command `0x30` through the
typed follow-up resolver: it sets bit 1 of `FFFC30` and opens the F4E8C debt
question. The question's B option dispatches command `0x31`, sets bit 0 of
`FFFC27`, and opens the F5AB0 Jump House referral. `FieldSceneRuntime` owns
these optional field flags; New Game seeds `FFFC30` bit 0, `FFFC27` clear, and
the Contacts gate `FFFC2C` to `0x1C7F` after the startup clear. The Notebook
builds Contacts, Mr. Johnsons, and Shadowrunners from `FFFC2C`, `FFFC27`, and
`FFFC2A` at each entry, so the referral reveals its row and a missing loaded-
save value remains a source boundary. The debt command also unlocks the next
Tips & Clues record through the shared `FFFC30` flag word. The follow-up QA
dumps the two prompts, returns to the field, opens the Notebook, and checks
both updated menus. Source command `0x05` (`0x213FA`) still owns debt payment;
its funds/payment-result branch is not connected. Independent section 03
frames 000600-000602 show the debt prompt, while 000690/000750/000780/000840
show the referral page.

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

The first conversion-to-Qt seam is now explicit for the field command
overview:

```text
FieldRuntimeCoordinator (converted Start edge)
    -> FieldCommandOverviewBridge (non-blocking host frame adapter)
        -> menu_service_sequence_conversion (source selector/panel/cursor)
            -> FieldSceneRuntime (typed presentation state and direct assets)
```

The bridge owns no ROM image and does not create a second menu state machine.
It enters selector `6`, passes the live runner's typed record fields (including
the command-panel `+0x2C` value), preserves the source d-pad/A/B/mode priority,
and suspends field movement while the source command loop owns frames. The Qt
layer paints the extracted shell, source text catalog, portrait, status strip,
inventory icons, equipment overlays, cursor, and command focus blocks at the
source-derived positions. The normal equipment-detail seam passes the typed
`EquipmentCombatBarResult` and `ItemActionDetailResult` from converted code
into that presentation, so the item art, phase-value tiles, Damage/Power,
Wpns/Guns, and Atk/Def rows are not recomputed by Qt. The label catalog is
generated offline from ROM text tables into `dialogue/script-text.json`; the
action-detail data for Attributes, Cyberware, Magic, and Pocket Secretary
remain explicit conversion work. The top-level Clips branch now runs through
`gameplay/scripts/menu/screens/clips.lua` and its source-derived arithmetic in
`gameplay/scripts/menu/rules/clips.lua`. C++ exposes typed party/ammunition
facts, magazine capacity, and a narrow ammo-record write; Lua owns selector,
confirmation, trade, distribution, and result-wait state. `FieldSceneRuntime`
mirrors that state into the Qt presenter. The renderer loads separate named
source rows for top-level Clips (`0xCD33E`), nested Trade (`0xCD37A`), and
Reload confirmation (`0xCD3B6`); it no longer reuses the inventory action row
(`0xCD302`) beneath those screens. The Trade path consumes the same typed
`FieldPartyRosterRuntime` records used by the field scene: `0x11DD2` recipient
filtering, `0x1209A` recipient cycling, `0x120E4` quantity changes, transfer
arithmetic, and `0x11FDE` distribution are all scripted. The selected
runner's display name comes from its live-record `+0x58..+0x65` field; source
`D1B71` `????` text remains the fallback for a missing name. Loaded-save field
composition remains an explicit boundary.

The Clips branch also establishes the menu verification shape for later
panels. Selection movement is tested across every entry and wrap edge, while a
frame-shell comparison ignores only the source action row and its changing
caption. This catches a focus sprite painted over a stale five-button row, or
a selector transition that changes the portrait, inventory, status bars, or
other surrounding panel state. The Clips CLI QA also compares Debug and
Release full-screen captures for root, nested Trade, reload confirmation,
messages, and return states; its source-asset comparisons remain internal
checks because there is not yet an independent Genesis Clips capture. Route
tests also cover the ROM's byte-wrapped distribution total and quotient writes
to inactive roster slots. The party-roster test checks new-game Joshua
projection, signed save-marker activation, active-slot selection, and typed
clip transfer without allowing the Qt presenter to fabricate a local party
array.

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
`analysis/retired/webm_ffmpeg_microframes/` as a reference-analysis artifact. It is never part
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

## 4. Active project and offline source tools

The repository-root Ghidra conversion project is the active host application.
Its typed C++ and Qt layers consume extracted files and present authoritative
Lua gameplay/menu state. No 68000 interpreter, VDP, or ROM-addressed runtime is
part of the project.

Root ownership is:

- include/, src/, qt_runtime/, and tests/: active native model, Qt host, and QA.
- gameplay/scripts/ and gameplay/world/: Lua content and authored world data.
- assets/: individually named assets consumed by the runtime.
- input/ and export/: canonical ROM input and Ghidra source evidence.
- decompilation/ghidra_conversion/tools/rom_research/: offline ROM/Ghidra analysis and asset extraction tools.
- docs/ghidra/: source traces, behavior audits, and reconstruction status.

The older native-port code is retained under
archive/legacy_native_port/decompilation/. The local Ghidra database is kept
under archive/ghidra_analysis_project/. Neither archive is an active build
target. The prior screen-capture analysis is under
archive/retired_analysis/analysis/retired/webm_ffmpeg_microframes/; current
longplay evidence remains under video analysis/.
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
