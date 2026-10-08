# Physical Stoker doorway to Hotel Karma route

Date: 2026-10-08

Scope: the ordinary New Game field route from scene-2 Stoker collision record
91 through the paid-room message, Hotel status/options, Hotel Karma spending,
checkout, and a stable return to the field. This route does not load a motel
interior map.

## Source trace and native connection

The scene-2 collision table record 91 at ROM `0x173138` has property `0xC2`
and bounds `(0x3C0..0x3DF, 0x280..0x29F)`. New Game spawns Joshua at
`(0x3CE0, 0x2D00)`; ordinary Up input reaches the doorway, publishes raw
collision `0xFFC2` and event 2, and selects Stoker's mode-2/event-2 arrival
dialogue through the field-exit route at `0x1F65A`. The ordinary A command
uses the bounded arrival branch at `0x249C4`; the QA fixes its seed to select
the 0..79 manager branch. The manager's room choice dispatches command `0x06`
to the payment logic at `0x21458`. At 20 nuyen, it deducts the live field
wallet and selects the paid message at `0xF5AB8`; after that message is
dismissed, the source caller hands off to Hotel Options at `0xCF38`.

The Qt path sends controller-port input through
`FieldSceneRuntime::advance_frame()`. Its typed party/wallet state reaches the
shared generic dialogue presenter, the Hotel status/options controller, the
separate Hotel Karma upgrade child at `0xD024`, and checkout. The Hotel child
uses the Attributes/Skills data grid but has spending controls and next-value
cost details; the in-game Start-menu Karma status page is a different view.
In ordinary play, `GhidraConversionWindow::keyPressEvent()` and
`keyReleaseEvent()` forward the focused window's controller byte, and its
16-ms host timer calls `FieldSceneRuntime::advance_frame()` while the game is
in the field. The QA drives that same controller-port interface with brief
button presses, so payment and Hotel are exercised through the normal field
dispatcher.

The test seeds Body 1 and Karma 50 to make the upgrade outcome observable,
sets the status/cyberware fields used by the Hotel panel, then ordinary A
input buys Body 2 for Karma 48. The PRNG seeds (`0` for the manager branch
and `0x51A7` for repeatable Hotel bezel animation) are QA fixtures, not a
claim that live ROM seed initialization has been recovered.

## Captures and review window

Debug and Release `--field-hotel-qa` runs write
`build/conversion/qt/{debug,release}/qt_runtime/field-hotel-qa/sequence/`.
Each sequence has 223 consecutive 320x224 BMPs: frame 0000 is the initialized
New Game field before input; frames 0001..0192 cover approach, trigger/fade,
dialogue, Hotel pages, Karma prompt/result, checkout, and the immediate field
return; frames 0193..0222 are the 30-frame stable field hold. The QA also
writes `physical-route.tsv` and named full-screen state checkpoints. All 223
Debug/Release image SHA-256 pairs match.

The route's state milestones are:

| Frame | Ordinary input/state | Result |
|---:|---|---|
| 0000 | New Game spawn | Scene 2, position `(15584,11520)`, wallet 20 |
| 0017 | Up | Record 91; raw collision `0xFFC2`, event 2 |
| 0034 | Up released; transition complete | Stoker arrival panel |
| 0036 | A | Deterministic manager branch |
| 0038 | A | Room offer for 20 nuyen |
| 0040 | A | Paid message; wallet 20 to 0 |
| 0042 | A | Hotel status page |
| 0074 | A | Hotel options |
| 0076 | A | Hotel Karma spending page |
| 0116 | A on Body upgrade | Body 1 to 2; Karma 50 to 48 |
| 0118 | Start | Hotel options redrawn |
| 0152 | Down, A | Checkout prompt, default No |
| 0190 | Up | Checkout Yes selected |
| 0192 | B | Field restored; Hotel closed |
| 0222 | 30 idle frames after return | Stable field still has no Hotel graphics |

The source and native route checks were reviewed alongside every full-screen
frame. The capture is an internally generated regression artifact and is not
an independent Genesis visual oracle.

## Nine-question visual review

1. **Does it look correct? — Unknown against the Genesis game.** The supplied
   independent section 03 footage supports the Stoker doorway/fade/panel
   family, including frames 000028..000055 and 000080..000082. No matching
   independent Hotel status, options, Karma-spending, or checkout frames were
   found. The native screens are internally legible and consistently laid
   out, but that cannot establish their exact original appearance.
2. **Does the visible look match behavioral state? — Yes for the tested
   route.** Ordinary input selects collision `0xFFC2`/event 2, the manager
   offers rent, payment changes the wallet from 20 to 0, the paid-message
   dismissal opens Hotel status, A enters Hotel options and the distinct
   spending page, and the confirmed upgrade changes Body/Karma to 2/48.
   `physical-route.tsv` records the route states and input milestones.
3. **Are the visuals themselves correct? — Unknown against independent
   Genesis pixels.** The source-derived full-page redraw, text colors,
   selection cursor, cost/result details, and output dimensions are checked by
   the Qt route assertions. No independent Hotel screenshot is available to
   verify its exact pixel placement, palette, or text appearance.
4. **Does it animate as expected? — Yes for the traced native cadence.** The
   Hotel status/options wait loop calls `0xE660` once per loop update; Debug
   and Release checks 30 updates, stable text, and change confined to the
   lower bezel. The Hotel Karma child suspends that parent animation and stays
   static; nested checkout also stays static. The QA fixture seed stabilizes
   the random bezel art. Independent footage does not establish Hotel
   animation timing.
5. **Does it have sound? — Unknown for the complete route.** The traced room
   payment leaf at `0x21458` contains no direct sound request. The indirect
   field-exit audio hook and any associated playback duration remain
   unresolved. The runtime QA checks that the Karma child and checkout add no
   audio events after the event log is cleared; this does not establish that
   earlier dialogue or Hotel entry is silent.
6. **Are the other visible parts of the screen also correct? — Reviewed.**
   The full 320x224 sequence covers the field/HUD, doorway transition, shared
   dialogue panel, Hotel status/options, Karma prompt/result, checkout, and
   returned field. The Hotel screen removes the field HUD; the Karma page does
   not retain Hotel controls; return restores the field. Other unrelated
   scene defects are not closed by this route.
7. **Do multiple frames support the interpretation? — Partial.** The native
   capture includes separate input moments, idle frames, page changes, and
   return. Independent section 03 frames support the Stoker doorway and
   dialogue appearance, but contain no Hotel menu visit or controller log.
   Independent Hotel interaction frames remain unavailable.
8. **Does the new UI replace the old UI cleanly? — Yes in the tested route.**
   Hotel status replaces the field/HUD; options replace status; the Karma
   spending page contains no old Hotel options; Hotel options are redrawn on
   child return; checkout removes the option rows; and the field-only redraw
   equals the immediate and 30-frame returned full-screen images. No old
   buttons or graphics remain over the next page.
9. **Was the whole time window reviewed? — Yes for this route.** The sequence
   begins at initialized New Game field spawn, runs continuously at one image
   per controller/update frame through checkout, then holds the field for 30
   idle frames. All 223 full-screen images were reviewed in chronological
   contact sheets and transition frames were inspected at native size. The
   QA process reached the stable field state and exited normally. This scope
   starts after New Game initialization; it is not a full boot/title capture.

## Verification outcome

- **Code changed:** the Hotel QA now follows the physical doorway and normal
  input through the live `FieldSceneRuntime`, dialogue, payment, Hotel,
  Karma, and checkout handlers; no test helper injects a dialogue, payment
  result, or menu state.
- **Behavior implemented:** this slice adds no parallel game state machine;
  it documents and exercises the connected typed path and verifies the live
  wallet/stat mutations and clean screen handoffs.
- **Internally checked:** Debug and Release builds pass
  `shadowrun_ghidra_conversion_qt_field_hotel_qa`,
  `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`,
  and `shadowrun_ghidra_conversion_qt_karma_screen_distinction_qa`. The Hotel
  route writes its continuous sequence, named checkpoints, and
  `physical-route.tsv` under `qt_runtime/field-hotel-qa/`.
- **Independently visually checked:** section 03 supports the Stoker doorway
  and dialogue panel only. No independent Hotel-page capture is available.
- **Reported defect resolved:** no new independent Hotel appearance claim is
  made. The native route is connected and internally verified; exact Hotel
  visual fidelity remains open.
