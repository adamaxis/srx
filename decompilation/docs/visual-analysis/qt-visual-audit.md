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

### Applied review: SEGA shock figure fade

The previous Qt fade reduced the extracted figure by a linear RGB weight over
many updates. ROM `0x5D8BA` instead calls the figure palette routine on its
five-tick cadence, decrementing the Genesis color components one discrete level
per call. The rendered actor therefore remained visible after the logo had
resolved.

1. **Does it look correct?** Independent section 01 frames 000060, 000090,
   and 000120 show the raised figure and lightning around the SEGA mark. Frames
   000150 and 000180 show the resolved mark without either layer; 000198 shows
   the mark dim, and 000204 is black. The red uploader overlay is excluded.
   Native `sega-shock-start.bmp`, `sega-shock-lightning.bmp`,
   `sega-strobe-start.bmp`, `sega-strobe-lightning-cleared.bmp`,
   `sega-figure-fade-step-1.bmp`, and `sega-resolved.bmp` are under
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/boot-fasa-qa/`.
   The native resolved image shows the logo alone. The independent capture is
   30 fps, so it supports the state order but not an exact 60-Hz frame offset.
2. **Does the look reflect behavioral state?** ROM `0x5D8BA` decrements
   `FFDF90`, calls `0x5B2E2` with palette index 3 when the counter reaches 2,
   and resets the expired counter to 4. `0x5B2E2` updates fifteen color words
   and marks the palette dirty. Native state records
   `strobe_countdown` and `figure_palette_fade_steps`; the fade is capped at
   the seven levels available in a Genesis color component. The typed
   `sega_lightning_batches_active` records what `0x5D996` built before the
   parent loop updated the strobe flag, so it can differ from the post-loop
   stage for one tick. There is no menu selection state here.
3. **Are the visuals themselves correct?** The ROM call subtracts one
   `0x002`, `0x020`, or `0x200` component step only when that component is
   nonzero. Qt now applies those discrete palette levels to the named
   `boot.guy` asset; it no longer scales the actor's RGB channels by a
   fractional weight. The lightning and SEGA logo remain separate visual
   layers. The internal full-frame check confirms the resolved Qt composition
   contains only the snow background and the current logo strobe layer; this
   checks stale-layer clearing, not Genesis pixel fidelity.
4. **Is it animated, and should it be?** Yes. `0x5D8BA` checks the countdown
   every source VBlank; after its first phase update, `0x5B2E2` runs once every
   five VBlanks. Seven calls take the figure palette to black, after which the
   step count stays at seven even if the source callback continues. Unit QA
   checks the first call, five-VBlank spacing, seven-step limit, and later
   callbacks. ROM `0x5D938` builds the lightning batches before the parent
   sets its strobe-active flag, so the lightning remains on the activation
   frame and is absent on the next frame. Qt QA captures shock entry at 93,
   lightning at 94, strobe activation at 222, its clean next frame at 223,
   fade step 1 at 225, and the resolved figure at 255.
5. **Does sound accompany it?** `0x5D8BA` and `0x5B2E2` make no sound call.
   The parent `0x5D746` submits command `0x16` through `0x530D2` when its logo
   counter is below six and again when it reaches zero, the threshold that
   enables the strobe. It also submits command `0x5C` through `0x530DE` while
   the counter is above four. These requests do not run once per palette fade
   step. The extracted aliases and measured clip durations for `0x16` and
   `0x5C` remain unresolved, so the sound identity and length are not claimed.
6. **Are related screen parts also correct?** The source frames and native
   dumps were checked across the entire black background, SEGA mark, lightning,
   and figure. At the resolved state, the full-frame assertion rejects a
   leftover actor or lightning layer. FASA's next stage clears both planes;
   its separate QA also checks the black handoff and rejects old SEGA pixels.
7. **Do multiple frames support the interpretation?** Yes. The independent
   frame set covers initial/rising shock (000060/000090), active shock
   (000120), resolved logo (000150/000180), dim logo (000198), and black
   (000204). Native checkpoints cover shock entry, lightning, the final
   lightning frame at strobe entry, its removal, the first palette decrement,
   and the resolved mark. The capture cadence cannot confirm the exact native
   update count.
8. **Was an old screen left underneath the next one?** This boot sequence has
   no buttons or menu controls. The resolved-frame comparison checks that the
   figure and lightning are gone while the SEGA mark remains. The following
   FASA path clears both planes before drawing its mark, and the FASA QA checks
   those transition frames for old SEGA graphics.

The code now follows the source palette cadence, and internal Debug/Release
tests pass. The whole pre-title hill remains partial: the supplied 30-fps
capture cannot settle exact source-frame alignment, and the adjacent SEGA
audio command assets and durations have not been identified.

### Applied review: FASA boot mark

The first Qt pass skipped the FASA transition and showed its logo in cyan.
The full-frame review traced two causes: the incremental state machine had
collapsed blocking palette fades, and the offline bitmap importer omitted the
palette-line attribute that the ROM adds when it draws FASA.

1. **Does it look correct?** Independent section 01 frames 210, 216, 240, 280,
   290, and 300 show a dim blue FASA mark, the full mark, a sustained hold,
   the mark dimming, then black. The corrected full-bright Qt image is
   `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/boot-fasa-qa/actual.bmp`;
   its blue hue and centered 127x40 mark match the independent frames. The
   previous cyan result came from palette line 0.
2. **Does the look reflect the behavioral state?** Native state now carries
   `presentation_stage`, `palette_transition_passes_completed`,
   `palette_transition_vblank_phase`, and `fasa_hold_frames`. ROM `0x5D746`
   runs 337 source frames, `0x131C` fades the SEGA scene to black, `0x5D886`
   installs FASA and calls `0x12F6` to fade toward its target, then the
   `0x5D86A` DBF loop waits 121 VBlanks before the second `0x131C`. There is
   no selection state
   or cursor on this screen. Qt QA records FASA fade-in at frame 371, full
   brightness at 405, hold end at 526, and title handoff at 560.
3. **Are the visuals themselves correct?** The ROM draws the 16x5 name table
   from `0x1FFAC6` at Plane B `0xE518` with attribute `0x4000`, selecting
   palette line 2. The offline importer now applies that attribute; CRAM word
   `0x0E60` expands to the blue `#006DFF`. This agrees with the independent
   blue logo. The source capture is compressed video, so it supports hue and
   placement but not exact pixel equality.
4. **Is it animated, and should it be?** Yes. ROM `0x131C` and `0x12F6` each
   execute 17 palette passes; each pass waits for two VBlanks in `0x1342`, so
   each fade lasts 34 VBlanks. The FASA hold is the `0x78` DBF loop, or 121
   VBlanks. The native trace includes fade-in start/pass 1/full, the final
   hold frame, fade-out start/pass 1/black, and title handoff. The independent
   30-fps capture shows the same sequence across frames 210-300 but does not
   establish an exact cross-capture frame offset.
5. **Does sound accompany it?** Neither FASA palette routine nor the FASA
   install/hold path requests a sound effect, so there is no FASA event ID or
   clip duration to assign. The preceding SEGA loop issues `0x16` and `0x5C`
   through separate audio service paths; they are not tied to the FASA
   transitions. The longplay WAV does not isolate those service requests or
   establish their individual clip durations.
6. **Are related parts also correct?** The full-frame audit checks that the
   fade begins from black, that the stable scene contains the blue logo, and
   that no old SEGA layer or control is visible. `actual.bmp` remains static
   through the last hold snapshot. The final fade reaches a black frame before
   the title screen appears.
7. **Do multiple frames support the interpretation?** Yes. The independent
   frame set covers dim, full, sustained, dimming, and black states at 210,
   216, 240, 280, 290, and 300. Native QA saves the corresponding state
   boundaries under `build-ghidra-conversion-qt-debug/qt_runtime/boot-fasa-qa/`.
   It also verifies that `--boot-dump-frame 373` matches the recorded first
   FASA palette pass, so the review CLI includes the live fade state.
8. **Was the previous screen cleared?** ROM `0x5D886` clears both planes
   before writing the FASA grid. The Qt FASA stage draws only the FASA asset;
   QA compares the whole 320x224 result with that single layer over black.
   On completion, the title renderer replaces it and the QA rejects a stale
   FASA frame.

The FASA transition and palette defect are fixed in the Qt conversion path.
The broader boot hill remains partial until the complete pre-title sequence
has its own defended screen audit and an independent capture that resolves
the exact 60-Hz timing and the adjacent SEGA audio clips.

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
   `QMediaPlayer` now decodes and starts this FLAC on the default output. The
   shared audio CTest checks `PlayingState` and the 2.133-second decoded
   duration. Independent Ares longplay audio has repeated waveform matches for
   resource 037; Windows loopback for this exact First Aid visit remains
   unrecorded. Action-button dispatch
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

### Applied review: equipped and unequipped inventory glyphs

1. **Does it look correct?** The report was a large dark block remaining
   around an item after Equip/unequip. I inspected full 320x224 Qt checkpoints
   `field-action-qa/00_inventory_action_equip.bmp`,
   `05_weapon_1_unequipped.bmp`,
   `10_weapon_2_equipped_at_50_percent.bmp`, and
   `11_weapon_2_unequipped_at_50_percent.bmp`, plus an enlarged 2x4-cell
   contact sheet. In these states the pale equipment-cell treatment appears
   for the active weapon and disappears after unequip; the dark/gray rectangle
   inside the item glyph remains part of the glyph artwork.
2. **Does the look reflect the behavioral state?** The typed
   `RunnerRecordFields::equipped_inventory_slot` and `equipped_item_code`
   control the equipped-cell treatment and icon palette. The inventory codes
   remain in `+0x66..+0x6D`. QA equips slot 0, unequips it, switches to slot 1,
   equips and unequips it, and checks that the selected item stays selected
   while the separate equipment treatment follows the live slot.
3. **Are the visuals correct?** ROM `0x11136` writes each 3x2 inventory icon
   as name-table glyphs through `0x11190`; `0x3602` selects its palette
   attributes. These are opaque plane tiles, not transparent SAT sprites, so
   color zero supplies the icon's tile background. ROM `0x11624` applies the
   separate equipment cell before the icon row; `0x11592` supplies the slim
   selected-item outline after it. The Qt QA compares the 22x14 icon interior
   against the named 24x16 palette asset and checks separate equipped-cell
   pixels before and after each transition.
4. **Is it animated?** The icon, equipment treatment, and cursor have no
   animation counter in these paths. They redraw on menu entry or a selection
   or equipment state change. The tested icon remains stationary between
   inputs.
5. **Does sound accompany it?** Equip/unequip sends event `0x24`, mapped to
   `audio.menu.command-024` (2.484 seconds including its release tail).
   Selector movement sends event `0x25`, mapped to
   `audio.effects.resource.037` (2.134 seconds including its release tail).
   The Qt QA checks event requests and aliases; it does not measure speaker
   output. The static icon itself has no separate sound.
6. **Are related parts correct?** I checked the full screen shell, inventory
   boxes, both weapon cells, icon interiors, cursor, equipment treatment,
   center detail panel, and right-side HUD across both weapon slots. The QA
   repeats the weapon-1 equip/unequip cycle and requires the full 2x4 inventory
   pixel region to equal the first unequipped result, guarding against
   accumulated overdraw or leftover cell graphics.
7. **Do multiple frames support this?** Independent section 06 frames
   002550, 002560, 002580, 002610, and 002640 show the repeated item-cell
   layout, armor treatment, and cursor states. The first weapon glyph in
   002550 and later held frames is closest to extracted palette variant 0.
   The capture does not show this exact weapon-1/weapon-2 equip cycle, so it
   does not independently prove the weapon toggle transition or every icon
   palette state; keep the inventory hill `partial`.
8. **Was old UI cleared before drawing?** `paint_command_overview` redraws
   the command background and inventory boxes first, then adds equipment
   treatment only for the live equipped slot, followed by item glyphs and the
   cursor. Pixel checks require each treatment to disappear after unequip;
   the repeated-cycle comparison checks the complete grid region for stale
   pixels. The action child and parent return checkpoints continue to verify
   their own panel redraws.

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
   `0x24` maps to `audio.menu.command-024` (2.484 seconds). The shared audio CTest also verifies that event `0x24` decodes and starts on
   the default output with the 2.483-second duration. It does not capture
   Windows loopback for this specific First Aid input path. No separate result sound is
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
`partial`: Joshua-only roster integration, host RNG seeding, Windows loopback
capture for the exact First Aid sequence, and same-archetype visual support
still need work.

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
   Independent Ares longplay audio has repeated matches for resource 037. The
   shared audio CTest confirms the event reaches default-device `PlayingState`;
   Windows loopback for this command-menu route remains unrecorded.
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
shadowrun_ghidra_conversion_qt.exe --field-player-action-dump <dir> --assets <assets>
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
   and `0x24` maps to `audio.menu.command-024` (2.484 seconds). The source sample is cross-checked against independent Ares longplay audio.
   The shared audio CTest verifies event `0x24` reaches default-device
   `PlayingState`; Windows loopback for this exact Trade route remains
   unrecorded.
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
   against the independent Ares longplay audio. The shared audio CTest verifies
   event `0x25` reaches default-device `PlayingState`; Windows loopback for this
   exact Tips action remains unrecorded.
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
   route; the shared audio CTest verifies default-device playback for both
   events, while Windows loopback for this exact screen remains unrecorded.
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
   connected. A on the room offer dispatches `0x21458` through the typed
   payment resolver: exactly 20 nuyen opens the paid page and subtracts the
   charge, while 19 leaves the wallet untouched and opens the low-cred page.
   Dismissing the paid page requests Hotel Options, whose Qt screen is still
   an explicit boundary; loaded-save wallet state is unavailable. The
   synthetic second-speaker check supplies its own text and no
   portrait, so the presenter does not borrow Stoker's image. The property
   selecting the three-option package state in longplay frame 000300 and
   additional live speaker providers remain unconnected.
3. **Are the visuals themselves correct?** ROM `0x10984` installs nine shared
   frame regions using patterns loaded at `0x10918`; its tenth `0xCEFF0` block
   replaces the left panel when `FFF0AF` is set. The offline importer exports
   both 320x224 frame variants. ROM `0x1F5A8` clears the body at `(88,16)` and
   the lower panel at `(16,152)` before `0x1F3C4` draws content. That prompt
   path uses `0xC168` with a 27-count wrap scan and up to 26 visible glyphs,
   then sends its 2x2 A/B/C markers
   from `0xCF11C`, `0xCF124`, and `0xCF12C` through `0x1232`. These assets
   replace the former hand-drawn bevel and square markers. The `FFF0AF` frame
   choice is represented in typed view state, but its live source binding is
   not implemented.
4. **Is it animated, and should it be?** The prompt redraws on entry or page
   change, waits for VBlank, and samples input; it has no dialogue animation
   counter. Independent frames 000300-000302 and 001800-001802 are pixel
   identical holds. The frame and portrait therefore stay static in the
   observed pages; other event scenes have not been checked. The paid and
   insufficient-funds pages also remain pixel-stable through 30 input-free
   controller frames in both Qt configurations.
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
   now reaches the manager prompt, and room payment selects its source
   paid/insufficient records from the typed wallet. The paid message requests
   Hotel Options, but that screen and live building placement are not yet
   connected end to end. The rare effect, debt payment, package-property
   selection, and restored-save wallet also remain open.
7. **Do multiple frames support the interpretation?** Section 03 frame 000300
   shows a three-option manager/package state, 000600 shows the debt question,
   001200 shows an information page with another portrait, 001500 shows
   another three-choice page, and 001800 shows `< MORE >`. The shared frame
   persists while the speaker and source-provided contents change. Adjacent
   000300-000302 and 001800-001802 captures are pixel-identical holds. The
   current `0xF4AB4` branch has two choices, and the payment-result records
   have no exact independent longplay frames. Frame 000300 is not used as a
   golden for those states.
8. **Was the previous screen cleared before the new one was drawn?** Each open
   page clears the full 320x224 canvas, redraws the frame, then clears both
   source content wells before drawing new text and markers. The Debug and
   Release tests paint over a magenta sentinel and confirm it is gone; they
   also verify the source A marker asset, no Stoker portrait for the synthetic
   speaker, and removal of the third option and marker on the next page. The
   arrival-result QA confirms that the previous `<MORE>` and C-option regions
   are cleared before the two-option manager screen is shown; the injected
   successful ambush result clears A/B/C markers before its no-choice text.
   Both room-payment results also clear the room offer's A/B controls before
   drawing their no-choice message.
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
debt-payment result, Hotel Options UI, loaded-save wallet state, live field
restoration, and measured dialogue audio remain open. Exact independent
longplay frames for the two-option manager and room-payment outcomes are not
available.

### Applied review: Stoker follow-up commands and Notebook flags

1. **Does it look correct?** I inspected the Debug Qt captures at
   `field-dialogue-qa/debt-followup/actual.bmp`,
   `field-dialogue-qa/referral-followup/actual.bmp`,
   `field-dialogue-qa/referral-followup/notebook-after-referral/actual.bmp`,
   and `field-dialogue-qa/referral-followup/notebook-tips-after-debt/actual.bmp`.
   The debt prompt shell, Stoker portrait, source A/B choice labels, and the
   title-only referral match independent section 03 frames 000600 and 000690.
   The subsequent Notebook output shows Current Run, Tips & Clues, Contacts,
   and the newly available Mr. Johnsons row.
2. **Does the look reflect the behavioral state?** The manager-package C
   command runs typed source command `0x30`: bit 1 is added to `FFFC30` before
   F4E8C opens. The debt-page B command runs `0x31`: bit 0 is added to
   `FFFC27` before F5AB0 opens. New Game already seeds bit 0 in `FFFC30`;
   together the flags expose Tips entries 0 and 1. New Game clears
   `FFFC27/2A`, then sets Contacts `FFFC2C` to `0x1C7F`, so the referral makes
   Mr. Johnsons a fourth Notebook row while Shadowrunners remains gated. A
   restored save without these fields stops at the source-state boundary.
3. **Are the visuals themselves correct?** ROM bytes for `0x213CA` and
   `0x213E2` set those individual bits and return F4E8C/F5AB0. F33E tests
   FFFC27 before appending Mr. Johnsons; `0xF458` checks bit zero. The Qt
   controller opens the text records through the generic speaker panel and
   rebuilds the Notebook from its current typed flags. The referral has no
   choice markers; the lower choice panel is blank after the debt page's A/B
   controls are removed. The Notebook capture has its Current Run selector,
   four source rows, and no fifth Shadowrunners row. The source frame is
   directly extracted; visual agreement was checked against the independent
   longplay, not against a second compositor.
4. **Is it animated, and should it be?** The prompt command records do not
   set an animation counter. Independent debt frames 000600-000602 are
   byte-identical. Referral frames 000750 and 000780 are also byte-identical,
   and frames 000690, 000720, 000810, and 000840 show the same held prompt;
   nearby compressed-video differences do not show a new visual state. The
   Qt runtime test holds both pages for 30 input-free controller frames and
   checks the entire 320x224 image remains unchanged.
5. **Does it have a sound or sound effect, and how long does it play?** The
   two command routines do not issue a direct sound-play call. Shared prompt
   routine `0x1F246` has an optional indirect `A1` hook when FFF10C is set,
   and its callee is unresolved. The gameplay capture does not isolate a
   command-specific effect from the continuous soundtrack, so the effect
   identity and duration are unknown; no duration is invented.
6. **Are other parts of the screen also correct?** QA checks the debt prompt,
   referral, field return, Pocket Secretary entry, Notebook rows, and Tips
   page. It verifies the debt writer reveals source tip 1, the referral writer
   reveals only the Mr. Johnsons gate, Contacts remains visible from its New
   Game seed, and Shadowrunners stays absent. The Mr. Johnsons row's detail
   handler is still an explicit boundary.
7. **Do multiple frames support this interpretation?** Independent section
   03 frames 000600-000602 show the debt question held across three adjacent
   captures. Frames 000690, 000720, 000750, 000780, 000810, and 000840 show
   the Jump House referral at several later points; frames 000750 and 000780
   are byte-identical. Debug and Release CTests add controller-driven route
   coverage and named `actual.bmp` artifacts, but are kept separate from the
   Genesis evidence.
8. **Was old UI cleared before new controls were drawn?** On the debt-to-
   referral change, the panel is redrawn as a title-only record and the old
   A/B controls disappear. The diagnostic then dismisses the prompt, uses the
   command overview to open Pocket Secretary and Notebook, and enters Tips.
   Pixel checks confirm the Stoker text and old Notebook rows do not remain
   over those screens; Notebook keeps the shared frame and redraws only its
   current rows/child page.

**Status:** partial. Both follow-up command bodies, their prompt selection,
the New Game flag sources, dynamic Notebook gates, and the filtered Tips
result are connected and checked in Debug/Release. Independent Genesis frames
support both text records. The property choosing the manager-package variant,
loaded-save flag restoration, paid-debt/package result for source command
`0x05`, Mr. Johnsons detail handler, unrelated live field trigger, and
unresolved shared audio hook remain open.

### Applied review: Stoker debt-payment low-funds branch

1. **Does it look correct?** I inspected
   `field-dialogue-qa/debt-payment-insufficient/actual.bmp` beside independent
   section 03 frames 000690, 000750, and 000840. The title-only Jump House
   page, text line breaks, frame, and blank lower choice well agree. The
   extracted Stoker portrait is visibly more saturated than the compressed
   video capture, so this pass does not claim pixel-exact portrait color.
2. **Does the look reflect the behavioral state?** ROM `0x213FA` compares
   FFFBFE with 250; below that balance it branches to `0x213E2`, which sets
   FFFC27 bit 0 and returns F5AB0. The new-game initializer at `0x007F0`
   supplies 20 nuyen. Qt keeps that balance, writes the referral flag, and
   selects the title-only result. Balances at or above 250 stop at the
   explicit success-state boundary because FFDD64 and the runner/package
   fields are not available here.
3. **Are the visuals themselves correct?** The referral string at ROM
   `0xF41A1` includes the comma after “Hey” and two `0x80` paragraph
   separators. The shared panel uses the source 27-count wrap scan as 26
   visible glyph columns; the previous 27-glyph host line let words run into
   the frame. The result has no A/B/C markers, matching its source record.
4. **Is it animated, and should it be?** The command only updates flags and
   selects the referral record; it sets no animation counter. Independent
   frames 000690, 000720, 000750, 000780, 000810, and 000840 show the same
   held page. The Qt runtime test holds it for 30 controller frames and
   confirms the 320x224 image stays unchanged.
5. **Does it have a sound or sound effect, and how long does it play?** The
   `0x213FA` and `0x213E2` command bodies issue no direct sound call. The
   common `0x1F246` prompt has an unresolved indirect `A1` hook, and the
   capture does not isolate an effect from the soundtrack. Sound identity
   and duration remain unknown.
6. **Are other parts of the screen also correct?** The route assertion checks
   the unchanged 20-nuyen wallet and FFFC27 bit 0, then opens Pocket
   Secretary and Notebook. Its dump includes the Mr. Johnsons row, excludes
   the still-gated Shadowrunners row, and shows the two unlocked Tips entries.
   The text well is fully repainted when the title-only page opens.
7. **Do multiple frames support this interpretation?** Frames 000600-000602
   show the debt question held before selection. Frames 000690, 000750, and
   000840 show the referral page later in the same section; the captured page
   follows the B route, so it supports the output appearance but does not
   independently show the A/low-funds transition. Debug and Release CTests
   cover that controller route and write named `actual.bmp` files.
8. **Was old UI cleared before new controls were drawn?** The A/B debt rows
   and markers disappear when the referral replaces that panel. Pixel tests
   check the lower well and compare the result to a separately opened
   title-only referral, then verify the page remains unchanged for 30 frames.
   The controller route dismisses the referral before opening the field menu;
   its Notebook and Tips captures contain no dialogue text or old selector.

**Status:** partial. The New Game low-funds command path is connected and
checked in Debug/Release. The paid-debt branch, loaded-save balance, direct
Genesis example of this A-to-referral transition, live Stoker property
selection, and indirect sound hook remain open.

### Applied review: electromagnetic-lock and Security Terminal interaction

1. **Does it look correct?** I inspected the Debug `actual.bmp` files for
   `maglock-choices` and `security-terminal`, plus each `return-field.bmp`.
   Both prompts use the shared 320x224 dialogue frame. The lock shows its
   three A/B/C actions; the Security Terminal shows Electronics and Walk Away.
   A nearest-neighbor crop of the Security choice well confirms the long
   Electronics label is drawn. These are internal renderer captures, not an
   independent Genesis comparison.
2. **Does the look reflect the behavioral state?** The ROM mode-2/event route
   selects the electromagnetic-lock branch at `0x20E48` and the Security
   Terminal branch at `0x20EC2`. The lock begins on `<MORE>`, waits the shared
   ten-frame choice gate, then builds typed Electronics/Grenade/Walk Away
   rows from the interaction selection and active party inventory. The
   terminal exposes typed Electronics/Walk Away rows. The movement runtime
   now receives the active runner's source `+0x10` behavior byte from the
   selected archetype or restored record; `0xB7B8` requires bit 1 before it
   can resolve the blocked-cell interaction. The three `0x7291A` archetypes
   all carry `0x0F` there. The separate `0x44BC` default `0x05` is for spawned
   world entities. The floor source chain is now recorded: each resource-scene
   profile `0x15..0x2B` points at a bundle through `+0x58`; the bundle's first
   longword points to the bound consumed by `0x0D3A`, which publishes its sample
   to `FFE12C` before `0x0D44` selects the activity. The offline importer now
   exports all 23 bounds as individually named two-byte assets, and the typed
   loader validates them and passes the matching bound with scene metadata.
   The Qt activity dispatcher still does not sample that field, so the floor
   sentence remains withheld when the field bootstrap lacks `FFE12C`.
3. **Are the visuals themselves correct?** The presenter fills the complete
   screen, redraws the extracted shared frame, clears both source content
   wells, then draws source text and A/B/C marker assets. The first option is
   present in the terminal capture; its green text was difficult to see at
   full-frame scale, so the test now requires substantial green-pixel counts
   in both choice rows as well as checking the exact typed label and role.
   The exact Genesis placement remains unverified without matching gameplay
   frames.
4. **Is it animated, and should it be?** These prompt/result records do not
   set an animation counter. The shared prompt logic gates choice input for
   ten frames and holds result text for thirty frames before accepting a
   dismissal input. The choice captures are static across the input-free
   holds; field animation is paused while the full-screen modal owns the
   window.
5. **Does it have a sound or sound effect, and how long does it play?** The
   unlock command `0x4B` resolves to extracted resource 075. The FLAC is valid
   and contains 97,770 samples at 44,100 Hz, or 2.2167 seconds; the Qt resolver
   test confirms that this event decodes and reaches `PlayingState` on the
   default output. Windows loopback for the in-world unlock sequence remains
   unrecorded. Grenade command `0x2C` is table resource 044, which
   the offline audio catalog classifies as silent and omits. Alert command
   `0x30` points to persistent resource 048; its FLAC is a 600-second
   observation capture without a verified source stop boundary. Qt leaves
   that event unmapped rather than playing the observation window as a
   one-shot. Its actual gameplay duration remains unknown.
6. **Are other parts of the screen also correct?** While the prompt is open,
   `FieldSceneRuntime::paint` returns after the modal draws, so the old field
   HUD/map is not visible behind the dialogue controls. Walk Away closes the
   modal and writes `return-field.bmp`; Debug/Release QA compares that entire
   image pixel-for-pixel with the pre-prompt field render.
7. **Do multiple frames support this interpretation?** Debug/Release tests
   exercise the `<MORE>` page, action rows, passkey success, electronics
   outcomes, grenade inventory mutation, Security Terminal outcomes, result
   holds, and return. No independent longplay frame showing these exact
   object prompts or their results was identified in supplied sections
   03/04, so the internal frames do not serve as multiple Genesis examples.
8. **Was old UI cleared before new controls were drawn?** The shared presenter
   clears the full 320x224 surface and both content wells before drawing each
   page. The return check compares the post-modal field image with the exact
   pre-modal image, so any dialogue text, button marker, or frame left over
   fails the route QA. Debug/Release interaction QA asserts the selected
   runner's bit-1 gate, but still injects the typed continuation after event
   resolution. The actual in-world object collision trigger still needs an
   end-to-end gameplay test.

**Status:** partial. Typed prompt/result behavior, menu rendering, audio
resource 075 mapping, source behavior-flag binding, full-frame modal ownership,
clean field return, and extraction/loading of the scene random-index bounds
have Debug/Release test coverage. The runtime `FFE12C` sample, independent
Genesis frames, the actual field object trigger route, resource-048 alert
lifetime, and captured device audio remain open.

### Applied review: Start-menu Karma view and Hotel Karma spending page

1. **Does it look correct?** Inspected the fresh Start-menu status capture and
   Hotel spending capture. They share the Attributes/Skills grid and current
   Karma total; the Start page has no upgrade preview, while the Hotel page
   shows the selected next value and cost. These are Qt images. No matching
   independent Genesis capture is available, so Genesis appearance remains
   unknown.
2. **Does the visible look match the behavioral state?** ROM `0xD01C` writes
   `FFF116=-1` and opens the read-only current-Karma route. It does not mutate
   stats or Karma. Hotel option 0 enters at `0xD024`, carries upgrade
   selection/cost data, and opens confirmation before mutation. FieldSceneRuntime
   owns separate fixed-mode command-view and Hotel screen types.
3. **Are the visuals themselves correct?** The two variants share their grid
   and current-total placement. The pixel QA requires the only difference to
   be in the source lower detail line and checks that the Hotel upgrade/cost
   detail is present only there. This validates route composition against
   extracted assets, not Genesis fidelity.
4. **Is it animated, and should it be?** Both child pages are static between
   input edges. The row selector moves on input; Hotel E660 animation pauses
   while its Karma child is open. No independent capture establishes Genesis
   cadence.
5. **Does it have a sound or sound effect, and how long does it play?** The
   outer Start command action sends event `0x24` before entering the view-only
   child; that child adds no event. The Hotel Karma child is silent. The
   extracted `0x24` file is 2.484 seconds and the shared Qt audio CTest reaches
   decoded `PlayingState`; exact-route output loopback was not recorded.
6. **Are the other visible parts of the screen also correct?** Start entry
   replaces the command shell and return rebuilds it. Hotel entry removes
   Hotel Options controls; return redraws the options page. Debug/Release QA
   checks the shared value/grid, mutation boundaries, and parent redraw.
7. **Do multiple frames support the interpretation?** Qt QA captures row
   movement, prompts, Yes/No outcomes, spending, and returns. These are
   internal captures. No independent gameplay examples of either Karma page
   were found.
8. **Was old UI cleared before new controls were drawn?** Both routes replace
   the full panel. The Hotel child capture rejects leftover Hotel controls,
   and the Start view return is checked against a fresh command render.

**Status:** the distinct routes and conversion behavior are defended by ROM
trace, typed state, CTests, and dumps. Their exact Genesis visual appearance
remains unverified until matching independent frames are available.

### Applied review: Empty-ammo player reload action

1. **Does it look correct?** Inspected the full 320x224 `before.bmp`,
   `unselected-action-frame.bmp`, `ready-idle-frame.bmp`, and `actual.bmp`:
   the field, active runner HUD, named weapon glyph, and ammo counter remain
   in place while the counter changes from `00` to `06`. No matching
   independent reload frame is present in the supplied longplay, so Genesis
   appearance/timing remains unknown.
2. **Does the visible look match the behavioral state?** The source path
   requires the selected-action behavior bit at `+0x10`, held A
   (FFDD22 bit 6), ammo `+0x4E==0`, shared clips `+0x4F>0`, and a negative
   signed `+0xDC` action gate. QA first presses A while unselected, then
   selects the action behavior while A is released; neither frame reloads.
   The next selected, held-A frame loads six rounds, consumes one clip, and
   rearms the action delay. QA sets the behavior bit directly because live
   combat entry into that action-capable state is not connected.
3. **Are the visuals themselves correct?** The QA compares the complete ammo
   counter and weapon-glyph rectangles with their named extracted HUD assets
   at all four captures. It checks the whole field image for unexpected menu
   state, but the same-source comparison does not prove Genesis fidelity.
4. **Is it animated, and should it be?** The typed `+0xDC` gate counts down
   once per active field behavior frame. This slice does not yet connect the
   ROM attack/target sprite sequence, so it makes no claim about the complete
   combat animation cadence.
5. **Does it have a sound or sound effect, and how long does it play?** The
   successful route submits event `0x4B` twice, from `0x14226` and the `0xACAE`
   success tail. Resource 075 is 97,770 samples at 44.1 kHz (2.2167 seconds).
   Qt QA verifies both event IDs and order; the shared audio CTest verifies a
   single event decodes and plays for about 2.217 seconds. It does not record
   output loopback for the paired reload submissions, so audible retrigger or
   overlap duration remains unknown.
6. **Are the other visible parts of the screen also correct?** The reload
   preserves the weapon glyph, field background, face, bars, and panel chrome.
   All four state frames are checked as full-screen output, and the clip
   reserve is checked alongside the displayed ammo value.
7. **Do multiple frames support the interpretation?** The CLI records the
   initial empty state, A while unselected, selected with A released, and the
   selected held-A reload. They support the Qt state transition, but are not
   independent Genesis examples. The supplied longplay does not show this
   reload route.
8. **Was old UI cleared before new controls were drawn?** This action draws no
   menu controls. QA requires command, dialogue, Hotel, inventory-action, and
   Clips pages to be closed; the captured field retains only its field HUD.

**Status:** partial. The selected-action and held-A entry gates, empty-ammo
clip consumption, action delay, audio event submissions, and HUD projection
are connected and tested with an explicitly injected action behavior bit.
Live combat entry into that state, weapon firing, target selection, collision,
damage, full attack animation, and independent reload/audio captures remain
open.

### Applied review: three-slot field-party HUD

1. **Does it look correct?** Debug QA captures
   `field-hud-qa/party-equipped/actual.bmp`,
   `field-hud-qa/party-active-winston/actual.bmp`, and
   `field-hud-qa/party-ricky-unarmed/actual.bmp`. The supplied longplay frames
   000001/000017/000049 show Joshua with empty companion panels, so they do not
   independently confirm recruited portrait appearance or alignment.
2. **Does the look reflect the behavioral state?** ROM `0x3404` scans the
   three fixed live-record slots. Save markers select recruited record
   families; the field controller's active slot selects the active meter
   profile without moving another runner's panel. Qt reads each typed record
   for its +0x10 draw flag, portrait family, +0x56/+0x57 item state, +0x2C
   item palette, +0x90 cyberware, and live meters/ammo. CTest changes the
   active runner and verifies fixed panel placement.
3. **Are the visuals themselves correct?** The ROM trace follows +0x50/+0x54
   to the runner's 4x4 portrait, +0x57 to the ordinary item row, negative
   +0x56 to D4EA0 spell art, and D1860/D186C/D1878 to unarmed art. The
   extracted catalog contains all 13 runner families, 50 ordinary item codes,
   palette lines 1/2/3, and both 0x3602 portrait/spell palettes. QA checks
   opaque source pixels at each of the three panel coordinates and checks
   that unarmed art replaces Ricky's equipped item.
4. **Is it animated, and should it be?** Portraits and item icons remain
   static between record refreshes. Source `0x3674`, `0x37EC`, and `0x36FC`
   update the value bars and ammo count from live record changes; the Qt
   overlay selects the resulting named value row. There is no separate idle
   animation on the portrait or item image in this path.
5. **Does it have sound, and how long does it play?** No audio call is traced
   to the field HUD asset upload or panel-stamp routines. This slice requests
   no sound and therefore has no HUD-specific playback duration.
6. **Are the other visible parts of the screen also correct?** Full-frame QA
   checks the field scene, right-side HUD shell, and all three portrait/item
   areas together. Each repaint starts from the field HUD background before
   applying the live overlays.
7. **Do multiple frames support the interpretation?** Qt QA records equipped
   Ricky, active-runner switch to Winston, and Ricky unarmed. Those establish
   state-dependent host output. The independent longplay does not contain a
   recruited-party HUD example, so its interpretation remains open to an
   independent game capture.
8. **Was old UI cleared before new controls were drawn?** The HUD reuses the
   source's fixed sidebar panels. Painting rebuilds the frame from its
   background for every image and composes only the current live assets. The
   gear-removal check confirms the previous item pixels disappear before
   unarmed pixels occupy that panel.

**Status:** partial. ROM-derived named assets, fixed three-slot typed state,
portrait/item selection, active-runner profile selection, and unequip redraw
are connected and covered in Debug and Release. Independent recruited-party
visual evidence and recruited hire-to-field-session handoff remain open.

### Applied review: weapon inventory cells after equip and unequip

1. **Does it look correct?** The Debug output frames
   `05_weapon_1_unequipped.bmp` and
   `11_weapon_2_unequipped_at_50_percent.bmp` show the item tiles and slim
   selector without an added black region. Each 2x4 inventory grid also
   matches a freshly bootstrapped scene carrying the same runner record and
   selected column. Independent frames 002550, 002560, 002580, 002610, and
   002699 show the game's inventory layout and selector, but none shows the
   same weapon equip/unequip sequence.
2. **Does the visible look match the behavioral state?** Yes. ROM
   `0x11624` rebuilds the equipment treatment from the active record before
   drawing item glyph rows; `0x11740` handles the inventory action. Qt checks
   the typed equipped slot/code, stored 50-percent magazine value, selected
   cell, and resulting HUD counter at both unequipped weapon states.
3. **Are the visuals themselves correct?** Internally, yes: the unobscured
   22x14 icon interior matches its named 24x16 ROM-derived item asset, the
   one-pixel warm-gold outline remains on the icon edge, and the full inventory
   grid matches a fresh scene. The longplay supports the shared layout and
   selector color, but it does not independently verify this weapon state.
4. **Is it animated, and should it be?** No animation is traced for the item
   tiles or equipment treatment in this menu path. They update on equipment
   changes and remain static between inputs; Debug/Release QA checks the
   resulting frames.
5. **Does it have a sound, and how long does it play?** The confirmed menu
   action requests event `0x24`; its extracted file is 2.484 seconds including
   the release tail. Shared audio QA verifies decoding and `PlayingState` on
   the default output. Exact-route output loopback was not recorded.
6. **Are the other visible parts of the screen also correct?** The action
   frames keep the command shell, item details, HUD counter, and other item
   cells visible. The no-residue assertion covers every pixel in the 2x4
   inventory region, and the existing QA checks the HUD ammo tile and item
   art alongside it.
7. **Do multiple frames support the interpretation?** Yes for the host
   transition: the test captures action open, weapon 1 unequipped, weapon 2
   equipped and unequipped, and a repeated weapon 1 cycle. Independent frames
   002550/002560/002580/002610/002699 support neighboring inventory states;
   no matching Genesis weapon unequip example was found.
8. **Was old UI cleared before new controls were drawn?** Yes for this
   transition. After each unequip, the complete item-grid region is compared
   pixel-for-pixel with a fresh scene using the same runner record and cursor
   column. This catches stale borders or black residue from the prior equipped
   state; the child action row is closed before the parent grid is compared.

**Code changed:** QA now creates and saves fresh-state comparison frames for
both weapon unequip paths. **Behavior implemented:** the existing ROM-derived
equipment toggle and grid redraw remain unchanged. **Internally checked:**
`shadowrun_ghidra_conversion_qt_field_action_qa` passes in Debug and Release;
the new baseline frames are `05_fresh_unequipped_baseline.bmp` and
`11_fresh_weapon_2_unequipped_baseline.bmp`. **Independently visually
checked:** related inventory states only; there is no matching Genesis weapon
unequip frame. **Reported defect resolved:** the current host output has no
stale black-box residue in these tested states, but exact Genesis appearance
remains unverified without a matching capture.

### Applied review: equipped-weapon reload capacity

1. **Does it look correct?** The new Debug frames
   `19_weapon_2_equipped_full_capacity.bmp`,
   `20_weapon_2_empty_magazine.bmp`, and
   `21_weapon_2_reloaded_to_capacity.bmp` show the live HUD counter changing
   12 -> 00 -> 12. The item action opens on the second weapon column and the
   Clips result replaces the submenu contents.
2. **Does the visible look match the behavioral state?** Yes in the host
   route: when weapon 2 is equipped it receives 12 rounds and the runner has
   two shared clips; the QA then seeds an empty magazine before invoking the
   Clips route, which returns it to 12 rounds and consumes one clip. The ROM
   reads the equipped item capacity from D4540 `+0x24`; the tested weapon row
   bytes are 6 (item 1) and 12 (item 2). ROM `+0x4F` is the shared Universal
   Gun Clips count. Independent per-inventory-weapon magazines are not present
   in this ROM path.
3. **Are the visuals themselves correct?** The visible counter matches the
   extracted 12 and 00 counter tiles, and the equipped weapon icon and command
   panel remain visible. This is an internal comparison against named ROM
   assets; no independent Genesis image shows this weapon-2 reload transition.
4. **Is it animated, and should it be?** No animation is traced for the Clips
   selector, reload confirmation/result, or HUD counter. The frame changes on
   controller input and inventory state updates only.
5. **Does it have a sound, and how long does it play?** Opening and completing
   the route request source event `0x24`, mapped to `menu-024.flac` (2.484 s
   including its release tail). This QA checks the event requests; shared
   audio QA checks decoding, default-output `PlayingState`, duration, and stop.
   Output loopback for this exact reload route remains unrecorded.
6. **Are the other visible parts of the screen also correct?** The action
   parent, selected inventory column, portrait, equipped icon, HUD, and item
   cells remain visible during the route. The existing Clips Cancel check
   compares the returned parent frame with its pre-entry frame to catch stale
   child controls.
7. **Do multiple frames support the interpretation?** The three new frames
   capture equip, empty-magazine, and reload-result states; checkpoints 13-17
   cover the separate weapon-1 prompt/confirmation/result/return route.
   Nearby longplay inventory and HUD frames support their shared layout only;
   none independently shows switching capacity or reloading weapon 2.
8. **Was old UI cleared before new controls were drawn?** The Clips menu
   replaces its parent row, and the existing Cancel assertion requires a
   pixel-identical parent redraw. The new reload result is captured within the
   Clips child after the reload action, with no second control row over it.

**Code changed:** the Qt route QA now equips weapon 2 and exercises its
12-round reload path; the production capacity lookup was already keyed by the
currently equipped item. **Behavior implemented:** weapon capacity comes from
that item's ROM `+0x24` and reserve clips remain runner-wide. **Internally
checked:** Debug and Release `field_action_qa` capture both weapon capacities,
counter pixels, reserve decrement, and source event requests. **Independently
visually checked:** adjacent inventory/HUD layout only; no matching Genesis
reload frame is present. The source-defined active-load percentage can look
different from a model with persistent per-weapon magazines, but adding
independent magazines would contradict this ROM's `+0x4E` transitions.
