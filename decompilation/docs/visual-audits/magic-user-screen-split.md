# Magic User and Non-Magic Screen Split Audit

Date: 2026-10-07
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
classes 2 and 3 go directly to that fallback. The fallback retains the shared
command/status page and overlays its title and two defense bars. It computes
physical and mana defense separately and chooses the corresponding rows from
the named `defense-bar-*.bmp` assets.

The conversion already represented these paths as typed native runner state.
This slice corrected its fallback integration fixture: it had used a Gator
Shaman record after forcing `+0x7F` to zero, which tested the zero-resource
branch but did not verify a non-magic character. The fallback QA now uses
Samurai's unmodified startup record. The spell QA continues to use Gator's
unmodified record.

## Nine-question review

1. **Does it look correct? — Unknown for the complete split.** The independent
   longplay has matching magic-user screens, but no Samurai/Decker Defense
   page. The reviewed internal Samurai frame has the two-bar overlay described
   by the user and the ROM; it is not independent proof of Genesis appearance.
   The full-screen Qt images are `field-magic-nonmagic-qa/actual.bmp` and
   `field-magic-spell-qa/actual.bmp`.
2. **Does the visible look match the behavioral state? — Yes for tested
   records.** The Samurai page begins from class 2/`+0x7F=0`; A opens
   Magical Defense and does not open the spell interface. Gator Shaman begins
   from class 1/`+0x7F=6` and takes the spell route. The conversion test also
   covers class 1 with zero Magic resource and classes 2/3 with a nonzero
   resource byte.
3. **Are the visuals themselves correct? — Unknown for Genesis fidelity of
   the non-magic fallback.** Source tracing and Debug/Release QA verify the
   separate values, bar rows, positions, colors/assets, and named panel draw.
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
   reviewed internal captures.** All 25 generated BMPs were viewed at full
   screen. The shared portrait, inventory, posture strip, outer frame, and
   right HUD remain present. The yellow page replaces the old center content;
   the Defense fallback updates its center area over the retained command
   page. No stale spell text or unrelated selector is visible after entry or
   return.
7. **Do multiple frames support the interpretation? — Unknown for the full
   split; yes for the spell route.** Independent section 06 frames
   `000001`/`000002` show Rockskin, `000005` shows Mana Zap,
   `000015`/`000020` show force/posture selection, and `000030` shows the
   equipped HUD icon. The non-magic route has internal parent/open/hold/return
   screenshots only, with no matching independent gameplay frame.
8. **Does the new UI replace the old UI cleanly? — Yes in tested states.**
   The spell route follows the source plane clear and leaves no old command
   center text or buttons beneath the yellow panel. Magical Defense preserves
   the shared command/status page as the ROM does and replaces its center
   content. Its 30-frame hold is unchanged, and return restores the previous
   command page without stale Defense bars.
9. **Was the whole affected time window reviewed? — No, not as a continuous
   capture.** The complete generated state set was reviewed full-screen:
   Samurai parent, Defense, 30-frame hold, return; and Gator parent, spell
   display/browse, four selector phases, force/posture selection and edits,
   equipped item, return, and reopen. The dumper records transition
   checkpoints, not every frame of a continuous route clip. The user-provided
   longplay reference frames listed above were each inspected in full.

## Evidence and disposition

- **Code changed:** the Magic fallback CTest now selects the real Samurai
  startup record and asserts class 2, `+0x7F=0`, fallback state, two source
  values/bar assets, persistent HUD, and return. A conversion unit case checks
  that class 3 still falls back with a nonzero resource byte. No runtime menu
  branch was changed in this slice.
- **Behavior implemented:** the existing typed route distinguishes the
  yellow spell page from the two-bar Magical Defense page using runner class
  and Magic resource state.
- **Internally checked:** Debug and Release builds and CTests
  `shadowrun_ghidra_conversion_qt_field_magic_qa`,
  `shadowrun_ghidra_conversion_qt_field_magic_spell_qa`, and
  `shadowrun_ghidra_magic_menu_conversion` passed. The four Debug non-magic
  BMPs and 21 Debug spell BMPs were reviewed; SHA-256 hashes match every
  corresponding Release BMP. Dumps are under
  `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/`
  and `build/conversion/qt/release/qt_runtime/`.
- **Independently visually checked:**
  `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/`
  frames `000001`, `000002`, `000005`, `000015`, `000020`, and `000030`
  support the magic-user spell and action views. No corresponding non-magic
  Defense screen was found.
- **Retained comparison artifacts:** the older Oct. 4 zooms and contact sheet
  in the Debug dump directory were also inspected. Their Qt/reference upper
  crops show different selected spells (Mana Zap vs. Rockskin), so they are
  not a matched comparison and are not used as visual proof. The Oct. 7
  full-screen Qt spell states were compared with the corresponding section 06
  frames instead.
- **Defect status:** partial. The non-magic QA now uses a genuine non-magic
  character, and both branches pass their internal state and screen checks.
  Do not call the Defense fallback independently visually resolved until a
  matching real-game capture is available. Spell target/effect routing and
  route-level output-device audio capture remain open.
