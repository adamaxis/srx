# Scene-2 Motel Doorway and Scene-0x14 Entry Audit

Date: 2026-10-05

Source trace: US ROM scene-2 descriptor and event table (`0x172C80`,
`0x172CDC`, `0x172DAA`); field loop `0x075E`/movement `0xB7B8`; fade
`0x16396`; transition coordinator `0x095E`; scene preparation/re-entry
`0x167AE`/`0x2BD2`.

Runtime: `qt_runtime/field_scene_runtime.cpp`; QA:
`shadowrun_ghidra_conversion_qt_field_motel_entry_qa`.

## Evidence and captured frames

- The New Game field starts in scene 2 at `(0x3CE0, 0x2D00)`. The controlled
  runner has behavior bit `0x02` set. Controller nibble `1` is up according to
  the extracted heading table.
- Normal up input moves the runner into the C0 doorway collision. The blocked
  interaction publishes event 0; active scene-2 event-table row 0 selects
  destination scene `0x14`. After 17 held updates, the runner is at raw
  fixed-point position `(0x3CE0, 0x2A58)` with camera X `0x3400`; QA observes
  event code 0 and projected code `0x20` at the transition.
- The collision frame already includes fade update 1: signed timer 15 and one
  palette step. The source fade expires after 17 field updates. The following
  transition loads the destination buffers, clears the old fade overlay, and
  refreshes the camera/entity render state.
- The destination runner remains at `(0x5A00, 0x0800)` with camera
  `(0x5100, 0)` and stays visible through 30 no-input updates. Dialogue stays
  closed through the 30 no-input updates currently covered by this QA. This
  establishes the scene-entry result, not the trigger shown in the separate
  Stoker arrival sequence.
- **Review correction (2026-10-05):** the earlier `destination-idle-*` captures
  were taken while Up was still held. The runner looked stationary because the
  destination spawn is blocked, so those frames did not prove no-input idle
  behavior. The QA now releases the controller before `destination-entry` and
  asserts the D-pad nibble is zero and the runner's world position is unchanged
  after 30 neutral updates. The corrected Debug and Release outputs match by
  SHA-256; all 13 regenerated full frames were opened and reviewed at 320x224.
  The destination screen remains very dark in those captures. Without a
  matching independent Genesis frame, that appearance is unresolved rather
  than confirmed correct.
- Debug and Release QA each emit the same 13 complete 320x224 BMP checkpoints:
  `spawn`, `movement-8`, `movement-16`, `doorway-trigger`, `fade-08`,
  `fade-16`, `fade-17`, `destination-entry`, `destination-idle-01`,
  `destination-idle-08`, `destination-idle-30`, `destination-settled`, and
  `actual`. Their SHA-256 hashes match between configurations. The QA removes
  known old checkpoint names and fails on any missing or extra BMP, so frames
  from the earlier down-input run cannot be mistaken for current evidence.
- All 13 corrected motel-route screenshots were opened and reviewed at their
  full 320x224 size. The related six transition-fade and nine world-activity
  BMPs were also reviewed in full; the Debug and Release outputs are
  byte-identical.
- Nearby independent longplay frames were checked in the supplied 30-fps
  corpus: section 02 frames `001740`/`001800` show cab destination selection;
  section 03 local frames `000030`, `000060`, and `000090` show a city
  approach, dark transition, and Stoker arrival prompt, but do not expose the
  controller input or exact collision coordinate. Section 03 local frames
  `000300`/`000600` show later Stoker dialogue and `001200` shows Jump House
  text; section 04 frame `003000` shows a different dark field corridor. No
  checked frame identifies this exact scene-2 contact point or confirms the
  Qt destination pixels. This check is not an exhaustive scan of every video
  frame.

## Question pass

1. **Does it look correct? Unknown against the original game.** The full native
   frames show movement toward the doorway, a full-screen fade, and a distinct
   interior field map. No matching independent Genesis frame of this exact
   route was found. The extracted-asset renderer cannot serve as its own visual
   proof.
2. **Does the visible look match behavioral state? Yes for the covered route.**
   Up input moves the selected runner; the collision publishes event 0; row 0
   selects scene `0x14`; the transition clears the old scene; and the runner
   remains visible at the destination. The dialogue panel is closed. No
   Stoker arrival/payment dialog is selected by this evidence.
3. **Are the visuals themselves correct? Unknown against Genesis; internally
   coherent.** Every full frame was inspected, including the city map, runner,
   right-side field HUD, fade, and destination map. No old city layer or black
   fade sheet remains over the destination. The exact destination palette,
   floor texture, tile placement, and sprite pixels still need a matching
   independent capture.
4. **Should it animate? Yes while moving; no idle change was observed.** The
   movement checkpoints show the runner changing pose and position. The fade
   follows the source 17-update lifetime and even-count palette-step cadence;
   it reaches full black by checkpoint 16. With controller input released,
   destination frames at idle updates 1, 8, and 30 are identical, so this route
   currently presents a static idle pose. A source-derived idle animation
   requirement for this destination has not been established.
5. **Is there sound?** No direct collision/fade SFX call was traced. The source
   mode-entry audio service sends command `0x14` six times, waits three VBlank
   frames per transaction, flushes, then services one audio tick. The Qt
   callback currently records and discards those commands; no host playback,
   sample identity, or sound duration was verified. The QA sees zero discrete
   event-style SFX callbacks, which does not prove the source audio service is
   silent.
6. **Are the other visible parts correct? Yes for replacement and internal
   state; exact original appearance remains unknown.** Full-screen review
   included the HUD, surrounding map, fade, and destination. The intended field
   HUD persists while the old city map and transition overlay are replaced.
   The motel route does not open a menu or dialogue. Related transition and
   Security Terminal frames were reviewed as well.
7. **Do multiple frames support the interpretation? Yes internally.** Spawn,
   two movement points, doorway collision, three fade checkpoints, and three
   destination idle points show the state sequence. The independent recording
   samples checked above do not contain this exact doorway path, so independent
   multi-frame support is unavailable.
8. **Does new UI replace old UI cleanly? Yes for this scene transition.** There
   is no new button panel on this path. After scene assets commit, the full
   city background and fade overlay are gone; only the shared field HUD remains
   over the destination. No stale dialogue, selector, or prior scene actor is
   visible.

## Result

- **Code changed:** the scene-2 event-table route continues through the source
  coordinator; destination assets commit before clearing the Qt fade overlay;
  camera/entity presentation refreshes after scene re-entry; and the QA
  removes/checks its complete named screenshot set. This review correction
  releases Up before destination captures and checks both the zero D-pad
  state and position stability during neutral input.
- **Behavior implemented:** normal up input reaches the scene-2 motel doorway,
  publishes event 0, and loads event-table destination `0x14`. This route does
  not synthesize dialogue.
- **Internally checked:** the corrected motel-entry CTest passes in Debug and
  Release. All 13 corrected motel-route BMPs were reviewed; Debug/Release
  names and SHA-256 hashes match. Earlier evidence in this audit records the
  related fade-transition, world-activity, and Stoker-arrival-manager checks.
- **Independently visually checked:** no matching Genesis image for this exact
  doorway transition was found in the checked longplay sections.
- **Reported defect resolved:** no separate motel-screen appearance report is
  claimed as resolved. The internal conversion route now reaches the event
  table destination, clears the prior fade after the new scene is committed,
  and draws the destination runner. Genesis visual fidelity and host playback
  for the mode-entry audio service remain unverified.
