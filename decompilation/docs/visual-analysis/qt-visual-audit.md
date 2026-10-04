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

## Eight-question anomaly gate

Run this gate before changing any screen that looks suspicious, then answer it
again against the corrected native dump. Keep the answers with the ledger row
or its linked review note. `unknown` is an acceptable result when the supplied
evidence cannot settle a question; it is not a pass.

1. **Does it look correct?** Identify the suspected region and describe the
   visible mismatch in plain terms. Compare the whole source-size frame and
   record the reference and native artifact paths.
2. **Does the look reflect the actual behavioral state?** Name the typed state
   fields and ROM records/tables that drive it, including the speaker or
   object's properties and the event/context that opened the screen. For
   dialogue, verify the selected speaker, text/page, options, and return path
   come from those properties and that trigger. For selection states, check
   every source-visible attribute (icon, cursor, tint, border, label, and
   value as applicable). Record the input and relevant values before and
   after the transition.
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
8. **Was the previous screen or modal cleared before the new one was drawn?**
   Trace whether the ROM clears or redraws the panel, or intentionally keeps a
   shared interface. Check that old buttons, labels, cursors, icons, and other
   graphics are absent wherever the new screen replaces them. A shared shell
   may remain only when the source path preserves it; a new control must never
   sit over an old control by accident. Pass this check only when each
   replaced control is explicitly hidden/removed or its owning panel is fully
   redrawn before the new control is painted; verify the old control's former
   bounds in the new screen and on return from the child/modal.

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
   `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/field-inventory-selector-qa.bmp`;
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
3. **Visual construction:** `0x11592` yields host SAT origin
   `(primary * 40 + 12, secondary * 24 + 116)` for a 32x24 cursor footprint.
   The 24x16 item image starts four pixels inside that footprint, at
   `(primary * 40 + 16, secondary * 24 + 120)`. The independent capture shows
   a one-pixel warm-gold outline hugging the item image itself, with no extra
   inset. Its repeated medkit edge in frames 002580, 002610, and 002640 samples
   as `#C4BE27`; Qt and NativePort now draw that geometry from typed selected-
   cell state instead of the extracted sprite's oversized internal segments.
   ROM `0x11624` installs equipped-cell treatment before the item glyph rows;
   the renderer order keeps the item art visible above that treatment. Fresh
   Qt selector captures are in
   `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/`.
4. **Animation:** the observed focus is stationary while held. It moves when
   the selector changes and remains through the Heal-who prompt and use
   result. It is still visible in section 07 frame 000030; the field view at
   frame 000120 has closed the menu. Frames 002580, 002610, 002640, and
   002699 show no blink or per-frame animation. ROM `0x11592` rebuilds the
   sprite record on a selector refresh; this path has no separate cursor
   animation counter.
5. **Sound:** D-pad movement reaches the command detail refresh, which emits
   audio event `0x25`. ROM `0x530DE` sends that byte to the audio command
   queue; it selects table-2 index 37 (`audio.effects.resource.037`, 2.134
   seconds including its release tail). Qt maps and forwards the event to
   `QSoundEffect`. Independent longplay audio around the Ares shop has repeated
   waveform matches for this sample; the Qt tests verify the alias route, not
   speaker output for this specific First Aid visit. Action-button dispatch
   uses `0x24`, mapped to `audio.menu.command-024` (2.484 seconds including
   its two-second release tail).
6. **Related screen parts:** inspected the item art, equipped-cell frame,
   center detail panel, field HUD, command shell, and the nested action row.
   Qt page-1 QA checks all four outline edges, rejects matching-color pixels
   inside, and requires populated item ink. NativePort's focused test checks
   the same geometry for the occupied Scatter cell before its later field
   movement assertion. Field-action QA checks that the Qt outline remains
   around its item when Equip opens. The
   longplay shows it around the medkit through Heal-who/use. The Qt First Aid
   dump now checks the same outline through target selection and result and
   confirms it is removed on return.
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
listed longplay frames. The eight-cell equipment/empty-slot matrix remains
open.

### Applied review: First Aid target, result, and return

1. **Does it look correct?** Inspected the whole Qt 320x224 output at
   `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/field-first-aid-qa/actual.bmp`
   and the named target, CANCEL, result, and return checkpoints beside it.
   Section 06 frame 002610 shows the target prompt; 002640 shows
   `Medkit used.`; 002699 shows the returned Inventory page. The selected
   item has the narrow gold edge in target/result and no edge after return.
   Full-frame art is not a same-record match: the executable fixture uses the
   Samurai starter record because it supplies a Medkit, while these longplay
   frames show Joshua's Gator Shaman record after a Medkit was acquired.
2. **Does the look reflect the behavioral state?** ROM `0x12298` reads the
   three active-runner flags at `FF0100/FF0200/FF0300 + 0x10`, their names at
   `+0x58`, and stores the selected record pointers. The typed
   `CommandOverviewFirstAidState`, target list/cursor, selected inventory slot,
   `RunnerRecordFields` meters/flags/phase, and continuation drive the Qt
   screen. QA reaches Medkit through the inventory action row, verifies the
   single active Joshua target, rejects B/C, confirms A, then checks the
   meter, item phase, flags, continuation, result wait, and return.
3. **Are the visuals correct?** The prompt and rows are placed from source
   name-table destinations: `C5A0` for `Heal who??`, `C6A0` for the first
   target name, `+0x80` per following active record, and `+0x86` before
   `CANCEL`; the native coordinates are x/y `(128,88)`, `(128,104)`, and
   `(152,112)` for the one-target case. ROM strings at `D1EEA`, `D1F03`,
   `D1F0A`, `D1F1B`, and `D1F28` supply the prompt/cancel/result text. The
   centered target face, shell, HUD, item detail, and 24x16 `#C4BE27` selector
   remain visible. The test checks the source shell/HUD, extracted face and
   meter assets, text regions, and selector edges. The same-archetype full
   portrait comparison remains open because the longplay and QA use different
   runner records.
4. **Is it animated?** `0x1235C` waits for VBlank and samples input; this
   loop redraws on selection changes but has no First Aid cursor timer or
   animation counter. The Qt target frame is pixel-identical after 30 idle
   frames. Longplay frames 002630-002635 hold the target/list layout; the
   four-pixel change from 002630 to 002631 is not enough evidence to add an
   animation. D-pad movement changes the selected row; result text holds
   until the `0x0D94` event wait returns.
5. **Does sound accompany it?** Moving the target cursor requests ROM audio
   event `0x25` at `0x12392`; confirming/cancelling requests `0x24` at
   `0x12412`. Event `0x25` maps through the shared playback service to
   `audio.effects.resource.037` (2.134 seconds including its release tail);
   `0x24` maps to `audio.menu.command-024` (2.484 seconds). The Qt callback
   trace and focused audio CTest verify the source event and alias route, but
   neither captures Windows speaker output. No separate result sound is
   issued, and this longplay visit does not independently isolate its cursor
   cue.
6. **Are related parts correct?** Reviewed the inventory icons and item
   detail, selected/equipped layers, shell, right HUD and live meter, center
   target face, result line, and return to Inventory. QA checks the full
   screen shell/HUD on every checkpoint, confirms the live HUD meter follows
   the changed runner record, verifies Cancel leaves the item and meters
   alone, and checks that the selector is restored only when another
   direction is pressed after return.
7. **Do multiple frames support this?** Independent frames 002610 and
   002630 show the target list on separate visits; 002611 and 002640 show the
   used result; 002615 and 002699 show return. These support the repeated row
   layout, selection/result transition, selector persistence/removal, and
   static hold. They do not establish the Samurai portrait or capture Qt's
   acoustic output for event `0x25`.

The focused CTests are
`shadowrun_ghidra_conversion_qt_field_first_aid_qa`,
`shadowrun_ghidra_first_aid_conversion`,
`shadowrun_ghidra_field_party_roster_runtime`, and
`shadowrun_ghidra_field_menu_text_catalog`. The CLI command is
`shadowrun_ghidra_conversion_qt.exe --assets decompilation/assets --field-archetype samurai --field-first-aid-dump <output-dir>`;
it writes `actual.bmp` plus the interaction checkpoints and verifies that
held A does not dismiss the result before a fresh controller edge. This hill stays
`partial`: Joshua-only roster integration, host RNG seeding, measured Qt
speaker output, and same-archetype visual support still need work.

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
   at 0xC2A6; page-5 A now opens the Magical Defense fallback from 0xFBA6.
3. **Visual construction:** pages 0/1 retain the two-column eight-cell
   inventory and selected-item detail; page 2 opens the Clips action panel;
   page 3 replaces the base frame and renders the 20 labels from 0xC1A2A;
   pages 4 and 6 open their own compositions on A. The page-3 values and
   0xD27C cursor are not rendered. Page 5's spell-capable panel is not
   connected; its static fallback has a separate applied review below.
4. **Animation:** the tab highlight stays still until a D-pad edge changes
   primary_page; it is not a blinking animation. Inventory selection changes
   secondary_page by input. The separate Attributes/Skills cursor navigation
   and Magic spell cursor cadence are unimplemented and remain unclassified.
5. **Sound:** A dispatch uses event 0x24, extracted as
   audio.menu.command-024 with a 2.484-second file including its two-second
   release tail. Selector movement uses event 0x25, mapped to table-2 entry 37
   (`audio.effects.resource.037`, 2.134 seconds including its release tail).
   A longplay audio cross-check matches that sample during the Ares shop;
   speaker output from the Qt build remains unmeasured.
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

### Applied review: Magical Defense fallback

1. **Appearance:** before the change, page-5 A had no Qt subview. The ROM
   branch writes a `Magical Defense` heading over the command/status page and
   two numeric strips. Inspected `magic_tab.bmp`, `actual.bmp`, `held.bmp`,
   and `return.bmp` under
   `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/field-magic-qa/`.
   The current 320x224 output follows the traced overlay positions; its match
   to an actual Genesis Magic screen remains unknown.
2. **Behavioral state:** page-5 A now dispatches into typed
   `CommandOverviewMagicState::magical_defense_wait`. ROM `0xFBA6` chooses the
   spell panel only when `+0x9B < 2` and `+0x7F != 0`; otherwise it chooses
   this fallback. The QA fixture uses the Gator field composition and sets
   typed `+0x7F` to zero to exercise the class-1 fallback. Both plans use the
   three party records, the `+0x10` active flag, class, item-0x18 skill, and
   separate active-runner attributes `+0x77`/`+0x7C`. A new input edge exits;
   the runner's inventory, equipment, force, posture, and ammo stay unchanged.
3. **Visual construction:** ROM `0xFBA6` sends `C89C0` to `C31C` at
   `(112,48)`, then calls `0x106DA` with `D2=0` and `D2=-1` for the physical
   and mana strips. The plans target `C51E`/`C69E`, use 13 tiles at stride
   `0x1A`, and select 104x8 source rows from `0xCF610` at `(120,80)` and
   `(120,104)`. The Qt capture matches the named strip assets at those spots
   and retains the page shell, selected tab, and right HUD. This is a same-
   source placement check, not proof of Genesis visual fidelity.
4. **Animation:** the fallback draws once, then calls `0x0D94` to service
   VBlank/input until a high-byte controller event and take its final sample.
   It does not enter the spell cursor/sprite loop or advance a Magic timer.
   Thirty empty Qt frames are pixel-identical; a fresh direction edge returns
   through `0x11928` to page 5. The supplied longplay has no Magic visit, so
   source cadence still needs an independent capture.
5. **Sound:** the parent command loop at `0x10E4E` sends audio event `0x24`
   before A/C dispatch. Qt now forwards that event to
   `audio.menu.command-024`; the extracted clip is 2.484 seconds including
   its two-second release tail. The fallback itself and its `0x0D94` wait
   issue no additional sound. The CTest checks event delivery, not audible
   playback on a speaker.
6. **Related screen parts:** the full screenshot checks the command shell,
   selected tab, page content, archetype face, and persistent right HUD. The
   return image equals the same page-5 image and record fields are unchanged.
   The other `0xFBA6` branch remains open: it clears/loads a separate spell
   panel and needs spell cursor visuals, force selection, target/effect, and
   result/return behavior.
7. **Frame support:** the CLI supplies before/open/held/return snapshots, but
   these are Qt captures. Sections 01-07 of the independent longplay do not
   show this Magic panel. Keep the route `partial` and obtain multiple real-
   game Magic frames before making a visual-correctness claim.

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
shadowrun_ghidra_conversion_qt.exe --field-first-aid-dump <dir> --field-archetype samurai --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-automatic-use-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-inventory-trade-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-clips-dump <dir> --assets <assets>
shadowrun_ghidra_conversion_qt.exe --field-magic-dump <dir> --field-archetype gator --assets <assets>
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

### Applied review: Automatic-use acknowledgement

1. **Does it look correct?** Inspected the full 320x224 Qt output in
   `build-qt-magic-qa/qt_runtime/field-automatic-use-qa/actual.bmp`. The green
   three-line message sits in the inventory detail panel while the item icon,
   right HUD, status strip, and thin item outline remain visible.
2. **Does the current look reflect the behavioral state?** ROM `0x12168` routes
   codes `0x13`, `0x14`, `0x16`, and `0x18..0x2B` to `0x121C0`. The typed Qt
   state is `CommandOverviewAutomaticUseState::result_wait`; the QA dispatches
   a Mana Blast item (`0x20`) from the real item/action loop and verifies its
   item code, phase, equipped slot/code, and armor index are unchanged. The
   selected outline stays through the wait and clears after return.
3. **Are the visuals correct?** `0x121C0` writes the D202F text at C620, which
   maps to `(128,96)`. The ROM's `0x80` separators decode to `This item is`,
   `automatically`, and `    used.`. Those lines come from the offline ROM
   import and render through the direct field font. Position and source text
   are traced, but the exact scene lacks an independent capture.
4. **Is it animated, and when?** The branch prepares the panel, renders text,
   then enters `0x0D94`; no cursor timer or message animation appears in this
   path. The Qt dump is pixel-identical after 30 idle frames and while the
   dispatch A is held or released.
5. **Is there a sound?** The parent action loop at `0x11C3C` requests event
   `0x24` before dispatch; `0x121C0` and the `0x0D94` acknowledgement add no
   separate request. Event `0x24` maps to the existing 2.484-second clip,
   including its release tail. The QA checks callback requests, not acoustic
   output from a speaker.
6. **Are nearby screen parts correct too?** QA compares the item name/detail
   region before and during the message, checks the one-pixel item outline,
   and verifies the command shell and HUD assets. It also checks no equipment
   or inventory mutation occurs and that the outline is gone after return.
7. **Do multiple frames support the interpretation?** The Qt dump includes
   message, 30-frame idle, held, released, and return images. Independent
   longplay frames 002610/002611/002615 and 002630/002640/002699 support the
   neighboring inventory/First Aid panel placement, selector persistence, and
   return. None shows the exact D202F message, so the visual status remains
   `partial` until a matching independent frame is available.

### Applied review: Trade recipient and transfer

1. **Does it look correct?** Inspected the complete 320x224 frames
   `field-inventory-trade-qa/trade_recipient_full.bmp`,
   `trade_recipient_dog_no.bmp`, `trade_recipient_dog_yes.bmp`, and
   `trade_transferred.bmp`. The action background, choice bar, item detail,
   and one-pixel yellow-gold outline remain visible. The recipient line and
   full-inventory message use the traced source text positions. The screen is
   still only partially reviewed visually because no supplied independent
   Trade frame is available.
2. **Does the look reflect the behavioral state?** ROM `0x12836` rejects
   `0x17..0x2B`; the eligible path at `0x1288E` scans the three-record ring
   for another active runner, and `0x1296A` starts each candidate on No.
   Typed state is `InventoryTradeState` plus the live
   `FieldPartyRosterRuntime` records. The Qt QA reaches no-recipient, full
   recipient, next active recipient, Yes/No, and transferred states; it
   verifies item code/phase and source equipment cleanup. These diagnostic
   records are test fixtures, not normal-game party members.
3. **Are the visuals correct?** ROM writes the full-recipient name/message at
   `C69E/C71E` and the eligible prompt/name at `C7A6/C820`; the Qt positions
   follow those plane addresses. The selected-item edge matches the
   independent inventory captures: a single warm-gold pixel around the item
   art. The source also calls `0x110E8` at `C928` to draw the recipient's HUD
   panel. Qt currently leaves the active runner's panel in place, so this
   source-visible recipient projection is a known missing visual detail.
4. **Is it animated, and when?** `0x1296A` waits for VBlank between input
   samples. The source redraws the Yes/No row after Left/Right; it does not
   animate the focus during idle. The Qt QA checks the rendered image after
   30 idle frames and captures No, Yes, next-recipient, and return states.
5. **Does sound accompany it?** `0x12988` requests event `0x25` after either
   horizontal edge; `0x129B2` requests `0x24` on confirm. Event `0x25` maps to
   `audio.effects.resource.037` (2.134 seconds including its release tail),
   and `0x24` maps to `audio.menu.command-024` (2.484 seconds). The source
   sample is cross-checked against the independent Ares longplay audio; this
   Trade QA checks the event and alias path but does not capture speaker output.
6. **Are related parts of the screen correct too?** The whole frame was
   checked for the inventory item, item detail, command shell, active runner
   HUD, Yes/No focus, and return to the inventory grid. The item remains at
   its source slot until transfer; transfer clears it and updates the source
   inventory view. The recipient HUD call at `0x110E8` is the related panel
   still missing from Qt.
7. **Do multiple frames support the interpretation?** Native checkpoints
   cover a full recipient, the next recipient on No, that recipient on Yes,
   transfer, and the one-runner no-op. Independent section 06 inventory
   frames 002550-002699 support the shared shell and selector shape, but none
   shows Trade or its recipient panel. The Trade visual status remains
   `partial`; independent reference capture and the `0x110E8` projection are
   outstanding.

### Applied review: Pocket Secretary Group Items

1. **Does it look correct?** Inspected `actual.bmp`,
   `group-items-populated.bmp`, `group-items-empty.bmp`, both 30-frame held
   captures, both return frames, and the adjacent Cyberdeck no-deck/return
   frames under `build-qt-magic-qa/qt_runtime/field-secretary-qa/`. The outer
   Pocket Secretary frame remains around the black center. The child removes
   the parent title/options and shows only its current source rows. This is a
   same-source visual review; no independent Pocket Secretary frame is in the
   supplied longplay sections. The parent cursor tile is source-mapped to the
   lowercase `x` at x=136; its appearance against each row still needs an
   independent reference.
2. **Does the look reflect the behavioral state?** The Group Items row is
   selection 1. The typed `PocketSecretaryMenuState::group_items_wait` holds
   the modal state, and `group_item_flags` selects either the empty message or
   a descending-bit item list. Fresh New Game supplies FFFC22=0. The populated
   QA frame uses diagnostic bits 14 and 1 to prove order (`Ork Armband`, then
   `Indian Lands Visa`); normal quest acquisition and restored-save FFFC22
   are not connected. Any controller event dismisses the child and preserves
   the parent selection at row 1.
3. **Are the visuals themselves correct?** ROM `0xC8C8` calls `0xC3E6`,
   reads FFFC22, and on the zero branch passes C1471 to C0DA at C50E. The
   extracted strings remain separate: `You are not carrying` and
   `  any group items.`. C50E maps to host (56,80), with the second source row
   at y=88. The populated path reads the name table at C25C4, begins at C208
   (host x=32,y=32), and advances A2 by `0x100` bytes (16 host pixels) per
   visible entry. C3E6's 28x21 repeated grid is now an offline named asset:
   its first tile word comes from C1320 and selects the space glyph in the
   C1064 font stream uploaded at B560. Qt draws that decoded 224x168 layer
   inside the extracted frame, then draws the child text; the parent is not
   redrawn over the cleared panel. The source placement is implemented, but
   the output has no independent Genesis comparison.
4. **Is it animated, and should it be?** The child writes once and jumps to
   `0xF312` to wait for any controller event. That shared wait calls `0xE660`
   once after each VBlank, so the complete source path does animate a random
   two-cell grid effect while the child is waiting. `0xE660` chooses a
   destination and tile pair from its random tables; its `0x1232` call uses
   D1=0 and D7=1, which writes one row by two adjacent cells. The typed plan
   now records width 2 and height 1. Qt loads seven named source-decoded
   patterns and advances this effect once per frame. Each written pair stays
   in its source-mapped lower-bezel destination until the panel is rebuilt.
   Held-frame QA confirms the pair changes while child text and controls stay
   stable. Independent Genesis frames are still needed to verify visual
   cadence.
5. **Is there a sound?** `0xC8C8` contains no sound request, so the child adds
   none. The parent command overview requests event `0x24` when A opens the
   Pocket Secretary; its extracted `audio.menu.command-024` clip lasts 2.484
   seconds including the release tail. The child wait does not request a new
   sound, and the QA records callback/state behavior rather than speaker
   output. `0xE660` contains random sampling and a name-table write, but no
   sound request; the source wait adds no separate sound.
6. **Are nearby screen parts correct too?** Compared the parent menu at
   selection 1, populated and empty children, both returns, and the neighboring
   Cyberdeck no-deck child/return. The frame and black center persist, parent
   text clears during each child, and the same selected row returns. The
   source random-grid destinations lie in the bottom bezel. Held captures
   confirm the lamps do not cover child text or selectable rows. Notebook detail
   handlers, Save/Load, deck-present Matrix, and restored-save flags remain
   explicit Qt boundaries; Dismiss E660 art and candidate HUD projection are
   documented in its applied review below.
 7. **Do multiple frames support the interpretation?** Native checkpoints
    capture entry, each Group Items branch, 30 held frames, return, and the
    Cyberdeck branch at its own wait/return. They show the lower-bezel effect
    changing while child content remains stable. The supplied independent longplay contains
   no Pocket Secretary example, so visual fidelity remains `partial` until
   several matching Genesis frames are available.
8. **Was the previous screen or modal cleared before the new one was drawn?**
   `0xC8C8` calls `0xC3E6` before writing either child branch. The Qt painter
   redraws the frame and blank inner grid before child text, and the captures
   show no Pocket Secretary title/options beneath Group Items or Cyberdeck
    feedback. The two-cell effect stays within the lower bezel and does not
    overlap child controls. Both panel replacement and overlay bounds are checked.

### Applied review: Notebook and Current Run no-run response

 1. **Does it look correct?** The corrected Qt captures are in
    `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/field-secretary-qa/`:
   `notebook-menu.bmp`, `notebook-selection-1.bmp`,
   `notebook-current-run-unavailable.bmp`, `notebook-current-run-held.bmp`,
   `notebook-current-run-return.bmp`, and `notebook-return.bmp`. They establish
   the present renderer's full entry, selection, response, wait, and return
   sequence; no matching independent Pocket Secretary frame is in the supplied
   longplay.
2. **Does the look reflect the behavioral state?** The typed
   `NotebookMenuState` starts at Current Run, includes Current Run and Tips &
   Clues, and includes Contacts because fresh New Game sets `FFFC2C` at
   `0x59C48`. With no active run, A enters the three-line `0xF2E2` response;
   it waits for a controller edge, rebuilds Notebook at selection zero, and
   B returns to the selected Pocket Secretary row.
3. **Are the visuals themselves correct?** Raw US ROM bytes show `0xF33E`
   clearing through `0xC3E6`, writing the encoded labels from `0xC567E` at
   `0xC216`, and writing the separator at `0xC292`. `0xC0DA` advances the
   name-table destination by `0x80` bytes per source row marker. This places
   the title at `(88,32)`, Current Run at `(88,56)`, Tips & Clues at
   `(88,72)`, and gated rows from `(88,88)` with 16-pixel spacing. The cursor
   base `0xC392` is `(72,56)`. `0xF2F6` clears again and writes the no-run
   response from `0xC56E7` at `0xC30C`, or `(48,48)`, with 8-pixel line
   spacing. The Qt layout now follows those placements. An independent
   Genesis image is still required to call the visual fidelity verified.
4. **Is it animated, and should it be?** The `0xF236` menu loop calls
   `0xE660` once per VBlank. The Current Run response enters `0xF312`, which
    also advances `0xE660` once per VBlank. The source writes one row of two
    random bottom-chrome cells while waiting. Qt draws seven decoded patterns
    into the twelve traced lower-bezel destinations, retaining earlier writes
    until the panel is rebuilt. Thirty held frames verify that this region
    changes while Notebook and response text remain fixed.
5. **Does sound accompany it?** The Notebook setup, no-run handler, and wait
   contain no sound request. The parent command overview plays event `0x24`
   when A opens Pocket Secretary; the extracted clip lasts 2.484 seconds
   including its release tail. The Notebook response does not request a new
   sound.
6. **Are related parts of the screen also correct?** The review checks the
   Pocket Secretary parent, Notebook selection, no-run child, held response,
    rebuilt Notebook, and parent return. The no-run child redraws the same
    frame and blank grid used by other children. The shared `0xE660` effect
    changes only the lower bezel during the wait, with text and selectors stable.
7. **Do multiple frames support the interpretation?** Native captures cover
   the initial row, moved selection, A response, 30 held frames, response
   dismissal, and B return. They are not independent Genesis evidence. The
   supplied longplay has no Notebook example, so repeated real-game visits
   and cadence remain `needs_independent_capture`.
8. **Was the previous screen or modal cleared before the new one was drawn?**
   Both `0xF33E` and `0xF2F6` call `0xC3E6` before drawing the replacement
   panel contents. Qt paints the shared outer frame and blank inner grid,
   then draws only the active Notebook or response state. QA checks that
   the old title and lower gated rows disappear on the response, and that
    response-only text disappears when Notebook is rebuilt. The outer frame
    is the intentionally shared interface; old buttons and labels are not
    retained beneath the new content. The animated lower bezel stays within
    its twelve source positions and does not cover either page's text.

### Applied review: Notebook Tips & Clues

1. **Does it look correct?** I inspected the Qt captures under
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-secretary-qa/notebook-tips/`:
   `actual.bmp`, `page-2.bmp`, `held-30-frames.bmp`, and all three return
   frames. The labels, first-page text, page counter, and cleared Notebook
   return are internally consistent with the ROM trace. The supplied longplay
   contains no Tips visit, so this does not establish an independent visual
   match.
2. **Does the look reflect the behavioral state?** Fresh New Game seeds
   `FFFC30` to `1` at startup `0x084E`, so page list entry zero is available.
   `0xF9A4` tests D1 from bit 0 through bit 31 in each of the three flag
   groups. The Qt QA adds source bits 2, 33, and 95 and verifies visible order
   `0, 2, 33, 95`, page selection, the one-page default, and right-edge clamp.
3. **Are the visuals themselves correct?** ROM `0xF9A4` clears through
   `0xC3E6`, writes the title/separator from `0xC56C9`/`0xC56D6`, the source
   page label from `0xC663A`, and the selected entry from `FFEF16` through
   `0xC0DA` to `0xC38A`. Qt places those at `(88,32)`, `(72,40)`, `(136,184)`,
   and `(40,56)`. Current/total page numbers are right-aligned at the traced
   `0xCBAE`/`0xCBBA` cells. The loaded source page-two image fits its 25-column,
   16-row panel bounds and leaves the page counter row clear.
4. **Is it animated, and should it be?** `0xF9A4` calls `0xE660` once per
   VBlank. The typed Qt update advances it once per host frame; at 60 FPS that
   follows the source cadence. The 30-frame QA shows the lower-bezel cells
   changing while page text and counter stay fixed. The effect remains below
   the panel content.
5. **Does sound accompany it?** Tips drawing and Left/Right/A/B/Start do not
   request audio. C calls `0x391A`, which requests event `0x25`; Qt maps that
   event to `audio.effects.resource.037` (2.134 seconds including its release
   tail) and keeps the selected page open. The asset identity is checked
   against the independent Ares longplay audio, but speaker output during this
   Tips action remains unverified.
6. **Are related parts of the screen also correct?** The review checks both
   page text and page count, the shared lower animation, input edge behavior,
   and the parent Notebook before/after child entry. The page body reaches at
   most 25 characters by 16 rows; the frame/counter remains visible and the
   effect does not cover those controls.
7. **Do multiple frames support the interpretation?** The QA captures the
   default page, page two, a 30-frame hold, and A/B/Start returns. These are
   multiple Qt states, not multiple real-game examples. No independent Tips
   frame is present in the supplied longplay; visual fidelity remains partial.
8. **Was the old screen cleared before the new one was drawn?** Every Qt paint
   rebuilds the shared outer frame and blank inner grid, then draws only Tips.
   The QA checks the old Tips button and selector are absent at `(88,72)` and
   `(72,72)`, where page zero has a blank body row. A/B/Start return captures
   confirm the tip body and counter disappear before Notebook options return.

### Applied review: Dismiss Runner

1. **Does it look correct?** The Qt dump checkpoints are written under
    `decompilation/ghidra_conversion/build-qt-magic-qa/qt_runtime/field-secretary-qa/`;
   the route records entry, moved selection, 30 held frames, B return,
   one-candidate layout, Abort return, Start return, and active-runner
   replacement. These show the current Qt output only. The supplied longplay
   has no Dismiss Runner visit, so there is no independent Genesis image to
   judge visual correctness against.
2. **Does the look reflect the behavioral state?** Typed
   `DismissRunnerMenuState` contains the live candidate slots and selected
   row. Source `0xC448` tests bit 0 of `+0x10` while walking the two live
   records in source order. Qt builds candidates from active compact party
   slots whose corresponding live bit is set. The `x` selector follows the
   source cursor at x=72; selection movement requests event `0x25`. A/B/C
   confirms the highlighted candidate or Abort with event `0x24`, and Start
   returns to Pocket Secretary. Dismissing a candidate clears its compact
   party marker and preserves its live record flags. Dismissing the currently
    active runner switches the field HUD to the next active record. Selection
    also projects the chosen record's source portrait attributes into typed
    preview state.
3. **Are the visuals themselves correct?** ROM `0xC448` clears through
   `0xC3E6` and writes its fixed text from `0xC1499` at `0xC216`. The visible
   separator at `0xC14AE` is included. `Abort` is fixed at `(88,64)`;
   candidate names start at `(88,80)` and advance 16 pixels, regardless of
   how many candidates remain. The cursor starts at `(72,64)` and follows the
    selected row. Candidate selection invokes the `0x110E8` projection using
    the live record's `+0x50` table pointer, `+0x54` offset, and `+0x2C`
    palette bits; Qt draws its decoded 32x32 portrait at `(120,144)`. QA checks
    that the portrait crop matches the selected source-derived asset. This is
    renderer-consistency evidence; no Genesis capture is available to confirm
    typography, pixel color, or alignment independently.
4. **Is it animated, and should it be?** The source returns through `0xC4C4`
   and calls `0xE660` once per VBlank while the child waits. The typed Qt
    runtime advances the traced random two-cell effect once per frame and
    draws it into the twelve source-mapped lower-bezel positions. Cells persist
    until the owning panel is rebuilt. CTest checks 30 held frames: lamps
    change while text, cursor, and portrait stay stable. Candidate movement
    updates the portrait preview; the supplied longplay has no Dismiss visit,
    so Genesis animation and preview cadence remain unverified.
5. **Does it have a sound?** Up/down requests source event `0x25`; the
   selected/Abort action requests `0x24`; Start and child entry request no
   additional event. Event `0x25` maps to `audio.effects.resource.037` (2.134
   seconds including its release tail); event `0x24` maps to
   `audio.menu.command-024` (2.484 seconds). The CTest checks the event/alias
   route, not speaker output for this screen.
6. **Are nearby screen parts also correct?** The review includes the parent
   Pocket Secretary, the Dismiss child, both candidate counts, each return
    path, party membership, live-record flags, active-runner HUD selection,
    and the selected portrait. It confirms the shared outer frame remains
    while the child replaces the parent controls. The lower-bezel lamps stay
    within their traced positions and do not overlap child controls.
7. **Do multiple frames support the interpretation?** The Qt QA produces
   entry, movement, held, dismissal, return, Abort, Start, and active-runner
    replacement frames. They support the implementation's state changes,
    portrait selection, and lower-bezel animation over time. They are not independent Genesis examples;
   the supplied longplay has no Dismiss Runner frames.
8. **Was the previous screen cleared before new controls were drawn?** The
   child painter redraws the blank panel before writing Dismiss, Who??,
   separator, Abort, names, and cursor. QA verifies the five parent option
   rows are absent in the child. After B, Abort, and Start, it compares the
    complete inner content panel against the pre-entry parent image and checks
    that child title, prompt, Abort row, candidate names, and portrait are gone.
    The shared outer frame is intentional. The 30-frame check also confirms the
    lamp effect does not paint over controls or the portrait; no old button or
    child graphic remains underneath the replacement controls.

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
   `manager` and `manager-package` states. It now reads the source command on
   the arrival `<MORE>` option and runs `0x249C4`: a 0..79 sample opens the
   two-option manager record at `0xF4AB4`; A then opens the room offer. The
   field uses the source bounded-random algorithm, but its seed remains
   host-initialized. The `0x1E1FC` effect callback for samples 80..99 is not
   connected. The synthetic second-speaker check supplies its own text and no
   portrait, so the presenter does not borrow Stoker's image. The property
   selecting the three-option package state in longplay frame 000300 and
   additional live speaker providers remain unconnected.
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
   continuation page use the same panel. The ordinary random-arrival result
   now reaches the manager prompt, but the rare effect, room payment, debt
   payment, package-property selection, and live building placement are not
   connected end to end.
7. **Do multiple frames support the interpretation?** Section 03 frame 000300
   shows a three-option manager/package state, 000600 shows the debt question,
   001200 shows an information page with another portrait, 001500 shows
   another three-choice page, and 001800 shows `< MORE >`. The shared frame
   persists while the speaker and source-provided contents change. Adjacent
   000300-000302 and 001800-001802 captures are pixel-identical holds. The
   current `0xF4AB4` branch has two choices; no exact independent longplay
   frame for that result has been identified, and frame 000300 is not used as
   its golden.
8. **Was the previous screen cleared before the new one was drawn?** Each open
   page clears the full 320x224 canvas, redraws the frame, then clears both
   source content wells before drawing new text and markers. The Debug and
   Release tests paint over a magenta sentinel and confirm it is gone; they
   also verify the source A marker asset, no Stoker portrait for the synthetic
   speaker, and removal of the third option and marker on the next page. The
   arrival-result QA confirms that the previous `<MORE>` and C-option regions
   are cleared before the two-option manager screen is shown; the injected
   successful ambush result clears A/B/C markers before its no-choice text.
   CLI captures are standalone views, so the live field-to-dialog-to-field
   restoration remains unverified.

**Status:** partial. The shared frame, conditional frame asset, source option
markers, body/choice placement, blank rows, and observed static timing are
source traced and checked against multiple independent gameplay frames. Debug
and Release tests cover the 0xE9 thresholds, ordinary arrival-to-manager and
room-offer transitions, injected ambush text, stale-control clearing, and the
manager-to-debt-to-referral path. The manager dump for the two-option
`0xF4AB4` record is renderer output, not an independent golden. The rare
`0x1E1FC` effect, host-vs-source random seed, property selecting the
three-option package state, `FFF0AF` frame selection, additional speaker data,
remaining payment results, live field restoration, and measured dialogue
audio remain open.
