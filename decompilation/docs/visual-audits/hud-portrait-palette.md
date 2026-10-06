# Field HUD Portrait Palette Audit

Date: 2026-10-06
Primary source: US ROM `decompilation/input/shadowrun_genesis_us.bin`.
Source path: `0x3404` field asset setup -> `0x3548` slot loop -> `0x3560`
slot panel refresh -> `0x35D4` portrait stamp / `0x35F0` item stamp ->
`0x3602` item attribute selector -> `0x3648` panel draw -> return at `0x3600`.
Native view: `ghidra_conversion/qt_runtime/src/field_scene_runtime.cpp`; QA:
`shadowrun_ghidra_conversion_qt_field_hud_qa` and
`shadowrun_ghidra_conversion_qt_field_player_action_qa`.

## Source trace

- `0x3404` finishes its field asset setup by calling `0x3548`. The latter
  iterates slot indices 0, 1, and 2, calling `0x3560` once for each slot.
- `0x3560` selects live records `FF0100`, `FF0200`, and `FF0300`. The
  portrait stamp sets D7/D1 to 3/3, so inclusive loops draw a 4x4 image at
  `F1C6`, `F5C6`, or `F9C6`. It loads the palette/attribute word from record
  `+0x2C`; zero selects `0x2000` (palette line 1). The draw enters `0x3648`.
- The item stamp sets D7/D1 to 2/1, so it draws a 3x2 image at `F448`,
  `F848`, or `FC48`. `0x3602` selects its attributes from `+0x57`, signed
  `+0x56`, and signed `+0x9B`; it returns `0x2000` or `0x6000`. Those item
  fields do not select the portrait palette.
- The US-ROM Gator archetype record at `0x72B1A` contains `+0x2C=0x2000`,
  so the live portrait uses palette line 1. Other archetype records use
  `0x2000`, `0x4000`, or `0x6000`, so extracted face families need all three
  palette-line assets. The slot routine returns at `0x3600` to `0x3548`,
  which returns to its caller at `0x3404`.
- No sound call occurs in the traced panel/portrait/item path. The face and
  item stamps are static until a later record/panel refresh; the meters and
  ammo count update through separate routines.

The old Qt state used the item selector's `0x3602` result for the portrait.
The native state now carries the raw `+0x2C` word for the face and computes
the separate `0x3602` result for ordinary and unarmed item art. The offline
importer now emits portrait palette lines 1, 2, and 3 and gives zero-valued
face words the ROM's line-1 fallback.

## Question pass

1. **Does it look correct? Yes, for the reported Gator face-color symptom.**
   I reviewed whole 320x224 independent captures in section 04 frames
   000900/000906/000912 and section 06 frames 000001/000015/000030. The same
   Gator face and HUD position appear in both sections. Their fields and
   adjacent UI differ from the Qt fixture, so they support the portrait color
   comparison, not a claim that the entire fixture matches those scenes.
2. **Does the look match behavioral state? Yes.** The Gator record's
   `+0x2C=0x2000` selects face line 1. QA independently asserts the live
   record word and the chosen named face asset. Item selection remains tied
   to `0x3602`, not the face word.
3. **Are the visuals correct? Yes for palette, size, and stamp placement.**
   The portrait is 4x4 at the slot's source location; ordinary and unarmed
   item art is 3x2 at its separate source location. The tests cover the zero
   fallback and all three face palette lines. Comparing the 32x32 opaque
   Gator face crop against the supplied compressed longplay gives mean
   absolute RGB errors of 17.713 for line 1, 34.191 for line 2, and 37.849
   for line 3 (1024 pixels). This supports line 1 while allowing for video
   compression.
4. **Is it animated? No.** The face and item stamps do not advance an
   animation counter in this path; they remain still between panel refreshes.
   The portrait changes only when the selected slot/record presentation is
   refreshed. Meter and ammo changes are separate state updates.
5. **Is there a sound? No call traced.** No sound request is present in
   `0x3404`, `0x3548`, `0x3560`, `0x35D4`, `0x35F0`, `0x3602`, or `0x3648`.
   There is no HUD-specific sound duration to report for this path.
6. **Are the other visible parts of the screen also correct? Yes for this
   redraw check; overall field fidelity remains partial.** I reviewed every
   generated full-screen HUD frame and all four related player-action frames.
   The playfield and player remain visible; counters, slot order, equipment,
   and the unarmed replacement are legible; no new overlay obscures another
   panel. The inconsistent ammunition report stays open as `HUD-02`, and
   recruited-party appearance still lacks an independent game frame.
7. **Do multiple frames support the interpretation? Yes.** Six independent
   frames cover field movement and Magic browsing/action. The native matrix
   contains three archetypes, three meter states, three party/equipment states,
   and four related player-action states. Debug and Release outputs have
   matching SHA-256 hashes.
8. **Does the new UI replace the old UI cleanly? Yes; no page or button is
   added by this change.** Each capture rebuilds the field frame before
   applying current HUD assets. In the Ricky unarmed state the prior gun image
   is gone and the unarmed hand image occupies the item cell.

## Result

- **Code changed:** face palette state and extracted variants now follow
  `+0x2C`; item/unarmed palettes continue to follow `0x3602`.
- **Behavior implemented:** three face palette lines, the zero-word line-1
  fallback, and the separate item palette selection.
- **Internally checked:** Debug and Release graphics, typed-HUD, field-action,
  and Qt QA, with whole-screen dumps reviewed.
- **Independently visually checked:** section 04 frames 000900/000906/000912
  and section 06 frames 000001/000015/000030.
- **Reported defect resolved:** the reported Gator face-color symptom.
  Recruited-party visual fidelity and the ammunition values in `HUD-02`
  remain open; the full HUD hill remains partial.
