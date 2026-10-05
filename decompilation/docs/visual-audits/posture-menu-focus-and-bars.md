# Command-menu posture focus and combat bars

This review covers the `0x119CE` focus row and the posture state change in the
field command menu. It does not claim that the separate overhead combat loop,
the negative-equipped-slot special branch, or AI posture decisions are
implemented.

## Evidence

- ROM path: `0x119CE` accepts Up on command pages 2 through 6; `0x11ABE` clamps
  posture byte `+0x4D` to 0..4 and refreshes the row; `0x11AE4` draws the
  posture label/state; the caller reaches `0x12CEE` to refresh attack and
  defense values.
- ROM layout: the focus label is at `0xCA9E` (120,168), 48x8; the five-state
  bar begins at `0xCAAC` (176,168). Horizontal changes request sound event
  `0x25`, mapped to resource 037 (2.134 seconds in the decoded asset). Up,
  Down, and Start do not request this sound.
- Internal frames: `build-qt-runtime-debug/qt_runtime/field-posture-qa-frames/`
  and corresponding Release output. The CLI `actual.bmp` is a real focused
  posture state. Frame QA compares the named focus asset, full command-page
  pixels, status, and combat-bar regions.
- Independent visuals: supplied longplay sections show neighboring command
  pages, but no selected-posture transition or horizontal posture change was
  identified.

## Eight verification questions

1. **Does it look correct? — Unknown against Genesis.** The internal focused
   frame places `POSTURE` at the expected row and keeps the five-state bar
   visible. No matching independent selected-posture Genesis frame is
   available for a pixel comparison.

2. **Does the visible look match the behavioral state? — Yes, internally.**
   Up opens focus without changing the active command page or posture. Left
   and Right change the active runner's posture among Full Defense, Med.
   Defense, Neutral, Med. Offense, and Full Offense. The posture value and
   both attack/defense values are asserted after changes; attempts past either
   endpoint leave state clamped at 0 or 4.

3. **Are the visuals themselves correct? — Yes against extracted/source
   layout; unknown against Genesis pixels.** QA matches the 48x8 named focus
   image at (120,168), checks the five-state bar at (176,168), and verifies
   that both 104x8 attack/defense strips change when posture changes. It also
   requires every pixel outside the focus row to remain unchanged when focus
   opens or closes.

4. **Should it animate, and how often? — No focus animation.** The source
   redraws the selected focus/state in its controller loop; it does not
   advance a focus animation counter. The Qt QA holds the page for 30 field
   frames and requires the focused image to remain identical. Posture/bar
   changes occur on accepted horizontal input edges.

5. **Is there a sound, and how long does it play? — Yes for posture changes.**
   Each accepted Left/Right step requests event `0x25`; Up focus, Down
   deselection, and Start return are silent. The decoded resource lasts
   2.134 seconds. Shared audio QA verifies decode, `PlayingState`, duration,
   and stop on the default output; output loopback for this exact menu route
   has not been recorded.

6. **Are the other visible parts of the screen correct? — Internally checked.**
   Across command pages 2 through 6, focus changes only its 48x8 row. The
   portrait, item slots, status, bars, and selected command page remain
   visible. Changing posture updates both combat strips while preserving the
   rest of the page.

7. **Do multiple frames support the interpretation? — Internally yes;
   independently unknown.** QA captures entry and exit on all five eligible
   command pages, a 30-frame hold, every posture value, both clamps, and the
   Down-plus-horizontal priority case. The independent longplay has no
   matching selected-posture example.

8. **Does the new UI replace the old UI cleanly? — Yes, internally.** Down
   returns each page to a pixel-identical pre-focus frame. Start first clears
   focus, then returns to a field frame pixel-identical to the pre-menu
   capture. No old selector or command graphics remain over the returned
   screen.

## Outcome

- **Code changed:** no runtime code changed in this review; it records the
  already-connected Qt route and its current evidence.
- **Behavior implemented:** Up focuses posture on pages 2..6; Down deselects;
  Left/Right update typed posture, source labels, attack/defense bars, and
  event `0x25`, with source clamping.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_field_posture_qa`
  passed in Debug and Release and wrote the frames above. The shared audio
  test separately verifies event `0x25` playback.
- **Independently visually checked:** no matching Genesis selected-posture
  sequence found.
- **Reported behavior resolved:** the requested menu navigation and visible
  combat-bar response pass internal checks; exact Genesis appearance and
  route-specific audio output remain unverified.

The posture UI remains partial because source combat/AI consumers and the
negative `+0x56` special attack branch remain incomplete.
