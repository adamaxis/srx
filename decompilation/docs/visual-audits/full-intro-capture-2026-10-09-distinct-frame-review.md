# Full intro capture review: 2026-10-09 Release recapture

## Capture and review scope

The current Release executable was run through a fresh Gator Shaman New Game
route with 2,700 title-idle updates, the full intro, the opening field event,
and the first stable field/dialogue state:

```text
shadowrun_ghidra_conversion_qt_qa.exe --assets decompilation/assets \
  --full-intro-capture 2 2700 build/visual-review/intro-full-capture-20261009
```

The process exited normally after writing 6,782 consecutive 320x224 PNGs,
`frames.tsv`, and `boundaries.tsv` under
`build/visual-review/intro-full-capture-20261009/`. The run reaches the
Stoker encounter through the Lua-authored opening event, then holds the
resulting dialogue screen.

Every captured PNG was inventoried and hashed in chronological order. The
current run contains 2,032 distinct full-screen images; exact repeated images
are enumerated in the per-frame trace and the contact-sheet manifest. The
older 2026-10-06 run contains 2,012 distinct images. The current-state set has
1,563 images also present in the older run and 469 current images absent from
it. Chronological sheets under
`build/visual-review/intro-all-distinct-frames/page-00.png` through `page-31.png`
contain all 2,012 old-run states; sheets under
`build/visual-review/intro-newly-distinct-states/page-00.png` through
`page-29.png` contain the 469 current-only states. Together they cover every
distinct image in the current capture, with 449 additional old-only images
preserved for comparison. Each sheet uses complete frames; no crops replace
the screen-wide review. Key transitions and the reported defect frames were
also inspected as native-size 320x224 images.

The capture was compared frame by frame to
`build/visual-audits/intro-full/release-gator-shaman-20261006-current/`.
The captures have the same 6,782 frame names. Differences occur in frames
4296-4379, 4392-4793, 6480-6661, and 6663-6781. The newer field handoff and
event route are present in the current run. This recapture is the current
whole-screen evidence; older notes that describe only the 106 sampled overview
sheets are historical and are superseded for this pass.

## Findings

- The ambush patch at current frame `004446` has no solid rectangular border.
  The underlying scene remains visible through transparent cells. The center
  attacker/explosion composition is more complete than in the older capture
  and is consistent with independent section 02 frame `000644`.
- The paired spell sprites stop at the attacker's hit transition and do not
  return in the later smoke or cleanup states. Current frame `004650` shows
  Michael alone at the left; the stale Gator figure present in the older run
  is absent. Independent section 02 frame `000780` supports the cleared
  composition.
- The intro returns to the real scene-2 field. At current frame `006663` the
  player is on the street; the event transition begins at `006691`, and the
  generic Stoker dialogue is open at `006696`. Frame `006781` remains stable
  on that page. Current frame `006696` was compared full-screen with
  independent section 03 frame `000090`; the portrait, text panel, line
  placement, and `< MORE >` marker match. The independent recording supports
  appearance and sequence, not the native route's input or event bytes.
- Full-screen page transitions were checked for stale overlays. The field
  HUD/map are replaced by the dialogue panel, and no prior menu buttons,
  selector, or field graphics remain drawn over the new page.
- The capture includes a cab-map scene at current frame `006480`. Its full
  screen was reviewed as part of the distinct-state sheets. This note does not
  claim a pixel-perfect Genesis match for that scene.

## Nine-question visual review

1. **Does it look correct?** The ambush and Stoker frames agree with the
   identified independent section 02/03 references. Other intro states were
   reviewed across every distinct full-screen image, but this run is not a
   Genesis pixel golden.
2. **Does the visible state match behavior?** The paired spell sprites are
   present before impact and absent at and after the hit; the field transition
   reaches the Stoker dialogue through the map-event route. `frames.tsv` and
   `boundaries.tsv` record the typed phase and route progression.
3. **Are the visuals correct?** The explosion patch keeps transparent cells,
   the hit/cleanup layers remain visible in order, and the Stoker dialogue
   shows its portrait and text within the shared panel. No new asset change was
   made during this review.
4. **Should anything animate?** The intro counters and transition phases are
   covered by the existing ROM-trace audits. This complete capture confirms
   the spell pair remains stopped through cleanup and the destination reaches
   a stable hold. PNGs do not measure audio/video timing independently.
5. **Does it have sound?** This image capture has no audio track. Ambush event
   resources and field-event sound boundaries remain covered only by their
   separate ROM/audio audits; no sound-duration claim is made here.
6. **Are the other parts of the screen correct?** Every reviewed image is the
   complete 320x224 screen. The sweep includes the map, HUD, actors, story
   text, menus, transitions, and destination state, not only the modified
   explosion area.
7. **Do multiple frames support the interpretation?** Yes. Active spell
   phases, hit, smoke, cleanup, field handoff, and Stoker dialogue are each
   present in multiple consecutive frames. Independent section 02/03 frames
   provide additional visual support for the explosion, cleanup, and dialogue.
8. **Are old controls or graphics left beneath the new UI?** The full-screen
   handoff frames show the field/HUD replaced by the dialogue panel. No stale
   controls or doubled panel are visible.
9. **Was the whole time window reviewed?** The full 6,782-frame run was
   captured and indexed. All 2,032 distinct full-screen images were scanned in
   chronological sheets, and exact duplicates were counted from hashes and
   per-frame metadata rather than treated as new visual states. The process
   completed the field handoff and stable dialogue hold without stalling or
   crashing.

## Verification and limits

The following CTests pass in both Debug and Release:

- `shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa`
- `shadowrun_ghidra_conversion_qt_opening_death_layer_qa`
- `shadowrun_ghidra_conversion_qt_startup_integration_qa`
- `shadowrun_ghidra_conversion_qt_field_scene2_motel_street_collision_qa`
- `shadowrun_ghidra_conversion_qt_field_attributes_skills_qa`
- `shadowrun_gameplay_script_runtime_qa`
- `shadowrun_ghidra_field_menu_text_catalog`
- `shadowrun_ghidra_conversion_qt_field_command_qa`

The Scene 2 collision route reaches the motel frontage, settles while holding
Up against the source wall, then moves away with Up+Left and retreats South.
This targeted route does not reproduce a permanent lock or show a mismatch in
the traced ROM collision result. It cannot resolve the user's stuck report
without the exact input sequence or a matching independent Genesis collision
recording; FIELD-02 remains partial.

The Start-menu Attributes/Skills QA again confirms that cursor focus does not
open the Karma page. A/B/Start behavior and the separate Hotel Karma-spending
screen remain distinct.

No intro-renderer code changed during this recapture review. A separate menu
slice moves the command-root choice IDs and labels to Lua; its command-screen
and script-runtime checks are listed above. This evidence does not close
unrelated intro, title, HUD, or general field defects.
