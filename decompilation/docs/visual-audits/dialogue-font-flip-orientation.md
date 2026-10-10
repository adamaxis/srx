# Dialogue font flip orientation

## ROM trace

The Stoker arrival label and manager stage directions already contain the
correct text. The field font atlas was the defect: ROM character-table words
carry per-glyph tile flip flags, but the offline atlas decoder used the tile
index and ignored those flags.

The field font is uploaded from `0xC1064` to VRAM `0xB560`; the shared text
writer at `0xC0DA` resolves a character through the table at `0xC1320` and
applies the name-word tile and flip bits. In the US ROM:

- `<` at table word `0xC1358` is `0x05C6` (tile `0x5C6`, no flip).
- `>` at table word `0xC135C` is `0x0DC6` (the same tile with HFlip).
- `(` at table word `0xC1330` is `0x05B3`.
- `)` at table word `0xC1332` is `0x0DB3` (the same tile with HFlip).

This accounts for both the inward-facing `< MORE <` in the old Stoker capture
and reversed closing parentheses in any text that uses them. The current Lua
text remains `< MORE >`; no copy correction is needed.

## Change and checks

`decode_genesis_field_menu_font_with_palette()` now applies the HFlip and
VFlip bits while arranging each named glyph in the host atlas. The explicit
offline import command `shadowrun_graphics_extract --all-assets` regenerated
the local, ignored `assets/menu/field/font.bmp` from the US ROM. The resulting
right delimiter and closing parenthesis are horizontally mirrored source
glyphs; individual runtime screens still consume the named BMP directly.

The `shadowrun_graphics` CTest asserts the ROM-derived `<`/`>` and `(`/`)`
mirror pairs. Debug and Release `shadowrun_ghidra_conversion_qt_startup_qa`
write the live Stoker arrival and manager pages. The 320x224
`opening_stoker_arrival.bmp` matches independent section 03 frame `000090` in
portrait, panel, text, button marker, and outward `< MORE >` glyph direction.
The `opening_stoker_package_offer.bmp` manager line matches independent
section 03 frame `000300`, including the right-facing closing angle glyph.

## Nine-question visual verification

1. **Does it look correct? Yes.** The corrected arrival page matches the
   independent section 03 frame `000090`; the player video shows `< MORE >`.
2. **Does the visible look match the behavioral state? Yes.** The Lua arrival
   page has one A continuation and resolves the text key `stoker.motel.arrival.more`.
   The string contains a left and a right delimiter, and the ROM table assigns
   the right-facing orientation.
3. **Are the visuals correct? Yes for the affected glyphs.** Full-screen
   comparison checked the portrait, frame, green text, marker, and delimiters;
   the extractor preserves the source table's HFlip and VFlip attributes.
4. **Should anything animate? No.** The arrival page is static while waiting
   for A. The correction changes a glyph's orientation, not animation state.
5. **Does it have sound? No call traced for glyph rendering.** This change
   adds no sound or duration claim; dialogue audio remains a separate route.
6. **Are the other visible parts correct? Yes for the reviewed page.** The
   whole screen was inspected; the dialogue panel replaces the field HUD and
   the Stoker portrait/text are not covered by stale field controls.
7. **Do multiple frames support the interpretation? Yes for the screen
   family.** Independent section 03 frames `000090` and `000300` show the
   arrival and manager pages. They do not expose controller state.
8. **Does the new UI replace the old UI cleanly? Yes.** The shared presenter
   clears the full screen and both dialogue wells before painting the panel;
   the route's full-screen arrival frame contains no underlying field buttons
   or HUD.
9. **Was the whole time window reviewed? Partial.** The complete available
   New Game intro capture is reviewed separately, and the skip QA follows its
   field handoff through the Stoker arrival. No independent recording of a
   Start-pressed skip was found, so exact skip input timing is not externally
   confirmed.

## Outcome

- **Code changed:** the ROM-backed offline field-font importer now applies
  source glyph flips.
- **Behavior implemented:** right angle brackets and closing parentheses
  render with the orientation selected by the ROM character table.
- **Internally checked:** `shadowrun_graphics` plus the Debug and Release Qt
  startup QA; full-screen arrival and manager BMPs inspected.
- **Independently visually checked:** longplay section 03 frames `000090` and
  `000300`; these support glyph direction and page appearance, not input state.
- **Reported defect resolved:** the reversed `< MORE >` delimiter and affected
  closing-parenthesis glyphs are corrected in the regenerated local asset.
