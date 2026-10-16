# Magic Spell Action Visual Review

## Scope and evidence

This review covers the observed Mana Zap path from the in-game command menu:
spell browsing, A into force/posture selection, force and posture changes, B
to equip, and return. It does not establish every spell, archetype, or target
effect as visually complete.

Independent reference frames are in
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/`:

- `frame_000001.png`, `frame_000002.png`, and `frame_000005.png`: spell-panel
  browsing and selection.
- `frame_000015.png` and `frame_000020.png`: the Set Force & Posture view.
- `frame_000030.png`: the equipped Mana Zap item in the right HUD.

The review used the full 320x224 frames, with attention to the command panel
at x=24..240, y=16..160 and the HUD item at x=288..312, y=64..80. The longplay
frames are sampled at 30 fps and compressed; they support visual comparison,
not pixel-exact timing or a lossless golden.

ROM trace: `0xFF5A` enters force selection, `0x102D4` replaces the spell
description with the action heading, `0x10016` runs the force loop, and
`0x10722` handles posture. Finalization publishes the active spell to +0x56
and +0x57. `0x3404` uses the D4EA0 row's six tile IDs for negative +0x56;
`0x3602` chooses palette attributes 0x2000 or 0x6000. Ghidra's disassembly
was used only as a cross-check against the ROM bytes.

## Checklist

1. **Does it look correct? YES for the captured Mana Zap route.** The action
   heading is at the source position and the equipped blue/purple HUD glyph
   visually matches frame 000030. This is not a claim for unobserved spells
   or archetypes.
2. **Does the look match the behavioral state? YES.** A opens the action
   state and changes the displayed text; force/posture input changes typed
   runner state; B publishes spell code 8 (D4EA0 row 7, Mana Zap) with slot
   -1. The HUD shows that spell row after equipment finalizes.
3. **Are the visuals correct? YES for this observed route; UNKNOWN for the
   unobserved catalog.** The 24x16 art is decoded from the ROM's six D4EA0 tile
   entries, including tile flips, and the tested class selects palette 3 from
   the source 0x6000 attribute. The reference is compressed, so no exact
   pixel-golden claim is made.
4. **Is it animated, and when? YES.** ROM 0x10016 calls the cursor service in
   its frame loop. The typed cursor advances every seven field frames; Qt QA
   captures all four visible phases and verifies the cycle while force stays
   unchanged. The two independent action frames show sampled cursor states;
   ROM counters, not the 30 fps clip, establish cadence.
5. **Is there a sound, and how long does it play? NO new action sound call was
   traced for A, force/posture changes, or B.** The menu-opening event 0x24 is
   separate and has 2.484 seconds of file metadata. Actual device playback
   duration and release behavior are UNKNOWN. QA verifies that the action
   loop adds no audio event.
6. **Are the other visible parts correct? YES for the reviewed frames.** The
   Success, Damage, Drain, Defense, Force, and Posture rows, spell icons,
   lower inventory cells, portrait, HUD chrome, ammo counter, and old active
   item were reviewed. The old item remains during selection and changes only
   after B; the broader field HUD meter projection remains outside this check.
7. **Do multiple frames support this reading? YES.** Browsing, selection,
   action, and equipped states are visible in six separate longplay frames
   listed above. Qt QA also writes browsing, four cursor phases, force and
   posture changes, equip, and return images.
8. **Does the new UI replace the old UI cleanly? YES for this route.** The
   Magic panel replaces the command center; the action heading replaces and
   clears the spell description. The prior active item remains until the
   action commits, then the spell art replaces it in the HUD. QA checks the
   description is absent during action, retained HUD layers, and clean return.

## Internal verification

`shadowrun_ghidra_field_hud` checks negative equipped-slot spell selection,
the +0x57 row index, and both 0x3602 palette variants. The Qt Magic spell QA
checks action text replacement, the full four-phase cursor cycle, force and
posture limits, no action sound, and the selected D4EA0 HUD sprite. Debug and
Release runs write `force_selection.bmp`, `force_cursor_phase_*.bmp`,
`force_lowered.bmp`, `force_raised.bmp`, `posture_selection.bmp`,
`posture_raised.bmp`, `spell_equipped.bmp`, and return frames under
`qt_runtime/field-magic-spell-qa/`. Debug and Release `shadowrun_graphics`
tests check all 13 extracted spell rows in both palette variants, transparency,
dimensions, and asset-file round-trip. These are conversion regression
artifacts, not independent Genesis goldens. Barrier (spell code 13) and its
two HUD palette variants were added after those QA captures and have not been
reviewed in a full-screen frame or compared with an independent Genesis image.
