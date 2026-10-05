# In-game Karma status and Hotel Karma upgrade pages

The two pages reuse the Attributes/Skills grid artwork and have separate
routes, typed runtime instances, input rules, and lower detail text. The
command-menu page reports current Karma. The Hotel child previews a selected
upgrade and its cost, then can apply it. Treating the two as one menu would
give the command view a spending path it does not have.

## ROM and runtime evidence

- The command overview opens the Attributes/Skills page at `0xD01C`, writes
  `FFF116=-1`, and enters the common screen setup. `0xD33A` sees the negative
  marker and takes the view-only return path at `0xD4A0`. A/B/Start leave the
  page; they do not write the runner's attributes or Karma.
- Hotel option 0 dispatches through `0xC14C8` to `0xD024`, past the command
  route's negative-marker write. The live selection remains available to
  `0xD33A`; `0xD3EA` checks the `+0x8D` Karma balance and applies the selected
  stat change, while `0xD53C` owns the Yes/No prompt. The parent Hotel menu is
  suspended while this child is open.
- `0xD82A` supplies the shared Attributes/Skills grid. The fixed native types
  `CommandMenuKarmaViewRuntime` and `HotelKarmaUpgradeRuntime` wrap separate
  instances of that controller. Compile-time and runtime QA require the
  command type to reject upgrades and the Hotel type to own the upgrade flow.
- The command-view A action requests sound event `0x24`, mapped to
  `audio.menu.command-024` (2.484 seconds including the release tail). The
  Attributes/Skills child is silent. Hotel selection and the Hotel Karma
  child are also silent in the converted route and QA.

## Captures and tests

- Command view: `qt_runtime/field-attributes-skills-view-only-qa/actual.bmp`,
  `selection-moved.bmp`, `frame-30.bmp`, and return frames.
- Hotel spending view: `qt_runtime/field-hotel-qa/karma-spend/actual.bmp`,
  `frame-30.bmp`, `prompt-yes.bmp`, `upgrade-applied.bmp`, and
  `return-options.bmp`.
- Debug and Release pass `shadowrun_ghidra_attributes_skills_menu_runtime`,
  `shadowrun_ghidra_conversion_qt_field_attributes_skills_qa`,
  `shadowrun_ghidra_conversion_qt_field_hotel_qa`, and
  `shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa`.
- The pixel comparison requires the shared grid and current-Karma region to
  match and confines the generated-image difference to the lower upgrade/cost
  detail row. The runtime test separately checks that command A returns
  without mutation, Hotel No preserves the record, and Hotel Yes spends Karma
  and raises Body.
- The supplied longplay has no identified independent Genesis capture of
  either exact page. Section 06 frame `000090` shows a runner status panel, not
  either Attributes/Skills Karma page. The generated captures are internal
  conversion evidence, not Genesis visual goldens.

## Eight verification questions

1. **Does each page look correct?** Internally, the command view shows current
   Karma and the Hotel child shows an upgrade target and cost over the shared
   Attributes/Skills panel. No matching independent Genesis frames were
   identified, so exact Genesis appearance remains unverified.

2. **Does the visible state match behavior?** Yes in the typed runtime tests.
   Moving the command cursor only changes selection; A/B/Start return without
   mutation. The Hotel route can open the default-Yes prompt, decline without
   changes, or confirm the selected stat and Karma cost. A menu-row focus
   change alone never opens either page or the upgrade prompt.

3. **Are the visuals correct?** The generated views keep the same full-screen
   grid and current runner values. The command page shows no upgrade preview;
   the Hotel page shows the selected next value and cost. Pixel QA confines
   their generated-image difference to that lower detail row. This checks the
   conversion against its source-derived layout, not against an independent
   Genesis image.

4. **Should either page animate?** No page animation is indicated by these
   source paths. The command view stays pixel-identical for 30 advanced frames;
   the Hotel child also has a static 30-frame capture. It suspends the Hotel
   parent's `E660` animation, which resumes on return to Hotel Options.

5. **What sound accompanies each page?** The command-menu A action that opens
   the view requests event `0x24` (`audio.menu.command-024`, 2.484 seconds).
   The view-only child itself is silent. The Hotel selection and spending
   child produce no sound in the traced route or current QA. The shared audio
   CTest checks mapped-resource decoding and playback; Windows loopback for
   these exact menu routes has not been recorded.

6. **Are the other visible parts of the screen correct?** The captures check
   the entire 320x224 frame. The command page replaces the command shell; the
   Hotel page replaces Hotel controls. Runner values remain bound to the
   active record, and the shared grid stays in place while its detail line
   reflects the active route.

7. **Do multiple frames support the reading?** Internally, yes: cursor focus,
   row movement, static hold, Hotel prompt, applied upgrade, and both parent
   returns are captured. The longplay lacks a matching multi-frame sequence
   for either exact page, so it does not provide independent visual support.

8. **Are old controls cleared before the new page is drawn?** Yes in the
   route QA. Entry replaces the full parent composition; command return is
   pixel-identical to the pre-page shell; Hotel return rebuilds Options with
   no Attributes/Skills labels or prompt pixels left behind.

## Result

- **Code changed:** the command-view QA now saves the already-checked static
  frame-30 image so both routes have a direct still-frame checkpoint.
- **Behavior verified:** the two routes remain separate and the command view
  cannot spend Karma; the Hotel child owns preview, confirmation, mutation,
  and return.
- **Internal visual/audio verification:** Debug and Release route and runtime
  tests pass; each page is static and cleanly replaces/restores its parent.
- **Independent Genesis verification:** unavailable for these exact pages.
