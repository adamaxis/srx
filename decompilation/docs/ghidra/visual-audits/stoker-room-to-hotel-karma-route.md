# Physical Stoker doorway to Hotel Karma route

Date: 2026-10-08 (Lua text-binding review updated 2026-10-09; extended capture review updated 2026-10-27)

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
advances the arrival dialogue. In this New Game route, the content-authored
first-visit menu offers the room, leave, and Michael's package choices; its
room choice dispatches command `0x06` to the payment logic at `0x21458`. The
separate post-opening arrival branch still uses the bounded selector at
`0x249C4`. At 20 nuyen, the room choice deducts the live field
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
input buys Body 2 for Karma 48. The PRNG seed `0x51A7` stabilizes Hotel bezel
animation; it is a QA fixture, not a claim that live ROM seed initialization
has been recovered.

## Captures and review window

Debug and Release `--field-hotel-qa` runs write
`decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-hotel-qa/sequence/`.
Each sequence has 414 consecutive 320x224 BMPs: frame 0000 is the initialized
New Game field before input; frames 0001..0381 cover approach, trigger/fade,
dialogue, Hotel pages, Karma prompt/result, checkout, the field-door return,
and its 30-frame hold; frames 0382..0413 cover opening and closing the command
overview after checkout. The checkout return is visibly extended: the prompt
begins dimming after the Yes selection, faint HUD remnants remain in frames
0233..0234, the full screen is black from frame 0235 through frame 0317, the
field begins to reveal at frame 0318, and it is fully drawn at frame 0321. The
QA also writes `physical-route.tsv`
and 64 named full-screen state checkpoints, for 478 BMPs total per
configuration. All 478 Debug/Release image SHA-256 pairs match.

The route's state milestones are:

| Frame | Ordinary input/state | Result |
|---:|---|---|
| 0000 | New Game spawn | Scene 2, position `(15584,11520)`, wallet 20 |
| 0022 | Up | Record 91; raw collision `0xFFC2`, event 2 |
| 0039 | Up released; transition complete | Stoker arrival panel |
| 0041 | A | First-visit scripted manager menu with package choice |
| 0043 | A | Room offer for 20 nuyen |
| 0045 | A | Paid message; wallet 20 to 0 |
| 0047 | A | Hotel status page |
| 0080 | 30 idle frames | Hotel status remains stable |
| 0082 | A | Hotel options |
| 0101 | A | Hotel Karma spending page |
| 0141 | A on Body upgrade | Body 1 to 2; Karma 50 to 48 |
| 0143 | Start | Hotel options redrawn |
| 0177 | Down, A | Checkout prompt, default No |
| 0218 | Up | Checkout Yes selected |
| 0235..0317 | B on Yes; return transition | Full-screen black interval |
| 0318 | Field-door return fade | Field begins to reveal |
| 0321 | Field-door return fade | Field scene fully drawn again |
| 0381 | 30 idle frames after return | Stable field still has no Hotel graphics |
| 0411 | Start; 27 black frames plus reveal | Command overview visible |
| 0413 | Start | Command overview closed; clean field return |

The source and native route checks were reviewed alongside every full-screen
frame. The capture is an internally generated regression artifact and is not
an independent Genesis visual oracle.

## Supplemental external recording reviewed 2026-10-27

The user supplied `e8_karma_spend.zip`, containing `au/e8_karma_spend/` with
620 consecutive 320x224 PNG frames (`e_08541.png` through `e_09160.png`), an
input log, RAM snapshots, and audio. The package has no ROM hash, emulator
build identity, or capture method, so it is useful as an external comparison
but is not independently verified Genesis footage. The archive remains
outside the repository and is not part of this documentation commit.

The input log records Up at frame 8641 followed by A presses at 8751, 8767,
8807, 8847, 8891, 9001, and 9081. The visible sequence reaches the Stoker
door, room offer and payment, Hotel status/options, and the Karma child. At
frame 9040 Joshua is Human/Shaman with Body 2 and Karma 20; the page previews
Body 3 for cost 3. A later frame shows the default-Yes purchase prompt, but
the recording contains no subsequent confirmation input or resulting spend.
It does not show a checkout or field return.

The supplied Stoker, manager, room-offer, paid-message, Hotel Options, and
Karma frames resemble their corresponding native QA screens at the page and
layout level. Their differing runner state and unrecorded source identity
prevent using these comparisons as exact Genesis pixel goldens. The external
sequence also holds full black for frames 8693..8732 (40 frames), begins
drawing the shared panel at 8733, and shows its complete text by 8736. The
current native doorway QA has five full-black frames before drawing the
populated page. The ROM call relationship is now traced: fade completion at
`0x16396` is consumed at `0x095E`, which calls `0x53126` before the
`0x1F5CA` field-exit orchestrator. For the captured scene-2/event-2 state, the
mode gates `FFE186` and `FFDF40` are zero; `0x3168` takes the non-`0x10` branch
at `0x31EC` and clears carry, allowing that orchestrator call. The supplied
RAM counter `FFDF88` advances by exactly 18 waits from frame 8697 to 8718,
matching `0x53126`'s six three-VBlank groups. It stays at `0x20C6` through
frame 8734 while the image remains black through 8732 and the panel starts at
8733. The mode-entry service is therefore tied to this capture's transition,
but the later black frames remain unexplained and the capture has no ROM/build
identity. See the detailed [doorway transition trace](motel-doorway-scene-entry.md#supplemental-recording-reviewed-2026-10-27).

## Nine-question visual review

1. **Does it look correct? — Unknown against the Genesis game.** The supplied
   independent section 03 footage supports the Stoker doorway/fade/panel
   family, including frames 000028..000055 and 000080..000082. The supplied
   `e8_karma_spend.zip` adds Hotel status, options, and Karma pages, but has no
   ROM or emulator identity. It supports broad layout comparison, not verified
   Genesis pixel fidelity. The native screens are internally legible and
   consistently laid out, but their exact original appearance remains open.
2. **Does the visible look match behavioral state? — Yes for the tested
   route.** Ordinary input selects collision `0xFFC2`/event 2, the first-visit
   manager offers rent, leave, and Michael's package, payment changes the
   wallet from 20 to 0, the paid-message
   dismissal opens Hotel status, A enters Hotel options and the distinct
   spending page, and the confirmed upgrade changes Body/Karma to 2/48.
   `physical-route.tsv` records the route states and input milestones.
3. **Are the visuals themselves correct? — Unknown against independent
   Genesis pixels.** The source-derived full-page redraw, text colors,
   selection cursor, cost/result details, and output dimensions are checked by
    the Qt route assertions. The supplied external sequence resembles the
    corresponding Hotel pages, but uses a different runner state and does not
    identify its ROM/emulator build. Exact original pixel placement and palette
    remain unverified.
4. **Does it animate as expected? — Yes for the traced native cadence.** The
   Hotel status/options and checkout wait loops call `0xE660` once per update;
   Debug and Release check 30 updates, stable text, and change confined to the
   lower bezel. The Hotel Karma child suspends its parent animation and stays
   static. Checkout's `0xDABA` call advances the bezel while its question and
   selection remain fixed. The QA fixture seed stabilizes the random bezel
   art. Independent footage does not establish Hotel animation timing.
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
   capture includes separate input moments, idle frames, page changes, the
   full fade-to-black/field-return interval, and the later command overview.
   Independent section 03 frames support the Stoker doorway and
   dialogue appearance, but contain no Hotel menu visit or controller log.
   The separate supplied sequence covers Hotel status/options and the Karma
   preview/prompt, without a confirmed purchase or checkout; its source
   identity is unknown.
8. **Does the new UI replace the old UI cleanly? — Yes in the tested route.**
   Hotel status replaces the field/HUD; options replace status; the Karma
   spending page contains no old Hotel options; Hotel options are redrawn on
   child return; checkout removes the option rows; and the field-only redraw
   equals the immediate and 30-frame returned full-screen images. No old
   buttons or graphics remain over the next page.
9. **Was the whole time window reviewed? — Yes for this route.** The sequence
   begins at initialized New Game field spawn and runs continuously at one
   image per controller/update frame through checkout, field return, and the
   command-overview open/close sequence. All 414 full-screen images were
   reviewed in chronological contact sheets; frames 0218, 0233, 0235, 0317,
   0318, 0321, 0381, and 0411..0413 were inspected at native size. The
   QA process reached the stable field state and exited normally. This scope
   starts after New Game initialization; it is not a full boot/title capture.

## Verification outcome

- **Code changed:** checkout now advances its ROM-matched lower-bezel effect
  and places the No selector on the second text row, matching the two-row
  stride derived from `0xC376`/`0xC3B2` and the `0xE387E` Yes/No text.
- **Code changed:** `gameplay/scripts/hotel.lua` now supplies both cursor-menu
  choice lists to the live Hotel route, with explicit title and per-choice
  text keys for both Options and Checkout. Its `text_keys()` map also binds
  the Options separator, medical-attention suffix, and recovery rows. The
  native presenter resolves those keys against the ROM-derived copy in
  `assets/dialogue/script-text.json`; it keeps layout and input ownership.
- **Code changed:** the Hotel QA follows the physical doorway and normal
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
- **Text-binding follow-up:** Debug and Release passed
  `shadowrun_gameplay_script_runtime_qa`,
  `shadowrun_ghidra_conversion_qt_field_hotel_qa`, and
  `shadowrun_ghidra_field_menu_text_catalog`. The current Debug and Release
  outputs contain 414 route frames and 64 named BMP checkpoints (478 BMPs
  total per configuration), with matching SHA-256 hashes. The earlier review
  covered frames 0000..0222; this update reviews frames 0223..0413 in 24
  chronological contact sheets. The checkout fade, long black interval,
  field reveal, command overview, and clean return were inspected at full
  resolution. The text-binding change leaves the output pixels unchanged.
- **Independently visually checked:** section 03 supports the Stoker doorway
  and dialogue panel. The supplied external recording adds Hotel-page visual
  comparisons, but its source identity is unknown and it does not establish
  Genesis pixel fidelity.
- **Reported defect resolved:** no new independent Hotel appearance claim is
  made. The native route is connected and internally verified; exact Hotel
  visual fidelity remains open.
