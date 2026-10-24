# Magic User and Non-Magic Screen Split Audit

Date: 2026-10-15
ROM: US Genesis Shadowrun
Scope: Pocket Secretary Magic page, spell-capable route, and non-magic Magical Defense fallback at ROM `0xFBA6`

## Source trace

The raw US ROM is primary. The startup templates begin at `0x7291A`, with a
`0x100` byte stride. Samurai is selection 0, class 2, with Magic resource
byte `+0x7F = 0`. Decker is selection 1, class 3, with `+0x7F = 0`. Gator
Shaman is selection 2, class 1, with `+0x7F = 6`.

At `0xFBA6`, the class/resource condition selects the yellow spell page for
class `<2` when `+0x7F` is nonzero. That route clears Plane A through `0xFC22`
before drawing its spell panel, selected spell, metrics, and cursor. If the
resource byte is zero, the low-class route falls through to Magical Defense;
classes 2 and 3 go directly to that fallback. ROM rebuilds the common menu row
and central content grid before writing the title and two defense bars. The
Qt view keeps the shared outer frame, runner portrait, posture control, command
tab, and right HUD, while replacing the old center status/item details. It
computes physical and mana defense separately and chooses the corresponding
rows from the named `defense-bar-*.bmp` assets.

The conversion already represented these paths as typed native runner state.
This slice corrected its fallback integration fixture: it had used a Gator
Shaman record after forcing `+0x7F` to zero, which tested the zero-resource
branch but did not verify a non-magic character. The fallback QA now uses
Samurai's unmodified startup record. The spell QA continues to use Gator's
unmodified record.

## Nine-question review

1. **Does it look correct? — Unknown for the complete split.** The independent
   longplay has matching magic-user screens, but no Samurai/Decker Defense
   page in sections 01-41. Sections 08-41 were sampled every second with a title
   locator and manually checked at the top candidate frames; that bounded search
   is not proof that no brief or differently rendered Defense page occurs. The internal Samurai frame now shows the `Magical Defense` title and
   two distinct strips on a cleared center panel. This corrects the prior Qt
   capture, where model, weapon, and Atk/Def details remained behind the
   fallback. It is not independent proof of Genesis appearance. The full-screen Qt images
   are `field-magic-nonmagic-qa/actual.bmp` and
   `field-magic-spell-qa/actual.bmp`.
2. **Does the visible look match the behavioral state? — Yes for tested
   records.** The Samurai page begins from class 2/`+0x7F=0`; A opens
   Magical Defense and does not open the spell interface. Gator Shaman begins
   from class 1/`+0x7F=6` and takes the spell route. The conversion test also
   covers class 1 with zero Magic resource and classes 2/3 with a nonzero
   resource byte.
3. **Are the visuals themselves correct? — Unknown for Genesis fidelity of
   the non-magic fallback.** Source tracing and Debug/Release QA verify the
   separate values, bar rows, positions, colors/assets, and center-panel
   redraw.
   Independent section 06 frames support the yellow spell panel's placement
   and content. There is no independent Defense reference to check its exact
   colors, text raster, or pixel placement.
4. **Should anything animate? — Yes, the spell selector only.** The spell
   cursor has four phases at seven-frame intervals while its menu is active.
   The force/posture cursor uses the same tested cadence. Magical Defense
   starts no local animation; its full-screen `actual.bmp` and `held.bmp`
   images are identical after 30 idle frames. The supplied longplay is sampled
   at 30 fps, so it does not independently establish the ROM's seven-VBlank
   cursor cadence. No independent non-magic idle sequence was found.
5. **Does it have sound? — Yes, the parent-page entry requests event `0x24`.**
   The extracted `menu-024.flac` metadata reports 2.484 seconds. The fallback
   itself adds no sound request; the spell action loop also has no new call
   traced. Shared Debug/Release audio QA covers resource decoding and player
   state, but output loopback for this route was not measured, so the audible
   duration on the configured Windows device is unverified.
6. **Are the other visible parts of the screen also correct? — Yes in the
   reviewed internal captures.** All 25 generated checkpoints were reviewed
   in a contact sheet, with parent, open, hold, return, and selected spell
   states also inspected full-size. The shared portrait, inventory, posture
   strip, outer frame, and right HUD remain present. The yellow page replaces
   the old center content; the Defense fallback clears the old status/item
   content before drawing its title and bars. No stale spell text or unrelated
   selector is visible after entry or return. The Samurai fallback test now
   compares the entire frame outside the replaced center against the parent
   page, then checks return, reopen, and a second return. This specifically
   guards the shared posture/action row and HUD against defense-panel overdraw.
   ROM `0x11900`/`0x11928` rebuild that common row, so it remains visible by
   design.
7. **Do multiple frames support the interpretation? — Unknown for the full
   split; yes for the spell route.** Independent section 06 frames
   `000001`/`000002` show Rockskin, `000005` shows Mana Zap,
   `000015`/`000020` show force/posture selection, and `000030` shows the
   equipped HUD icon. The non-magic route has internal parent/open/hold/return
   screenshots only, with no matching independent gameplay frame.
8. **Does the new UI replace the old UI cleanly? — Yes in tested states.**
   The spell route follows the source plane clear and leaves no old command
   center text or buttons beneath the yellow panel. Magical Defense rebuilds
   the center content grid before writing its title and bars; the Qt route now
   redraws that panel from the named background so old model, weapon, and
   Atk/Def details cannot remain underneath. The ROM's 16-cell title includes
   a leading blank cell, which is also cleared. The outer frame and right HUD
   remain. Its 30-frame hold is unchanged, and return restores the previous
   command page without stale Defense bars.
9. **Was the whole affected time window reviewed? — No, not as a continuous
   capture.** The complete generated checkpoint set was reviewed in the
   contact sheet; Samurai parent, Defense, 30-frame hold, return, Gator
   parent, spell display/browse, cursor phases, force/posture edits, equipped
   item, return, and reopen were inspected as checkpoints, with representative
   states opened full-size. The dumper records transition checkpoints, not
   every frame of a continuous route clip. The user-provided longplay
   reference frames listed above were each inspected in full.

## Evidence and disposition

- **Code changed:** Lua now owns the live Magic, force, and posture route and
  its typed metrics. Qt redraws the ROM-cleared center panel before painting
  Magical Defense. `magic.lua` supplies explicit ordered Lua keys for all
  14 spell names, all 14 descriptions, and seven panel text entries. The
  field text catalog resolves descriptions from those script bindings and
  retains the exact ROM copy. Barrier is the new 14th row; its ROM text,
  spell-page icon, and two equipped HUD palette variants are extracted, but
  it has not had independent screenshot review. Runtime source-title drawing now clears the
  16-cell title row before painting, matching the ROM's blank-cell overwrite.
  Magic fallback QA uses the genuine Samurai startup record and asserts class
  2, `+0x7F=0`, fallback state, two source values/bar assets, cleared old
  model/weapon/Atk/Def content, a black leading title cell, persistent HUD,
  full-frame equality outside the center panel, and a return/reopen/return
  cycle. A conversion unit case checks that class 3 still falls back
  with a nonzero resource byte.
- **Behavior implemented:** the live Lua route distinguishes the
  yellow spell page from the two-bar Magical Defense page using runner class
  and Magic resource state.
- **Internally checked:** Debug and Release builds and all five focused CTests
  passed in each configuration, including
  `shadowrun_gameplay_script_runtime_qa`,
  `shadowrun_ghidra_field_menu_text_catalog`,
  `shadowrun_ghidra_conversion_qt_field_magic_qa`,
  `shadowrun_ghidra_conversion_qt_field_magic_spell_qa`, and
  `shadowrun_ghidra_magic_menu_conversion`. All 25 Debug BMP checkpoints (four
  non-magic and 21 spell-route frames) were reviewed in contact sheets;
  representative full-screen parent/open/hold/return and spell/action frames
  were opened at original resolution. Title and defense-strip close-ups were
  inspected. SHA-256 hashes match every
  corresponding Release BMP. Dumps are under
  `decompilation/ghidra_conversion/build/conversion/qt/debug/qt_runtime/`
  and `decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/`.
- **Independently visually checked:**
  `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/`
  frames `000001`, `000002`, `000005`, `000015`, `000020`, and `000030`
  support the magic-user spell and action views. No corresponding non-magic
  Defense screen was found in the reviewed and sampled sections 01-41.
- **Retained comparison artifacts:** the Oct. 9 full-screen contact sheet and
  close-ups in the Debug dump directory were inspected. The older Oct. 4 zooms
  and contact sheet in the Debug dump directory were also inspected. Their
  Qt/reference upper crops show different selected spells (Mana Zap vs.
  Rockskin), so they are
  not a matched comparison and are not used as visual proof. The Oct. 7
  full-screen Qt spell states were compared with the corresponding section 06
  frames instead.
- **Defect status:** partial. The non-magic QA now uses a genuine non-magic
  character, and both branches pass their internal state and screen checks.
  Do not call the Defense fallback independently visually resolved until a
  matching real-game capture is available. Spell target/effect routing and
  route-level output-device audio capture remain open.
