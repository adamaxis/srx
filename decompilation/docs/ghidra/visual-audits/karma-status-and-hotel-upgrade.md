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
- `0xD5DC` redraws the focused row using the input text mode: initial/new
  selection is `0x4000` (palette line 2), while movement first restores the
  old row with `0x6000` (palette line 3). Attribute labels/values use the
  `0xC386/0xC3A2` layout; skill labels/values use `0xC686/0xC6A2`. Essence is
  special: focused Essence uses `+0x7E` and `+0x8E` at `0xC4C4/0xC4C8`; focused
  Magic reads `+0x7F`. The static Essence cell is one tile left of the other
  right-column numbers.
- `0xD5DC` also reads the installed-cyberware word at `+0x90`. Its first set
  bit in 5..8 writes the literal `c` marker and the derived Quickness/Strength
  values at `0xC424/0xC428` and `0xC4A4/0xC4A8`; its first set bit in 9..11
  writes the Body marker/value at `0xC3A4/0xC3A8`. These are short row details,
  not prose descriptions. `0xD82A` redraws the selected lower label and
  current Karma; the command view suppresses Hotel's next-value/cost section.
- `gameplay/scripts/attributes_skills.lua` binds the 20 labels and five
  upgrade messages to explicit ordered `attributes_skills.text.option.*`
  keys. Script QA checks all 25 IDs, keys, and resolved ROM strings; the field
  catalog test checks the same source order and values.
- The command-view A action requests sound event `0x24`, mapped to
  `audio.menu.command-024` (2.484 seconds including the release tail). The
  Attributes/Skills child is silent; returning to the command route runs the
  full `0x10F50` rebuild and requests event `0x25` (2.134 seconds including
  its release tail). Hotel selection and the Hotel Karma child are silent.

## Captures and tests

- Command view: `qt_runtime/field-attributes-skills-view-only-qa/actual.bmp`,
  `cell-c0-r0.bmp` through `cell-c1-r9.bmp`, `focused-command-row3.bmp`,
  `frame-30.bmp`, `cyberware-modifiers.bmp`, `cyberware-none.bmp`, and A/B/Start
  return frames. Each image is a full 320x224 frame.
- Hotel spending view: `qt_runtime/field-hotel-qa/karma-spend/actual.bmp`,
  `frame-30.bmp`, `prompt-yes.bmp`, `upgrade-applied.bmp`, and
  `return-options.bmp`.
- Debug and Release pass `shadowrun_ghidra_attributes_skills_menu_runtime`,
  `shadowrun_ghidra_conversion_qt_field_attributes_skills_qa`,
  `shadowrun_ghidra_cyberware_detail_conversion`,
  `shadowrun_ghidra_conversion_qt_field_hotel_qa`, and
  `shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa`.
- The source-rule test checks all 20 cursor/label/value mappings, both Essence
  values, Magic, first-match cyberware modifiers, and the empty-flags case. The
  Qt pixel QA drives all 20 cells with ordinary d-pad input; it verifies the
  selected `#6D4992` label/value, prior-row clearing, Essence's second value,
  conditional cyberware details, read-only runner fields, and clean A/B/Start
  returns. Debug and Release each write 30 matching full-screen frames.
- The pixel comparison requires the shared grid and current-Karma region to
  match and confines the generated-image difference to the lower upgrade/cost
  detail row. The runtime test separately checks that command A returns
  without mutation, Hotel No preserves the record, and Hotel Yes spends Karma
  and raises Body.
- The supplied longplay has no identified capture of either exact page;
  section 06 frame `000090` shows a runner status panel. A separate user-
  supplied `e8_karma_spend.zip` recording, reported as obtained from Grok bot,
  shows the Hotel page with Joshua as
  Human/Shaman, Body 2, Karma 20, and a Body 3 preview costing 3 Karma, then
  the default-Yes confirmation prompt at frame 9080. [C] Its input log records
  a four-frame A press beginning at frame 9081; snapshots from 9040 through
  9160 retain `FFFC04=0x00FF0100`, Body 2 at `+0x77`, and Karma 20 at `+0x8D`,
  with cost/increase both 3. The recording has no applied upgrade. [C] At frame
  9080, `FFDD22=0x4040` and `FFF0DC=0`; at 9081, `FFDD22=0x0040`. ROM `0x19F2`
  maps A to controller bit 6, and `0x195C` stores current buttons in the low
  byte and new press edges in the high byte. The RAM edge at 9080 precedes the
  input-log onset at 9081 by one image. [C] The ROM's grid loop checks A at
  `0xD31E` and calls the prompt at `0xD426`; `0xD53C` then waits at `0xD570`,
  samples at `0xD57C`, and checks for a new A edge at `0xD5B4`. With A still
  held, `0x195C` produces no second high-byte edge, so the prompt remains on
  Yes until a later release-and-press. The captured unchanged Body/Karma and
  visible prompt through frame 9160 are consistent with that source path; the
  earlier interpretation that this was a purchase edge is withdrawn. The
  missing ROM/build identity still prevents treating the recording as verified
  US Genesis footage. Keep it as an external visual comparison. The expanded local copy
  (`frames/`, `inputs.txt`, `ram.npy`, `audio.wav`) is under
  `decompilation/ghidra_conversion/build/research/au/e8_karma_spend/`.

  The Human/Shaman values are consistent with the profile at ROM `0x72B1A`
  (Human race, Shaman class, Body 2), while Karma 20 is runtime state: the
  preset's `+0x8D` Karma byte is zero. The native Hotel route's Human/Samurai
  Body 1/Karma 50 setup is a separate QA fixture, intentionally chosen to
  expose a Body 2 preview costing 2. The profile permutation QA covers the
  source-derived class/race records and limits; neither fixture should be read
  as the only valid page state.

  The generated captures remain internal conversion evidence, not Genesis
  visual goldens.

## Nine verification questions

1. **Does each page look correct?** The source-derived command view shows the
   shared two-column grid, current Karma, focused row, Essence pair, and any
   installed-cyberware details. The Hotel child shows its upgrade target and
   cost. The supplied `e8_karma_spend.zip` provides a broad external
   comparison for the Hotel page. The user reports that Grok bot provided it,
   but the archive has no ROM hash, emulator/build identity, or capture method;
   exact Genesis appearance remains unverified.

2. **Does the visible state match behavior?** Yes in typed and pixel QA. Only
   the currently focused cell uses palette line 2; moving restores the prior
   row to line 3. Its name/value and the source-dependent details follow the
   cursor and active record. Lua's stable IDs and explicit label keys resolve
   to the same source strings without owning focus or state. Command-row focus
   alone does not open the page; A/B/Start return without mutation. Hotel owns
   the preview and spend path.

3. **Are the visuals correct?** Full-screen QA covers all 20 selectable
   cells, the special Essence positions, modifier markers and values, and the
   separate command/Hotel detail rows. The command page shows no upgrade
   preview; the Hotel page shows the selected next value and cost. With equal
   runner fixtures, pixel QA confines their difference to the lower
   upgrade/cost row. Script QA checks each label key and exact ROM copy. This
   checks the conversion against its source-derived layout, not against an
   verified Genesis image. The supplied external comparison is useful for the
   Hotel page's broad layout, but it is not a known Genesis capture.

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

7. **Do multiple frames support the reading?** Internally, yes: all 20
   selected cells, focus without opening, static hold, populated/empty
   cyberware, Hotel prompt, applied upgrade, and parent returns are captured.
   Debug and Release images match. The longplay lacks either exact page. The
   supplied external sequence does show the Hotel page over multiple frames,
   but no purchase result and no identified ROM/emulator build; it provides
   partial visual comparison only.

8. **Are old controls cleared before the new page is drawn?** Yes in the
   route QA. Entry replaces the full parent composition; command return is
   pixel-identical to the pre-page shell; Hotel return rebuilds Options with
   no Attributes/Skills labels or prompt pixels left behind.

9. **Was the complete affected interval reviewed?** Yes for this content
   slice: the 30 command-view BMPs were reviewed in two contact sheets, the
   command and Hotel pages were opened at native size, and the Hotel sequence's
   252 full-screen frames were reviewed in chronological contact sheets. The
   current Debug/Release filename sets and SHA-256 hashes match for both route
   directories.

## Result

- **Code changed:** Lua now owns the command-view page's row/column cursor and
  view-only exit; the Qt presenter mirrors that cursor while connecting the
  existing typed `0xD5DC` detail conversion to the screen. Lua returns explicit
  keys for all 20 labels and five upgrade messages. The presenter paints
  source-selected values, the Essence pair, Magic, and installed-cyberware
  markers and modifiers. Its base composition repaints the full screen before
  the page.
- **Behavior verified:** Lua route input covers all 20 cells; focus does not open
  the page; the view-only route preserves attributes, skills, Karma, and
  cyberware flags; A/B/Start return to a pixel-identical command shell. Hotel
  spending remains a separate typed route.
- **Internal visual/audio verification:** the nine-question review covers all
  30 command-view and 252 Hotel full-screen BMPs. Relevant Debug and Release
  CTests pass for script content, field text catalog, both Attributes/Skills
  routes, Hotel behavior, and screen distinction. The 30 command-view and 252
  Hotel frames match by SHA-256. The page holds static for 30 frames; only the
  command A action requests event `0x24`, the child input is silent, and the
  full command rebuild on return requests event `0x25`.
- **Independent Genesis verification:** unavailable for these exact pages.
