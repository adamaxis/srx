# Qt conversion visual-audit method

This is the working procedure for deciding whether a converted screen is
faithful enough to advance. It applies to the entire Qt conversion, not only
to the defects that have already been reported.

## Evidence order

Every audit item has three separate evidence channels:

1. **ROM behavior.** Identify the entry point, input/cursor state, records or
   tables read, draw calls/layer order, timing counters, and exit transition.
   The ROM is authoritative. Ghidra output is a cross-check and a place to
   annotate recovered calls; it is not a runtime specification.
2. **Independent visual reference.** Use the hand-inspected frames from
   `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`.
   Record the section, frame/range, visible selection, shell, moving layers,
   transparency, and transition state. The uploader's red opening mark is
   excluded as a game asset.
3. **Native output.** Run the deterministic Qt dump for the exact route and
   state, inspect the 320x224 image by hand, and record the artifact path.
   Internal same-source compositors are regression checks only: they can catch
   a renderer changing its own extracted assets, but they cannot prove that
   the extraction or interpretation matches Genesis.

An item is not source-verified because a unit test passes, because a named
asset exists, or because a screenshot contains some expected pixels.

## Seven-question anomaly gate

Run this gate before changing any screen that looks suspicious, then answer it
again against the corrected native dump. Keep the answers with the ledger row
or its linked review note. `unknown` is an acceptable result when the supplied
evidence cannot settle a question; it is not a pass.

1. **Does it look correct?** Identify the suspected region and describe the
   visible mismatch in plain terms. Compare the whole source-size frame and
   record the reference and native artifact paths.
2. **Does the look reflect the actual behavioral state?** Name the typed state
   field and ROM record/table that drive it. Check that the chosen, equipped,
   active, unavailable, or disabled state has every source-visible attribute
   (icon, cursor, tint, border, label, and value as applicable). Record the
   input that reached the state and the value before and after it.
3. **Are the visuals themselves correct?** Trace the ROM draw path and source
   placement, then compare shell, layer order, text, palette/alpha, clipping,
   and geometry against independent gameplay frames. Do not infer correctness
   from the same extracted asset or compositor used by the native renderer.
4. **Is it animated, and should it be?** Identify the source counter or event
   that advances it, its cadence, its loop/hold/exit behavior, and the
   circumstances that enable it. Compare entry, stable, intermediate, and
   exit frames; mark a static element static only when the source path supports
   that conclusion.
5. **Does sound accompany it?** Trace the sound event at entry, selection,
   confirmation, rejection, and exit as applicable. Record its source event
   ID, extracted alias, measured clip duration, and trigger/lifetime. If the
   event or sound is unresolved, record that boundary instead of assigning a
   plausible-sounding asset.
6. **Are related parts of the screen also correct?** Inspect the full frame,
   shared shell, neighboring controls, persistent HUD, sibling pages, nested
   dialogs, return path, and every attribute driven by the same record or
   renderer. A confirmed defect triggers this wider scan before it is closed.
7. **Do multiple frames support the interpretation?** Record several
   independent reference frames: at minimum an initial state, a state reached
   by an input, and a later/return state when the capture contains them. Use
   adjacent frames to distinguish animation from a static drawing and other
   gameplay/menu visits to confirm repeated layout. One screenshot is not
   enough to establish state, cadence, or persistence.

For each answer, record concrete ROM addresses or typed fields, reference
section/frame numbers, and a native artifact. If the reference lacks the
necessary state or sound, leave that part `needs_independent_capture` rather
than silently treating the current rendering as correct.

### Applied review: startup options and Save/Load

1. **Appearance:** the independent Start Options frame is section 01 frame
   001679. It matches native `02_start_options_new_game.bmp` across the
   selected row, text, device bezel, and decals. Native snapshots
   `04_save_load_slot_one.bmp`, `05_save_load_no_save_message.bmp`, and
   `06_start_options_returned_from_load.bmp` retain the same shell. A first
   visual impression that the no-save frame was black-only was false; pixel
   sampling and the saved 320x224 image show the bezel around its message.
2. **Behavioral state:** `0x59092` owns New Game/Load Game selection;
   `0x59172` enters the two-slot loop; `0x59202` redraws the selected grid
   before load validation; and `0x59224` selects the no-save message at
   `0x1B6527`. Native QA reaches slot one, the no-save state, and the return
   to Start Options.
3. **Visual construction:** the host composites the named startup background
   with authored text/cursor positions. The Save/Load message replaces the
   old menu ink while leaving the outer background visible; native QA checks
   bezel sample pixels for each menu snapshot.
4. **Animation:** the menu rows and cursor are static between inputs. The
   source vertical input moves the cursor through `0xC376`; the no-save
   message waits for a new controller event at `0xD94`, then returns.
5. **Sound:** the traced `0x59092` and `0x59172` menu paths have no direct
   sound call for cursor movement or the no-save message. No selector/message
   sound is assigned without a source event.
6. **Related screen parts:** the review checked the bezel, decals, old menu
   text, slot rows, message, cursor, and return state. The shell persists and
   stale Start Options ink is cleared in the no-save screen.
7. **Frame support:** the longplay independently shows the selected New Game
   row at frame 001679 but does not show Load Game. The slot/message/return
   sequence is supported by ROM flow plus multiple native QA snapshots; it
   remains `partial` until an independent Load Game capture and valid-save
   route are available.

### Applied review: field inventory selector

The pre-fix NativePort page-1 dump showed several gold segments crossing the
selected item. That did not match the single fine outline in the longplay.

1. **Appearance:** compared the whole 320x224 frame and the inventory crop.
   The mismatch was confined to the selected item marker. Corrected output is
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-inventory-selector-qa.bmp`;
   section 06 frame 002560 shows the armor cell selected and equipped-cell
   treatment at once. Frames 002580, 002610, and 002640 show the medkit with
   the cursor isolated from that equipment treatment.
2. **Behavioral state:** the marker follows
   `FieldCommandOverviewView::source_state.primary_page` and
   `secondary_page`; the selected inventory slot is
   `secondary_page * 2 + primary_page` in the active runner's
   `inventory_item_codes`. ROM `0x11506` updates the secondary selector and
   `0x11592` builds the sprite record from the selectors and `0xD29F6`.
   The focused QA requires the reached cell to contain an item and verifies
   that its icon pixels remain visible inside the outline.
3. **Visual construction:** `0x11592` yields host sprite origin
   `(primary * 40 + 12, secondary * 24 + 116)`. The independent capture shows
   a one-pixel warm-gold outline inset two pixels from that origin. The
   repeated medkit edge in frames 002580, 002610, and 002640 samples as
   `#C4BE27`; both Qt and NativePort now draw the same one-pixel outline from
   the typed selected-cell state instead of the extracted sprite's extra
   internal segments. ROM `0x11624` installs equipped-cell treatment before
   the item glyph rows; the renderer order keeps the item art visible above
   that treatment. Fresh NativePort CLI captures are in
   `decompilation/build-qt-debug/menu-selector-verified/`.
4. **Animation:** the observed focus is stationary while held. It moves when
   the selector changes and remains through the Heal-who prompt and use
   result. It is still visible in section 07 frame 000030; the field view at
   frame 000120 has closed the menu. Frames 002580, 002610, 002640, and
   002699 show no blink or per-frame animation. ROM `0x11592` rebuilds the
   sprite record on a selector refresh; this path has no separate cursor
   animation counter.
5. **Sound:** D-pad movement reaches the command detail refresh, which emits
   audio event `0x25`; no alias or measured clip is identified, and the Qt
   command route does not play that event. Action-button dispatch uses `0x24`,
   mapped to `audio.menu.command-024` (2.484 seconds including its two-second
   release tail), but that is not evidence for a selector-movement sound.
   Movement sound identity and duration remain unresolved.
6. **Related screen parts:** inspected the item art, equipped-cell frame,
   center detail panel, field HUD, command shell, and the nested action row.
   Qt page-1 QA checks all four outline edges, rejects matching-color pixels
   inside, and requires populated item ink. NativePort's focused test checks
   the same geometry for the occupied Scatter cell before its later field
   movement assertion. Field-action QA checks that the Qt outline remains
   around its item when Equip opens. The
   longplay shows it around the medkit through Heal-who/use, but there is not
   yet a native First Aid state dump to verify that complete route.
7. **Frame support:** section 06 frames 002550, 002560, 002580, 002610,
   002640, and 002699 show the unselected menu, armor selection, movement to
   the medkit, and held action/result states. Section 07 frames 000030 and
   000120 show the result and later field view. These support the geometry,
   state change, and persistence; they do not validate unrelated command
   pages whose corresponding source states are absent. The native QA image
   checks an occupied armor cell in the Gator starter record, so this is not
   a same-inventory full-frame comparison.

The focused selector CTest is a renderer regression guard, not a Genesis
golden. The selected-item appearance is independently checked against the
listed longplay frames. The eight-cell equipment/empty-slot matrix and First
Aid return route remain open.

### Applied review: Seattle opening text and focus box

The original ledger's claim that the Seattle phase ran 5.7 times too long was
based on a signature match that compared 60 Hz native updates directly with a
30 fps video dump. That ratio is withdrawn. ROM schedule delays and the
sequence of distinct captured states are the evidence used below.

1. **Appearance:** before correction, the team paragraph stopped after its
   first row and the animated focus frame was drawn as a thick white box. The
   corrected native Seattle capture is `opening_600.bmp`; the five phase
   captures are `opening_604.bmp` through `opening_612.bmp` in
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/startup-qa/`.
   Independent references are section 02 frames 000300, 000400, and
   000409-000413.
2. **Behavioral state:** text rows are emitted from the converted
   `0x5ADD0` schedule. ROM `0x5AEEE` continues past its cover-row writer at
   `0x5AF02` and `0x5AF16`; the native typed `OpeningIntroTextAsset` records
   now place the three rows at `0x2304`, `0x2384`, and `0x2404`. The focus
   animation is driven by `OpeningIntroState::seattle_outline_frame`; five
   `0x5B378` passes each consume two VBlanks.
3. **Visual construction:** the source table at `0x5B364` and offsets at
   `0x5B36E` describe Plane-A transfers rooted at `D4=0x040E`; those are not
   host-pixel rectangles. The native plan uses five measured screen-space
   bounds from source frames 409-413 and paints a one-pixel amber outline.
   Compared phase by phase, the native captures reproduce the captured
   large-to-small box without the old thick white bars. The three story rows
   match the content and placement visible in frame 000420.
4. **Animation:** the five ROM writes are held for two VBlanks each. Source
   frames 409-413 show the five shrinking phases; frames 420-430 retain the
   small frame, frame 435 shows the skyline darkening, and frame 440 is black.
   Native captures include the final outline hold, fade-out, black handoff,
   and wilderness fade-in at frames 620-682. The 0x5ADD0 hold record and the
   0x1406/0x12F6 transition now run through typed fade state. Their discrete
   snapshots agree with the captured sequence of lit city, darkening city,
   black, and appearing actors; the sparse 30 fps capture cannot establish an
   exact frame-for-frame timing match.
5. **Sound:** `0x5B378` has no audio call of its own. The surrounding intro
   route sends event `0x10` at `0x59C76` (mapped by the current host table to
   `audio.field.effect-016`, 2.7505 seconds) and event `0x11` at `0x59C92`
   before stage 2 (mapped to `audio.intro.title.stage-1`, 14.8337 seconds).
   Those are entry/transition events, not a selector-specific sound. The
   exact audible overlap with the captured box is not independently confirmed.
6. **Related screen parts:** checked the whole Seattle composition: date,
   place, three team rows, skyline, focus box, and next wilderness scene.
   The text omission, focus-frame rendering, final-box hold, and palette
   transition are now modeled. Later wilderness cadence, tri-vid/airport
   composition, and their resource-boundary fades remain open; do not treat
   the Seattle pass as completion of the full intro hill.
7. **Frame support:** section 02 frames 000120/000150/000210/000300 show
   skyline-only, date, date/place, and the full text. Frames 000400-000430
   show the stable text and outline progression/hold; 000435/000440 show the
   fade and black; 000470/000480 show the wilderness actors. These multiple
   frames support state and animation classification. Native frames 620, 628,
   648, 654, 662, and 682 independently show fade-out, black, and fade-in
   states. They do not establish exact native-to-video timing at the handoff.

The startup CTest now saves all five native outline phases and checks a
one-pixel amber edge in each phase. This protects native rendering; the
longplay frames remain the independent appearance evidence.

### Applied review: Joshua archetype selection

1. **Appearance:** inspected the complete device screen, portrait, labels, and
   selection arrow. Source section 02 frame 000060 selects Gator/Shaman and
   matches native startup QA snapshot 09_archetype_gator_shaman.
2. **Behavioral state:** ROM 0x59248 commits the selected record used by the
   opening. Native Gator/Shaman reaches that route; Decker and Samurai have
   separate typed records, but the supplied capture does not show their
   selection states.
3. **Visual construction:** portrait, option text, arrow, and startup shell
   are separate layers. The shell remains visible around the selected record.
   Decker/Samurai portrait placement still needs independent frames.
4. **Animation:** the selected arrow is static while held and moves on the
   menu input edge; no independent blink or timer is traced for it.
5. **Sound:** no selector-specific sound is assigned without a source call
   and an identified event. The duration of any audible menu cue is unknown.
6. **Related screen parts:** checked all three option labels, the arrow,
   portrait, decals, device bezel, and handoff into the selected opening.
   Field HUD archetype QA verifies record propagation, but does not substitute
   for a visual audit of these three menu choices.
7. **Frame support:** section 02 frames 000050-000062 repeatedly show the
   Gator/Shaman selection. They support that state and its shell, not
   Decker/Samurai selection or their transitions. Keep those routes partial.

### Applied review: seven command selector positions

1. **Appearance:** freshly rendered and inspected every selector position at
   source size in
   decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-menu-pages-verified/page-0.bmp
   through page-6.bmp. The thin inventory outline is a one-pixel warm-gold
   inset; the earlier crossed-segment version is gone. Page 3 visibly lacks
   values and a selection cursor.
2. **Behavioral state:** the authoritative view carries primary_page,
   secondary_page, and the active runner record. ROM 0x10F50 starts at page 6;
   the seven positions map to inventory columns 0/1, Clips, Attributes/Skills,
   Cyberware, Magic, and Pocket Secretary. The base status panel remaining
   visible while pages 4, 5, or 6 are merely focused is source-correct: A
   dispatches those panels. A opens Cyberware at 0x11950 and Pocket Secretary
   at 0xC2A6; Magic dispatch at 0xFBA6 remains an explicit boundary.
3. **Visual construction:** pages 0/1 retain the two-column eight-cell
   inventory and selected-item detail; page 2 opens the Clips action panel;
   page 3 replaces the base frame and renders the 20 labels from 0xC1A2A;
   pages 4 and 6 open their own compositions on A. The page-3 values and
   0xD27C cursor are not rendered, and page 5's spell panel is not connected.
4. **Animation:** the tab highlight stays still until a D-pad edge changes
   primary_page; it is not a blinking animation. Inventory selection changes
   secondary_page by input. The separate Attributes/Skills cursor navigation
   and Magic spell cursor cadence are unimplemented and remain unclassified.
5. **Sound:** A dispatch uses event 0x24, extracted as
   audio.menu.command-024 with a 2.484-second file including its two-second
   release tail. The selector cue at event 0x25 has no identified alias or
   duration. No panel-specific sound is inferred from the tab label.
6. **Related screen parts:** inspected the shared command background, field
   HUD, portraits, selected tab, inventory art/equipment treatment, Clips
   rows, Cyberware panel and return, and Pocket Secretary entry. QA checks
   preserve the shell and HUD. The old command-bar pixel failure used
   nonmatching source frames and is withdrawn.
7. **Frame support:** section 06 frames 002550, 002560, 002580, 002610,
   002640, and 002699 show Clips/inventory states and movement/result
   persistence. They do not show the other tab panels. The seven native page
   images prove current rendering only; retain partial until independent
   captures cover the missing pages and archetype/save states.

### Applied review: wilderness story and actor sequence

1. **Appearance:** source section 02 frames 000542, 000570, 000638, and
   000696 show the ambush, text, projectiles, and later actor states.
   Fresh native snapshots include the wilderness reveal at frame 688, the
   story paragraph at 849, and animation samples in
   qt_runtime/wilderness-recheck/frame-1030.bmp through frame-1090.bmp.
   These show a composed scene rather than a missing cockpit/airport asset.
2. **Behavioral state:** ROM 0x59D7C loads D7=0x1F0 and compares against
   0x164; the story write is driven at elapsed index 0x8C. Native text timing
   uses that elapsed value. Source animation records and typed stage-loop
   state drive the actors and map patches.
3. **Visual construction:** map patches, foreground actors, projectiles,
   shadow/death effects, and text remain separate layers. The selected
   snapshot comparison supports the text and major actor composition; exact
   sprite-by-sprite placement and every later animation descriptor are not
   independently reconciled.
4. **Animation:** actors and projectile effects advance through the source
   frame services. Compare entry, story-text, explosion, and later frames; do
   not treat a pose hold in a mismatched native/video frame pair as a cadence
   defect. Exact cross-capture VBlank alignment remains unknown.
5. **Sound:** the route emits event 0x11, mapped to
   audio.intro.title.stage-1 (14.8337 seconds). Later actor-event aliases
   and whether that track is stopped or allowed to finish at each scene
   boundary remain unresolved.
6. **Related screen parts:** checked both sides of the composition, text
   baseline, map, actor silhouettes, projectile rows, fades, and handoff to
   tri-vid. No unrelated shell is present to preserve; color-zero sprite
   pixels must still remain transparent.
7. **Frame support:** the four listed source frames show more than one actor
   and story state; fresh native samples 688, 849, 1030, 1050, 1070, and
   1090 show reveal, text, projectiles, and effects. They support the broad
   composition but not exact source-to-host timing or all sprite phases.

### Applied review: tri-vid and airport resource handoffs

1. **Appearance:** source section 02 frames 000820/000830/000840 show black,
   dim reveal, and the cockpit; 000900/000940/000960 show stable and fade
   phases. Native startup QA snapshots 1346, 1382, 1390, 1416, and 1693 show
   the wilderness fade, tri-vid reveal/hold, and fade before airport install.
   The former “unrecognizable cockpit” verdict was based on a bad crop and is
   withdrawn.
2. **Behavioral state:** typed stages distinguish fade-out, resource install,
   fade-in, stable loop, and the next fade-out. The two 0x131C/0x12F6 palette
   routines are 34 VBlank states each; integration QA checks all 34 snapshots
   for five transition profiles.
3. **Visual construction:** the cockpit uses its extracted plate and separate
   sprite layers. The airport plane uses its extracted transparent vehicle
   plate; fresh source/native anchors 1014/1725 and 1034/1765 show matching
   plane bounds after accounting for capture cadence. Do not reuse the old
   “aircraft slivers” statement: it was not evidence about the plane. Rotor
   and helicopter choreography still need an independent layer-by-layer
   comparison.
4. **Animation:** tri-vid actors, airport scroll, pulse, landing, and fades
   have separate typed counters. The fade advances one palette step every
   two VBlanks for 34 states. Seattle/tri-vid/airport boundary captures show
   entry and exit, but exact longplay-to-native cadence for moving actors
   and rotor remains unresolved.
5. **Sound:** tri-vid event 0x12 is emitted at 0x59CB8 and maps to
   audio.intro.title.stage-3 (21.0048 seconds). The airport callback at
   0x5AD1A has no direct audio call. No rotor sound is assigned without a
   traced event; playback lifetime for the stage track beyond the extracted
   file is not established by the screenshot.
6. **Related screen parts:** reviewed plates, transparency, sprite order,
   scroll, story/landing text, blank center rows, palette handoffs, and the
   next scene. The blank-row callback at local index 0xD7 now clears center
   text; the six-line landing paragraph is drawn at elapsed index 0x348.
   Native frame 2566 matches source frame 001440's paragraph.
7. **Frame support:** multiple source and native frames support the cockpit
   reveal/hold/fade and plane travel. The landing-text pair and plane bounds
   support those subpaths. They do not settle helicopter/rotor timing or all
   later scene behavior, so both intro ledger rows stay partial.

### Applied review: cab map and destination cursor

1. **Appearance:** source frame 001800 shows the Redmond Barrens caption,
   map, and selected outline. Qt startup-QA frame 3146 now uses a white
   caption and the ROM Plane-A position. NativePort CLI frames 0/30/60 use
   the extracted cursor tiles; frame 60 is saved at
   `decompilation/build-qt-debug/cab-map-tile-verified/cab-map-60.bmp`.
   The prior cursor offset of +30,-2 was wrong
   and has been removed. The visible map extent still differs from the source
   and remains open.
2. **Behavioral state:** post_intro_mode, post_intro_phase, and
   post_intro_caption_visible drive the view. ROM 0x134D2 selects mode 2;
   0x13726 maps through 0xD445D/0xD4464 and builds the border from 0xD4446.
   Startup QA checks a rendered top-left cursor tile at that plan's Plane-A
   coordinates and white ink in the caption. The NativePort regression checks
   the first and final destination tile at the exact ROM-derived positions.
3. **Visual construction:** the cursor is an eight-tile overlay. Its table
   position is already in video coordinates; the separate 26x17 map plate
   must not shift it. Caption text is written through 0xC0DA at 0xEA10.
   The 208x136 extracted plate is drawn at (46,14); its current visible extent
   does not match the supplied capture. Background ownership and viewport
   placement need a full source trace before changing the plate.
4. **Animation:** the cursor outline shrinks through five phases at two
   VBlanks per phase, followed by mode-4/mode-2 holds and the field handoff.
   Source frame 001800 shows the selected state; frame 001810 is black during
   handoff. Native frame 3146 shows the final selected state and frame 3150
   is black. The sparse capture does not establish exact wall-clock timing.
5. **Sound:** the caption path writes text and has no separate sound call.
   No selection or handoff cue is assigned without a traced event and
   identified clip.
6. **Related screen parts:** checked the caption, map, cursor, black
   transition, and field handoff together. Cursor/map layer placement is now
   separated, but map extent and any additional background plane remain
   unresolved.
7. **Frame support:** source frames 001800 and 001810/001820 show the
   selected map followed by black; Qt frames 3146 and 3150 show the same
   state progression. NativePort frame 60 shows the final cursor tile at the
   traced Redmond coordinate. This supports cursor placement and exit
   classification, not the map's full geometry or exact cadence.

## Required audit passes

Work through these passes in order. A later pass cannot hide a failure in an
earlier one.

### 1. Resource and route inventory

For each screen or shared mechanism, identify the direct assets, typed source
records, aliases, and native entry point. Reject a missing resource as a
conversion gap first; do not invent a replacement routine before rechecking
the ROM/Ghidra call path. Keep extraction sources and runtime source files
separate.

### 2. Shared shell and layer pass

Before judging text or icons, verify the frame's ownership and order:

- opaque device/menu/HUD shell survives every overlay;
- map/playfield remains visible only where the source leaves it visible;
- color-zero sprite pixels are transparent only for sprite families that the
  source writes transparently;
- plane-sized extracted overlays do not use their black canvas to erase an
  already-installed host layer;
- field HUD chrome remains opaque while the playfield remains underneath it;
- nested rows replace the correct source row instead of being drawn over a
  five-button row with a four-button row;
- vblank/clear/fade operations affect the intended surface and not the
  persistent bezel or the next scene.

The executable QA paths enforce representative shell invariants for startup,
normal field HUD, and command menus. Those invariants are deliberately small
layer checks, not a self-generated full-frame golden.

### 3. State-space pass

For every interactable, exercise the complete state sequence, recording the
typed state before and after each input:

```text
initial -> select each entry -> move across every boundary -> reselect
        -> unselect/cancel -> reopen -> confirm/deny -> return to parent
```

Repeat this for each source archetype where the records differ. For menus,
include empty and occupied cells, equipped and unequipped items, unavailable
actions, posture/status variants, and all nested submenus. A default Samurai
record is not evidence for Decker or Gator/Shaman.

### 4. Math and projection pass

For every displayed value, change or use the typed input that drives it and
check both the source-derived result and its visual projection. This includes
attack/defense/status bars, ammo rounds and clips, equipment borders, item
icons, cursor position, health/mental values, damage/fade weights, and
quantity/transfer arithmetic. Test boundaries and rejected operations, not
only a normal successful action.

### 5. Timing and transition pass

Capture the first frame, stable frame, transition frame, and return frame for
each animated route. Check source counters, sprite entry/exit, palette/fade
behavior, vblank clearing, scroll/camera placement, and the destination state.
Intro scenes are composed layers and choreographed sprites; the FFmpeg dump is
an analysis reference only and must never become a runtime bitmap flipbook.

### 6. Independent visual review

Inspect the native artifact at source size against the corresponding longplay
frames. Review the whole frame, not just the changed rectangle. Record:

- screen identity and visible choice;
- shell and boundary geometry;
- text baseline and clipping;
- sprite count, source-local bounds, alpha, and draw order;
- animation/fade phase and persistence across frames;
- camera/map/HUD relationship;
- state-specific math and selected/unselected treatment.

If the longplay is ambiguous, mark the item `needs_independent_capture`; do
not promote it to `defended` from an internal compositor.

## Status rules

The ledger in `qt-visual-audit.tsv` is the source of truth for this process.

- `pending`: ROM entry or route is known, but native reconstruction or review
  has not been completed.
- `partial`: one subpath is native, but the shared mechanism or state matrix
  is incomplete.
- `failed`: the independent review or an invariant exposed a mismatch.
- `needs_independent_capture`: source behavior is understood, but the supplied
  longplay does not establish the disputed detail.
- `defended`: ROM entry/cursor/draw/exit trace, typed native state, regression
  test, CLI image dump, and independent visual review all exist.

Only `defended` items may be used as evidence that a larger architecture hill
is complete. Updating a status requires adding the artifact command and the
reason for the decision in the ledger. Do not delete a failed observation;
resolve it with a new artifact and a new review entry.

## Current audit commands

These commands write verification artifacts outside `assets/`:

```text
shadowrun_visual_dump.exe <asset-dir> <out-dir> --scene <scene> --frames 0,90,128,500
shadowrun_ghidra_conversion_qt.exe --startup-qa <audit-dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-dump <file> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-hud-qa <audit-dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-command-dump <file> --field-archetype <samurai|decker|gator> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-action-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-clips-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-dialogue-dump <prompt> <file> --assets <assets>
```

The `--field-archetype` switch selects an existing startup record; it does
not create a second game state machine. It exists so visual review can prove
that the command/HUD projection follows the selected source record.
`--field-hud-qa` renders all three archetypes, verifies each frame against
its own extracted face family and HUD chrome, and fails if two archetypes
produce identical frames.

## What this prevents

This method prevents a narrow fix from being called complete when the actual
problem is shared: an opaque overlay erasing a shell, a menu tested only once,
a four-item submenu drawn over five parent buttons, a Decker/Gator record
silently rendered as Samurai, a sprite family merged into a background, a
fade affecting the wrong layer, or a runtime path that has no source-derived
record to explain it. Such a finding goes back to conversion analysis before
new behavior is invented.

### Applied review: shared field dialogue panel

1. **Does it look correct?** The Debug CLI CTest writes the manager prompt to
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-dialogue-qa/manager-package/actual.bmp`.
   The extracted standard frame now matches the metallic border and right rail
   in independent section 03 frames 000300, 000600, 001200, 001500, and 001800.
   The manager paragraphs begin at x=88 on rows 16, 32, and 56. The Qt view
   uses the same origin, preserves blank paragraph rows, and displays the
   source A/B/C marker sprites and lower option rows.
2. **Does the look reflect the actual behavioral state?** A conversation
   source resolves the speaker, triggering event, prompt/page, text, portrait,
   choices, and frame variant into `DialoguePanelView`. `DialoguePanelPresenter`
   consumes that view without choosing Stoker content. The Stoker controller
   binds the profile-0 portrait and `dialogue.stoker.motel` text for the
   `manager` and `manager-package` states. The synthetic second-speaker check
   supplies its own text and no portrait, so the presenter does not borrow
   Stoker's image. Live field-property/event selection and additional speaker
   providers remain unconnected.
3. **Are the visuals themselves correct?** ROM `0x10984` installs nine shared
   frame regions using patterns loaded at `0x10918`; its tenth `0xCEFF0` block
   replaces the left panel when `FFF0AF` is set. The offline importer exports
   both 320x224 frame variants. ROM `0x1F5A8` clears the body at `(88,16)` and
   the lower panel at `(16,152)` before `0x1F3C4` draws content. That prompt
   path uses `0xC168` with a 27-column body and sends its 2x2 A/B/C markers
   from `0xCF11C`, `0xCF124`, and `0xCF12C` through `0x1232`. These assets
   replace the former hand-drawn bevel and square markers. The `FFF0AF` frame
   choice is represented in typed view state, but its live source binding is
   not implemented.
4. **Is it animated, and should it be?** The prompt redraws on entry or page
   change, waits for VBlank, and samples input; it has no dialogue animation
   counter. Independent frames 000300-000302 and 001800-001802 are pixel
   identical holds. The frame and portrait therefore stay static in the
   observed pages; other event scenes have not been checked.
5. **Does sound accompany it?** The traced `0x1F246`/`0x1F3C4` draw and wait
   path and prompt callbacks `0x213CA`, `0x213E2`, and `0x21458` contain no
   direct sound-play call. The optional `A1` hook in `0x1F246` is indirect and
   unresolved. The background soundtrack has not been measured as a dialogue
   effect, so no dialogue sound ID or clip duration is assigned.
6. **Are related parts of the screen also correct?** The review covers the
   frame, left speaker well, portrait alias, paragraph rows, exact source
   button sprites, option labels, centered `< MORE >`, page/return state, and
   full lower panel. Manager choices, debt choices, and a no-choice
   continuation page use the same panel. Debt payment, room payment, random
   arrival, and a live building entry are not yet connected end to end.
7. **Do multiple frames support the interpretation?** Section 03 frame 000300
   shows manager choices, 000600 shows the debt question, 001200 shows an
   information page with another portrait, 001500 shows another three-choice
   page, and 001800 shows `< MORE >`. The shared frame persists while the
   speaker and source-provided contents change. Adjacent 000300-000302 and
   001800-001802 captures are pixel-identical holds. These examples do not
   prove every speaker or event uses the same transition graph.
8. **Was the previous screen cleared before the new one was drawn?** Each open
   page clears the full 320x224 canvas, redraws the frame, then clears both
   source content wells before drawing new text and markers. The Debug and
   Release tests paint over a magenta sentinel and confirm it is gone; they
   also verify the source A marker asset, no Stoker portrait for the synthetic
   speaker, and removal of the third option and marker on the next page. CLI
   captures are standalone views, so the live field-to-dialog-to-field
   restoration remains unverified.

**Status:** partial. The shared frame, conditional frame asset, source option
markers, body/choice placement, blank rows, and observed static timing are
source traced and checked against multiple independent gameplay frames. Debug
and Release tests cover rendering and the manager-to-debt-to-referral path.
Live speaker/event lookup, `FFF0AF` frame selection, additional speaker data,
remaining payment results, live field restoration, and measured dialogue
audio remain open.
