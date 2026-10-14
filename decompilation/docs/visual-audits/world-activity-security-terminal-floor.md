# World-activity resource scene and Security Terminal floor

This audit covers the path from a resolved field event code through resource
scene preparation, the Security Terminal result, and return to the selected
field. The feature remains partial where noted.

## Source and captures

- Primary source: US ROM paths at `0x167AE`, `0x16B7C`, `0x03074`, `0x2BD2`,
  and `0x20EC2`; profile/resource bundle values are extracted into individually
  named host records.
- Cross-check: Ghidra conversion routines and ROM address annotations in the
  conversion headers; they are not the visual authority.
- Internal captures: `build/archive/decompilation/build-qt-runtime-debug/qt_runtime/field-world-activity-qa/`
  and the corresponding Release directory. `actual.bmp` is the third-floor
  result. Other images capture initial scene, resource-scene entry/frames,
  Security Terminal choices, and field return.
- Independent visual reference: no matching Security Terminal floor-result
  sequence has been identified in the indexed longplay sections. Therefore no
  Genesis appearance claim is made here.

## Eight verification questions

1. **Does it look correct? — Unknown.** The internal 320x224 result frame is
   readable and uses the existing framed field-dialogue presentation, but no
   matching independent Genesis frame was found to compare its pixels.

2. **Does the look match the behavioral state? — Yes, internally.** The QA
   injects already-resolved `FFFBF9 = 2` for scene 2. ROM `0x167AE` uses this
   byte as an index into the active `FFDE14` table; scene 2's entry 2 selects
   resource scene `0x1A`. This differs from collision event 2 in
   building-interaction mode, which selects Stoker's dialogue. The focused QA
   resolves its generic world-activity table route through `building.lua`,
   keeping those meanings separate. The selected resource has random bound 3;
   its bundle cursor fields are `+0x14 = 0x18` and `+0x16 = 0`. With
   deterministic host seed `0x1234`, Scene 26 first initializes its one
   source-listed corporate guard, consuming one behavior-delay PRNG advance.
   The subsequent source draws publish `FFE12C = 2` and `FFE185 = 3`; the
   successful result text says “third
   floor.” QA compares each typed field and the resulting visible text. The
   physical collision that writes this focused QA's resolved event code is
   outside this test; the separate event-1 route verifies normal collision-to-
   resource-scene entry and the selected-floor handoff to the physical D8
   terminal.

3. **Are the visuals correct? — Unknown against Genesis.** Runtime loads the
   named scene map and cursor metadata, then uses the existing Security
   Terminal choice/result panel. The internal captures show no malformed
   selector or clipped result text. Palette, exact placement, and pixels still
   need a matching Genesis capture.

4. **Should it animate, and at what cadence? — Partly known.** The host field uses a 16 ms Qt timer (about 60 updates per second) while it
   owns the screen; the modal interaction path suspends field updates until it
   closes. QA captures
   the selected field after 0, 30, and 60 calls to `advance_frame`; the images
   differ by 45,768 and 46,902 pixels across those intervals. This shows host
   field changes, but does not isolate which pixels animate or establish the
   original scene's counter, cadence, or start/stop conditions. Those remain
   unverified.

5. **Is there a sound, and how long does it play? — Partly known.** No sound
   call was found in the traced world-activity selection or resource-cursor
   copy. When scene preparation initializes its one corporate guard, the Lua
   actor profile requests entry event `0x2E`; the successful Security Terminal
   result later requests event `0x30`, which maps to persistent resource 048.
   QA checks the ordered requests `{0x2E, 0x30}` only. It does not prove host
   playback or a matching Genesis waveform, and the `0x30` stop lifetime
   remains unresolved.

6. **Are the other visible parts of the screen correct? — Internally checked.**
   Full-frame captures include the field HUD, selected map, prompt, and result.
   The selected map replaces the starting scene. The Security Terminal owns the
   full frame while open, and the field return contains no terminal text or
   controls. Pixel identity here is a consistency check using the same host
   compositor, not a Genesis visual golden.

7. **Do multiple frames support the interpretation? — Internally yes;
   independently unknown.** The QA captures scene entry, two later field
   frames, both terminal pages, and return after separate input actions. No
   matching real-game clip showing this terminal and floor result has been
   identified, so there are no independent frames to support the visual
   interpretation.

8. **Does the new UI replace the old UI cleanly? — Yes, internally.** The
   terminal prompt/result replaces the field presentation. After Walk Away,
   `return-field.bmp` is pixel-identical to the field-only underlay captured
   immediately before the prompt. This catches stale terminal graphics in the
   host return path but is not independent fidelity evidence.

## Outcome

- **Code changed:** named resource-cursor extraction/loading, typed field
  activity dispatch and scene preparation, Security Terminal floor-state
  handoff, and `0x2BD2` modal return/re-entry.
- **Behavior implemented:** resolved event-table selection, resource bound and
  cursor publication, deterministic floor selection, floor-result text, and
  clean selected-field return.
- **Internally checked:** Debug and Release world-activity and Security
  Terminal Qt QA; Debug and Release field-session source-asset and world-
  activity conversion tests; `actual.bmp` and companion frame captures.
- **Independently visually checked:** no matching Genesis Security Terminal
  floor-result frames identified.
- **Reported defect resolved:** no visual defect-resolution claim; independent
  pixel evidence is unavailable.

On 2026-10-10, the dispatcher QA setup was repaired after the Lua route
migration left it injecting `FFFBF9` without constructing the typed area-event
route required by the shared executor. It now uses the explicit
`FieldExitMode::world_activity_table` context, resolves scene 2 table entry 2
to scene `0x1A` through Lua, and keeps the physical collision route outside
this focused dispatcher test. Its output directory clears old BMPs before a
run. All nine Debug full-screen frames were reviewed; the Release counterparts
match by SHA-256. Scene replacement, the terminal overlay, and clean return
remain intact. This is a test setup correction, not new independent Genesis
visual evidence.

The dedicated dispatcher QA still injects its resolved event byte, while the
joined scene-2 event-1 route now verifies one normal collision-to-resource-
scene path through the physical D8 terminal and selected-floor result. The
feature stays partial because other collision-to-event routes, exact source
animation cadence, persistent sound stop behavior, the scene-2 runner/HUD
overlap, and independent Genesis visual frames remain open.
