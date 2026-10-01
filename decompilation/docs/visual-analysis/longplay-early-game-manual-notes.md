# Longplay manual visual analysis: opening through 12:00

## Purpose and evidence boundary

This is a hand inspection of the independent longplay capture, not a test
report and not a claim that the capture is pixel-perfect Genesis evidence. The
ROM remains the behavioral authority. The video supplies a second, human-
inspectable view of screen ownership, visible choices, composition order,
timing, and the transitions that the current conversion must explain.

Source: `video analysis/long_gameplay_video.mp4`.

All persistent frame/audio evidence is in the single base folder
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`.
Section 01 is the boot/title folder inside that base; sections 02-07 continue
the same source timeline.
The source is 30 fps; section frame `N` maps to source time
`section_start + N/30`. Every section has a same-length WAV so a sound cue can
be checked against the visible transition without re-extracting anything.

The capture is visibly a longplay rather than a clean emulator oracle: the red
text at the beginning is the uploader's video overlay, not a game asset, and
the capture includes pauses/input choices. Timing below is therefore a
source-video anchor, not a replacement for the ROM's VBlank counters.

## Section index and hand observations

### 01. Boot, logos, and title handoff — 00:00-00:56

Evidence folder:
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_01_boot_title_0000-0056/`.

Observed sequence:

1. Black/initial video frame, then the SEGA shock figure with lightning.
2. The SEGA mark resolves, followed by the FASA Corporation mark.
3. The Shadowrun logo assembles and holds with the copyright/start prompt.
4. At about 00:00:56 the title presentation hands off to the Start Options
   panel. This is the end of the non-interactive boot/title portion used for
   the section boundary.

The earlier conversion already has source-facing boundaries for this path:
`0x5D746` pre-title sequencing, `0x5D938/0x5D996` sprite batches, `0x5D8BA`
strobe palette windows, `0x59092` Start Options, and `0x59248` archetype
selection. The video confirms that the shock figure, FASA mark, Shadowrun
logo, and title prompt are separate visual stages; they are not one bitmap
flipbook asset.

### 02. Gator Shaman selection and opening story — 00:56-01:58

Evidence folder: `section_02_archetype_opening_story_0056-0118`.

The player starts on `New Game`. The archetype cursor visibly moves from
`Samurai` through `Decker` to `Gator Shaman`; the Gator/Shaman portrait is
visible immediately before the screen fades to the story. This is important:
the current integration test's new-game example selects Decker, while the
captured route selects Shaman. The conversion has the named choice and the
three source records, but the captured Shaman route needs its own behavioral
and visual coverage rather than being inferred from the Decker test.

The story then proceeds as a composed scene, not as a series of full-screen
bitmaps:

- green text introduces the date and Seattle, UCAS;
- the lower blue Seattle skyline plane remains as a background layer;
- wilderness/ambush sprites enter over black and text;
- muzzle/projectile/flash/body layers change while the ambush text advances;
- the tri-vid cockpit scene appears as a framed image with the recovered
  footage/cybereye text;
- the aircraft/cab and Seattle map imagery carry the player toward the field
  start; the final frames transition to the first HUD field view.

Source comparison: this matches the typed `OpeningIntroScene` sequence
(`seattle`, `wilderness`, `trivid`, `airport`, `cab_map`) and the source text
records in `opening_intro_conversion.cpp`. The visible order agrees with the
`0x5ADD0` schedule and the stage-2/orchestrator/stage-loop boundaries. The
video does not prove source frame counts because the player-controlled
selection precedes it and the capture may include timing differences; it does
prove that the code needs separate text, background, and sprite layers.

### 03. Jump House, Stoker, and first job setup — 01:58-03:58

Evidence folder: `section_03_jump_house_stoker_job_0118-0238`.

The transition first shows a Stoker portrait/dialog panel explaining the Jump
House route. The lower command area presents three explicit choices:

- `A` information;
- `B` work;
- `C` talk later.

The player opens information and work branches, advances `<MORE>` pages, and
returns to the field/interior view between conversations. The Jump House
description identifies it as a small bar inside the Redmond Barrens. The end
of the section is the first sustained field/interior view with Joshua's HUD
on the right.

This is not generic dialogue. The visible portrait, text source, option rows,
and return-to-field behavior identify a named Stoker interaction. A source
audit corrected the old attribution: `0x20E48` selects the electromagnetic-lock
door prompt at `0x11CCCE`, while `0x20EC2` selects the building Security
Terminal prompt at `0x11CD62`. Neither is the Stoker motel text. The Stoker
text records and their event/portrait descriptor still need to be traced from
the ROM; the old `NativePortShell` tuple is not conversion evidence.

### 04. Facility/corridor movement and NPC/package dialogue — 03:58-06:30

Evidence folder: `section_04_field_corridor_npc_dialogue_0238-0390`.

The player moves Joshua through dark-blue floor corridors and gray industrial
walls. The right HUD remains present while the map scrolls and the player
sprite changes position. A receptionist panel appears with a package/signature
interaction; the visible text says to sign for it, use a fake name, and move
on. Other panels show a Halloween/ghoul confrontation and escort/package
instructions. The video repeatedly alternates between field movement and a
full-width dialog panel; it does not replace the field with a baked dialog
background.

The important rendering observations are:

- the field play area ends at the opaque 48-pixel right HUD column;
- color-zero field pixels are transparent over the map, but the HUD chrome is
  not transparent;
- NPC portraits and dialog frames are separate layers from the field;
- option rows are only present when the source interaction exposes choices;
- field return preserves the current map/camera rather than restarting a
  generic screen.

The current Qt compositor's opaque-HUD rule is the relevant implementation
boundary (`0x334C`/`0x39F2` and the typed active-runner HUD path). This section
should be used to catch regressions where the sidebar accidentally shows the
map through it.

### 05. Repeated Stoker job branch and field traversal — 06:30-08:30

Evidence folder: `section_05_field_and_package_dialogue_0390-0510`.

The player returns to Stoker and again receives the three-row information/work
menu. The visible work text routes Joshua toward a contact in the Redmond
Barrens and a package delivery, with a payment/acceptance prompt. The player
then traverses the field map through the same industrial visual language. The
capture shows several pauses on `<MORE>` and on the Yes/No job prompt, so these
are real input-driven states rather than animation frames that should be
collapsed into one image.

The conversion-side behavior to compare here is the field-mode/dialog handoff:
`initialize_field_scene_entry`, `advance_field_scene_transition`, the field
interaction action dispatcher, and the named Stoker/dialog boundary. A generic
"show message" callback is not enough: the source needs the current NPC,
dialog page, option cursor, payment value, and return mode.

### 06. Combat/item detail and package delivery — 08:30-10:00

Evidence folder: `section_06_combat_inventory_delivery_0510-0600`.

The capture includes a field encounter, a runner/item detail panel, and a
well-dressed elf receiving the package. The item panel uses the Joshua portrait
and a detailed item/ability sheet; it is not the same surface as the compact
right-side HUD. The player returns to field movement after the interaction.

The relevant conversion boundaries are the typed inventory/action path
(`0x11740`, `0x12168`, `0x1253E` and their named leaves), runner-record item
fields, and the field dialog return. The video is particularly useful here
because it exposes the visual distinction between possession/action detail,
runner status, and the ordinary field HUD.

### 07. Medkit use, Stoker follow-up, and letter/Holopix sequence — 10:00-12:00

Evidence folder: `section_07_letter_stoker_followup_0600-0720`.

The first frames show Joshua's item panel with `Medkit` selected and the
`Heal who??` target prompt, followed by the explicit `Medkit used.` result.
The player then returns to the field, reopens Stoker's dialog, and advances a
long letter/story chain. The panels name Michael's death, Tabatha Shale, the
Puyallup Barrens, a cyberdeck, and later Holopix/hospital contacts. The player
advances each `<MORE>` page; these pages are distinct text states with the
same panel shell.

This section maps directly to the source distinction between first-aid action
logic, dialog text/page state, and field return. It is evidence against using
one generic inventory panel or one generic dialog string for all of these
behaviors.

## Screen sketches and technical panels

These are hand-drawn layout references at the source-sized 320x224 host
presentation. Coordinates are approximate observations, not replacement for
ROM tracing.

### Start Options / archetype selection

```text
  x=0                                                            x=319
  +----------------------------------------------------------------+
  |  gray device bezel                                             |
  |      +-----------------------------------------------+         |
  |      |                 Start Options                 |         |
  |      |              -> New Game                      |         |
  |      |                 Load Game                     |         |
  |      |                                                   [I/O] |
  |      +-----------------------------------------------+         |
  |                 bottom hardware buttons                       |
  +----------------------------------------------------------------+

  Archetype state replaces the interior contents, not the bezel:
      [portrait]       Choose Archetype
                       Samurai
                   ->  Decker
                       Gator
                       Shaman
```

Technical panels:

- outer device/bezel is persistent presentation;
- title and cursor are separate text/grid requests;
- portrait changes with the selected archetype;
- cursor movement is input state, not animation;
- the observed route moves twice before the fade, selecting Gator Shaman.

### Opening story composition

```text
  +----------------------------------------------------------------+
  | green source text / story captions                             |  <- text
  |                                                                |
  |              sprite groups / tri-vid cockpit                  |  <- SAT
  |                                                                |
  | [blue Seattle plane / background patch / cab or aircraft]      |  <- plane
  +----------------------------------------------------------------+
```

The same 320x224 frame can contain text, a background plane, multiple SAT
sprite groups, and a palette transition. The source conversion's
`OpeningIntroSpriteFamily` and `OpeningIntroTextAsset` types reflect this
partition. A frame dump is useful for measuring timing, but it must not become
the runtime rendering technique.

### Field with active runner HUD

```text
  x=0                         x=272                 x=319
  +---------------------------+---------------------+
  |                           | portrait            |
  |      scrolling field      | weapon/item slots    |
  |   player + NPC sprites    | empty/filled slots  |
  |   terrain / doors / pipes | status markers      |
  |                           |                     |
  +---------------------------+---------------------+
       272px field viewport       48px opaque HUD chrome
```

Technical panels:

- map/terrain is the moving layer;
- entity sprites are overlaid at world-to-screen positions;
- HUD chrome is an opaque six-tile column, even where its internal slots are
  empty;
- item/action detail screens temporarily occupy the field presentation but
  are not the field HUD itself;
- no observation supports a universal transparent-black rule.

### Dialog and option panel

```text
  +----------------------------------------------------------------+
  | +---------+  +----------------------------------------------+  |
  | | portrait |  | NPC text / page                             |  |
  | | 64-ish   |  |                                              |  |
  | +---------+  +----------------------------------------------+  |
  |                                                                |
  | [A] information / [B] work / [C] talk later                    |
  |                 or <MORE>                                      |
  +----------------------------------------------------------------+
```

The portrait identity and page text are state. The lower rows appear only on
choice pages; `<MORE>` is an advance gate, not an extra sprite frame. Returning
to field after a page preserves the actor/camera context.

### Runner/item detail and first-aid target

```text
  +------------------------+------------------------+--------------+
  | Joshua portrait         | selected item/detail  | field HUD     |
  | action/icon grid        | name + properties     | remains at    |
  | equipment/action cells  | target/quantity text  | right edge    |
  |                        |                      |              |
  | inventory / magic tabs  | `Heal who??` / result |              |
  +------------------------+------------------------+--------------+
```

The `Medkit` -> `Heal who??` -> `Medkit used.` sequence in section 07 is a
concrete state chain. It must not be represented as a static item bitmap or a
generic action callback.

## Cross-check against converted code

| Video observation | Source/conversion boundary | Manual result and remaining work |
| --- | --- | --- |
| New Game, two downward moves, Gator Shaman selected | `0x59092`, `0x59248`, `0x42B0`; `startup_menu_conversion.cpp`, `startup_integration_conversion.cpp` | Choice names and three typed records exist. Existing integration coverage follows Decker; add/defend the captured Shaman route and its portrait/record binding. |
| Seattle -> wilderness -> ambush -> tri-vid -> airport/cab/map | `0x59C5E` chain; `OpeningIntroScene`; `opening_intro_conversion.cpp` | Visual order agrees with the typed scene families and source text records. Need independent frame/dump assertions for layer placement and fade/clear behavior; do not promote the longplay itself to a golden. |
| Stoker portrait, A/B/C menu, `<MORE>`, return to field | `0x1F65A` scene 2/submode 2; profile `0x121FA0` -> pattern `0x1287C6` / portrait `0x122420`; prompt records `0xF4AA0`, `0xF4AF4`, `0xF4AEC`; commands `0x213CA`, `0x213E2`, `0x213FA`, `0x21458` | The top-level Stoker prompt selection is source-identified as arrival/package-question/package-handoff. The portrait is now extracted as one 56x72 direct asset and the common dialogue-panel adapter is tested against converted page/choice state. Room payment has its exact success/insufficient-funds behavior; the debt/Jump House follow-up commands publish their source flags; and command `0x05` models the 250-nuyen payment, FFDD64-selected effect marker, runner-source increments, feature flag, story bits, and Michael package sequence. The Stoker prompt text records, panel shell, remaining package command chain, and complete return transition remain open; `0x20E48` remains the separate maglock doorway family. |
| Field map plus 48-pixel right HUD | `0x334C`/`0x39F2`, `0x110EC`, field scene controller | The opaque sidebar rule is visibly required. Recheck Qt screenshots at field/dialog return and defend the HUD compositing hill independently. |
| Package/signature and named NPC conversations | field interaction/entity/dialog records | Generic interaction callbacks are insufficient. Resolve NPC identity, text page, option cursor, package/credstick state, and return mode from the source records before implementing new visuals. |
| Runner item sheet and `Medkit` target flow | `0x11740`, `0x12168`, `0x1253E`, first-aid/inventory conversion modules | Converted action branches exist but have no defended visual hill for this captured route. The item-detail and first-aid states need separate typed presentation requests. |
| Letter/Holopix/hospital pages | dialog/text catalogs and field return | This is a multi-page text state, not one screen. Identify the source text records and page transitions, then compare the typed catalog to each observed page. |

## Opening palette/text verification pass

The uploader's red logo/text at the beginning of the capture is excluded from
the asset inventory and from all visual comparisons. It is not a Genesis
screen element.

The source and host dump were compared at the wilderness transition using the
numbered frames in section 02 and the deterministic Qt command
`shadowrun_ghidra_conversion_qt.exe --startup-dump opening 2 <frame> <bmp>`.
The comparison exposed two conversion/presentation errors:

- The ROM's `0x5AE9E` writer had been misread. It writes `without_warning` at
  `0x2186`, `ambushed` at `0x2286`, and then rewrites the five Seattle/team
  records at `0x210E`, `0x2210`, `0x2304`, `0x2384`, and `0x2404`.
- The converted callback treated `0x131C` (fade the current palette to black
  while installing a new resource set) and `0x12F6` (advance the new palette
  toward its target) as one per-frame transition. They are now separate source
  boundaries. The Qt host clears presentation state at the resource boundary,
  reveals the new scene, and does not simulate Genesis CRAM.

The source's `0x5AE3A` blank record is represented as a typed `blank` text
asset and removes the five prior story rows before the new `massacre`,
`captured`, and `slain` records are drawn. The source still emits the A000
Seattle/team writes during the wilderness pass; the Qt presentation filters
that bank in the wilderness scene because the independent capture shows those
rows invisible there. This preserves the converted writes while matching the
observed layer result instead of deleting source behavior.

The integration test now records and checks these exact text handles,
destination words, palette attributes, blank-row writes, and the six
fade-to-black/two target-fade callback split. The native dump is evidence for
the correction, not a visual golden; the opening hill remains partial until
the ROM trace, typed host state, regression test, and independent visual
comparison are all closed together.

## Immediate audit queue

1. Add a source-backed integration route for the captured Gator Shaman choice;
   verify the selected archetype record, portrait alias, and opening state.
2. Trace the named Stoker/Jump House dialog text records and their event
   descriptor. The portrait stream is now source-bound, but the old
   `NativePortShell::open_stoker_motel_dialog` tuple is explicitly not a
   source mapping. Do not invent text if the source record is not found;
   return to ROM conversion and report the omission.
3. Use the section frames to annotate field/HUD/dialog boundaries and compare
   them against the Qt output, especially the opaque sidebar and full-width
   dialog panel.
4. Map the `Medkit` target flow and the letter/Holopix pages to source text,
   action, and return-state entries. Keep inventory, first aid, dialog, and
   field state separate.
5. Continue sectioning the longplay only when a later sequence is needed for a
   specific source hill. Do not turn the entire three-hour video into an
   unindexed bitmap dump.
