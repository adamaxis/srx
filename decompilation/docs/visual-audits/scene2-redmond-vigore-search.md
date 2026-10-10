# Scene 2: Redmond Vigore search

Status: partial. The event path runs from ordinary field movement through its
three ROM text records and award, but the lead dialogue that sets its
prerequisite is not yet reachable through the live game. The UI capture is
from the native renderer; no matching independent Genesis capture has been
identified, so this audit does not claim Genesis visual fidelity.

## Source and route evidence

- The Redmond Barrens map at Mike's RPG Center labels location 9 Rat's Nest in
  the southwest, separate from location 6 Stoker's Coffin Motel. This is a
  secondary name crosswalk; the ROM does not store those place names.
- Scene 2 record 97 at `0x173174` is event `0x07`, with bounds
  `x=0x01A0..0x01BF`, `y=0x0440..0x045F`. Record 96 is also event `0x07` and
  reaches the same service, so the guide's label cannot distinguish the two
  records by itself.
- Indexed title service 22 at `0x545B6` runs before generic route selection.
  It requires scene 2/event 7, alternate result 0, `FFFC34` bit 13 set, and
  consumed bit 10 clear. It consumes the prerequisite and related service
  bits, sets bit 10, marks the Landmark Map group item, awards 2 Karma to each
  of the three runner records, clears the source cleanup bits, and sets
  `D6=-1` so the generic event-table destination does not load scene 26.
- The three narrative records are ROM `0x117193`, `0x117266`, and `0x117314`.
  The upstream command-menu option at `0x1045B0` is gated by `FFFC1E` bit 26;
  action `0xE7` / handler `0x249A4` writes the bit-13 prerequisite. The live
  bit-26 producer and dialogue option route remain unconnected.
- QA restores a typed Scene 2 field checkpoint at `(0x1B00, 0x4600)` and uses
  ordinary Up input. The collision service opens the Lua event after 18
  frames. The test deliberately seeds bit 13; it does not claim the player
  can reach the encounter through the unfinished lead dialogue.

## Seven visual-inspection questions

1. **Does it look correct?** The three pages render as a full-screen field
   dialogue presentation with the ROM narrative in source order. Page 0 and 1
   show the continuation caption; page 2 has no continuation control. The
   field is fully replaced while the page is open. Their composition is
   internally coherent, but exact Genesis appearance remains unverified
   without an independent Rat's Nest capture.
2. **Does the look reflect behavioral state?** The event only opens when the
   typed Lua facts report the ROM prerequisite pending and not consumed. This
   QA seeds that prerequisite explicitly. After page 2, it verifies the
   consumed flag, completed story fact, Landmark Map item, and +2 Karma on
   all three runner records. The required menu option that normally sets the
   prerequisite still needs wiring.
3. **Are the visuals correct?** The displayed copy comes from the three ROM
   records, including empty `0x80` separators, and is projected through the
   generic field dialogue panel. The panel is an environmental narration
   page with no portrait and no choice rows; the first two pages have only the
   shared MORE continuation. This confirms content and internal layout, not
   an independent pixel match to Genesis.
4. **Is it animated, and when?** The text, frame, and continuation control
   remain pixel-identical during a 30-frame neutral hold. A edge plus release
   advances each page. Closing page 2 fades the field back in for 60 host
   frames; this is a native presentation hold, not a ROM timing claim, and
   world simulation is paused during it.
5. **Is there a sound?** The indexed event handler has no explicit sound
   request in the traced path, and the QA observes no new audio event while
   opening or advancing its pages. Generic prompt/input audio outside the
   traced handler remains to be checked before declaring the source route
   silent.
6. **Are the rest of the screen and old controls correct?** Full-screen
   captures show no field buttons or map layer behind the modal. At return,
   the field map and HUD replace the dialogue; the QA verifies the same scene
   and event context, zero pending transition, clear fade state, and no stale
   dialogue pixels. The return-frame image is saved as `actual.bmp`.
7. **Do multiple frames support the interpretation?** The route trace records
   every approach frame; captures include the approach, all three pages, two
   idle frames, and all 60 return-fade frames. The 72 selected Debug/Release
   captures match byte-for-byte, and the complete fade contact sheet was
   inspected. These are two builds of the same renderer, not independent
   player captures; the supplied gameplay reference has not yet been matched
   to this exact Rat's Nest sequence.

## Artifacts and code

- Test: `shadowrun_ghidra_conversion_qt_field_redmond_vigore_search_qa`.
- Debug dump: `decompilation/ghidra_conversion/build-ghidra-conversion-qt-debug/qt_runtime/field-redmond-vigore-search-qa/actual.bmp`.
- Release dump: `decompilation/ghidra_conversion/build-ghidra-conversion-qt-release/qt_runtime/field-redmond-vigore-search-qa/actual.bmp`.
- Script: `gameplay/scripts/redmond_barrens.lua`; generated ROM copy is in
  `assets/dialogue/script-text.lua`.
- Host integration/effects: `qt_runtime/src/field/field_scene_runtime.cpp`.
- Source profile and outstanding record ambiguity:
  [field-map-trigger-profile.md](../field-map-trigger-profile.md) and
  [Mike's RPG Center notes](../mikes-rpg-center-shadowrun-reference-notes.md).
