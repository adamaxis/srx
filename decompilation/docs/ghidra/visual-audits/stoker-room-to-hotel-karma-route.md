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
input log, RAM snapshots, and audio. The user reports that Grok bot provided
the archive. Its expanded copy is now available under the ignored research
output `decompilation/ghidra_conversion/build/research/au/e8_karma_spend/`
(`frames/`, `inputs.txt`, `ram.npy`, and `audio.wav`). The package has no ROM
hash, emulator/build identity, or capture method, so it is useful for matching
screens to its own RAM/input trace but is not independently verified Genesis
footage.

[C] The frames pinpoint the reported fade around 8690: `e_08670.png` shows
Joshua at the Scene-2 doorway, `e_08690.png` is black while RAM remains
`FFFBF8/FFFBF9 = 0x02/0x02`, and `e_08740.png` shows Stoker's opening dialogue
with that pair unchanged. This is the Scene-2/event-2 Stoker interaction from
collision record 91 at `0x173138` and the field-exit route at `0x1F65A`; Hotel
services appear without loading a separate motel interior scene.

The input log records Up at frame 8641 followed by A presses at 8751, 8767,
8807, 8847, 8891, 9001, and 9081. The visible sequence reaches the Stoker
door, room offer and payment, Hotel status/options, and the Karma child. At
frame 9040 Joshua is Human/Shaman with Body 2 and Karma 20; the page previews
[C] Frame 9080 shows the default-Yes purchase prompt, and the input log records
a four-frame A press beginning at 9081. From frames 9040 through 9160 the
snapshots keep `FFFC04=0x00FF0100`; Body at record `+0x77` and Karma at `+0x8D`
remain 2 and 20, while the displayed cost and increase remain 3. No upgrade
is applied in this capture. [C] At frame 9080, `FFDD22=0x4040` and
`FFF0DC=0`; at 9081, `FFDD22=0x0040`. ROM `0x19F2` maps A to controller bit 6,
and `0x195C` stores current buttons in the low byte and new press edges in the
high byte. The RAM edge at 9080 precedes the input-log onset at 9081 by one
image. [C] The source call order explains why this need not be a confirm:
the ROM grid loop checks the A edge at `0xD31E` and calls `0xD53C` at `0xD426`.
The prompt then waits at `0xD570`, samples at `0xD57C`, and checks for a new A
edge at `0xD5B4`. The continued hold has no new edge, so the prompt stays on
Yes and the record remains unchanged. The visible prompt at
9080/9081 and through 9160 is consistent with the US-ROM input path. This
replaces the earlier conclusion that the capture conflicts with that path.
The missing ROM/build identity still prevents treating it as verified US
Genesis footage. It shows neither a checkout nor a field return.

## Doorway and Karma input audit of the expanded capture (2026-10-27)

The 620 snapshots keep `FFFBF8/FFFBF9 = 0x02/0x02`, the current scene/event
pair, from frame 8541 through 9160. The ROM copies the scene byte from
`FFDDC4` at `0x03084..0x0308A`; the selector at `0x1F086` consumes the pair.
The capture therefore stays in Scene 2/event 2 throughout the Stoker pages,
room payment, Hotel Options, and Karma child. The doorway is the Scene-2
terrain collision record 91 at `0x173138` (property `0xC2`, event 2); it does
not load a motel interior scene. The route resolves through the Stoker arrival
record at `0xF4AA0`, the room-payment command `0x06`/handler `0x21458`, and
Hotel Options at `0xCF38`; its Karma child is `0xD024`.

`inputs.txt` records B for four frames at 8621 and Up for 200 frames from
8641. It then records A holds of 12, 12, 32, 5, 4, and 17 frames at 8751,
8767, 8807, 8847, 8891, and 9001; a four-frame Left hold at 9041; and a
four-frame A hold at 9081. The long Up hold overlaps the first Stoker pages,
while the input log and RAM show the same-scene service route rather than a
door-to-interior transition. The active conversion's expected path is the
Stoker interaction and Hotel menu, both still in Scene 2; its field-Hotel QA
exercises that path from ordinary input.

At frame 9040 the active record is Joshua's Human/Shaman preset: race index 0,
class index 1, Body `+0x77 = 2`, and runtime Karma `+0x8D = 20`. The selected
Body preview is 3 at cost 3. The capture logs Left at the upper-left grid
boundary; `FFF0D0/FFF0D1` remain `0/0`, and the full 9040/9041 images are
identical. This matches `0xD01C`: Left at column zero leaves the selected
Body cell unchanged.

The A input logged at 9081 appears as `FFDD22 = 0x4040` at frame 9080, with
`FFF0DC = 0` (default Yes); at 9081, the low held byte remains `0x40` while the
high new-press byte is clear. Body and Karma remain 2 and 20, and the prompt
stays visible through frame 9160. In the source path, `0xD01C` consumes the
grid A edge at `0xD31E` to call `0xD53C` at `0xD426`; the prompt routine then
waits and samples at `0xD570/0xD57C` and checks new A at `0xD5B4`. Since
`0x195C` publishes only newly pressed buttons in the high byte, continuing to
hold A does not accept Yes. A release followed by a distinct A press is
required to reach the write at `0xD3EA`. The unchanged record and
persistent prompt therefore fit the source behavior. The capture's missing ROM
hash or emulator identity still limits it to an external comparison, not
verified US Genesis footage.

The Hotel profile QA now repeats all four direction boundaries for each of
the thirteen ROM profiles and checks the full 320x224 frame, cursor, preview,
race/class, attributes, skills, auxiliary/Karma, cyberware flags, and status
message after each rejected direction. Its supplied-capture input path holds
A for four frames from the grid: that edge opens the default-Yes prompt, held
frames do not buy, and a distinct A press after release applies one Body
increment and Karma debit. Debug and Release each pass; all 489 profile BMPs
and the `profile-cells.tsv` manifest match by SHA-256.

### Human/Shaman value cross-check against the supplied frame

**[C]** Capture frame `e_09040.png` identifies Joshua as Human/Shaman and shows
Body 2 with the preview `Upgrade to 3`, `Cost 3`. The ROM preset record at
`0x72B1A` has Body 2 at `+0x77` (the next three stat bytes are `4,1,2`). The
source-backed profile permutation row for `joshua-gator-shaman`, column 0,
cursor row 0, independently records current 2, next 3, cost 3, and writes
`joshua-gator-shaman/actual.bmp`.

**[H]** This is a class/race-specific value and page-layout match between the
supplied capture and the native profile audit. The external frame shows Karma
20 while the profile permutation fixture shows Karma 0, and the archive does
not identify its ROM or emulator build. It supports the displayed Body
preview/cost for this preset, not exact whole-frame fidelity.

### Karma grid and held-confirm visual sequence

**[C]** Pixel hashes of the supplied PNGs show one unchanged full-frame state
from frames 9040 through 9079, a prompt transition at frame 9080, then one
unchanged prompt state from frames 9080 through 9160. The Left press logged at
9041 leaves the upper-left Body selection unchanged. The A edge opens the
default-Yes prompt; its continuing hold produces no visible cursor, bezel, or
text blink during the 81-frame prompt interval.

The ROM path matches those states: the grid loop at `0xD31E` dispatches A to
`0xD426`, then `0xD53C` draws the prompt once and clears `FFF0DC` to select
Yes. Its `0xD570` loop waits for VBlank, latches and samples input, moves the
cursor only for Up/Down edges, and checks a fresh A edge at `0xD5B4`. The held
A no longer has a high-byte press edge after the grid action opened the prompt,
so Body and Karma remain unchanged. This is a visual and input-path match for
the captured hold, not a verified US-ROM timing reference: the package has no
ROM or emulator identity.

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
8733. The next source path at `0x1F246` loads the title-font stream, the nine
menu grids, the mode-specific profile-pattern stream, then its sprite stream
and text prompt. The captured RAM ring change from frames 8732 to 8735 is
bounded to the first 1,952 bytes, matching the Stoker pattern stream header
`0x079F + 1`; the frames reveal the border, portrait, then text in that same
order. This connects the later interval to source-ordered resource work rather
than an identified idle timer. The supplied archive has no ROM/build identity
or capture method, so the 40 black images remain an observation and do not
establish the original game's exact displayed-frame timing. See the detailed
[doorway transition trace](motel-doorway-scene-entry.md#supplemental-recording-reviewed-2026-10-27).

### Hotel Options idle cursor and bezel in the supplied capture (2026-10-28)

**[C]** The supplied `e8_karma_spend` input log has no input between the
four-frame A hold at 8891 and the next logged A hold at 9001. Across the fifty
320x224 captures `e_08950.png` through `e_08999.png`, pixel hashes show one
unchanged options-panel state: `Hotel Options`, the selected `Karma Screen`
arrow and label, and `Check Out` remain identical. The full frames have 33
distinct images because the only changed pixels are in the lower bezel; all
397 pixels that differ from frame 8950 lie within `(44,202)..(226,214)`.
There are 36 changed adjacent-frame steps and 13 repeats in this sample.
RAM snapshots keep `FFF0DC=0` (Karma Screen selected) and `FFDD22=0` (no
controller state) throughout frames 8950..8999. `FFDF88` advances from
`0x219B` to `0x21CC`, an observed 49-count increase across the 49 intervals.
Frame 9000 already shows the transition and the RAM input word is `0x4040`,
so it is excluded from the idle-screen pixel comparison; the external input
log places the A hold at 9001.

The ROM path accounts for the separation. `0xCF38` prepares the Hotel Options
display; `0xCF6C` clears `FFF0DC`, `0xCF7E` draws the cursor through `0xC376`,
and the loop at `0xCF82` waits for VBlank. `0xCF94` then calls `0xE660`,
which selects a random display-grid pattern and writes it through `0x1232`.
The `0xCF98..0xCFCE` direction checks redraw the cursor only after directional
input, while A dispatches the selected option at `0xCFD2`. The stable label
and arrow with changes confined to the bezel agree with these distinct paths.

**[H]** This is a capture-to-source consistency check, not a Genesis timing
reference: the archive has no verified ROM or emulator identity. The RAM
counter and image cadence do not establish the source VBlank rate, and the
recording does not provide frame-synchronized sound evidence.

## Independent Section-11 hotel and Karma sequence (2026-10-28)

The same longplay folder used by the package-route audit contains a later
Stoker and Hotel visit:
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_11_source_1620-1920/`.
The sampled 320x224 frames show the sequence below. They provide real-game
visual evidence for this path, but no controller log or RAM snapshot.

| Frame | Visible state and ROM link |
|---|---|
| `002430` | Stoker's Coffin Motel description and manager portrait. This matches the scene-2/event-2 Stoker content reached from record 91 at `0x173138`; the video does not expose the active scene/event bytes. |
| `002460` | Manager dialogue and the “place to crash” / “talk later” choices. |
| `002490` | A room is offered for 20 nuyen, with “Easy money, chummer. I'll pay.” selected. ROM command `0x06` reaches payment handler `0x21458`. The wallet debit is not visible in this capture. |
| `002520` | The paid-room message (“Hope you brought your own towels… Enjoy your stay.”) matches the page selected by the room-payment path at `0xF5AB8`. |
| `002550` | The Hotel status page displays “A steaming mug of soykaf would be just wiz about now,” matching the extracted `hotel.status.recovery.2..4` text. |
| `002580` | Hotel Options appears with “Karma Screen” selected over “Check Out,” matching the `0xCF38` options handoff. |
| `002610` | The Hotel Karma grid shows Body 3 and an unaccepted preview of 4 at cost 4, with Karma 11. |
| `002640` | Willpower 5 is selected; the preview is 6 at cost 6, and Karma remains 11. |
| `002670` | Willpower displays 6, Karma is 5, and the page reports the maximum level. Human Willpower's ceiling is 6 in the race table at `0xC22A6`; `0xD3EA`'s purchase path and `0xD82A` preview rules explain the visible `5 -> 6` and `11 -> 5` changes as one accepted purchase. |
| `002700` | Throwing remains 0 with a preview of 1 at cost 2 and Karma 5; this is a visible preview, not evidence of a second purchase. |

The transaction is strongly source-consistent, although the footage does not
show the input edge or live record bytes. Body stays at 3 and Karma stays at
11 after its cost-4 preview, while the later Willpower and Karma values
change by exactly the rank/cost amounts. The final Throwing preview leaves
Karma unchanged. See the [Attributes/Skills profile audit](attributes-skills-profile-permutation-audit-2026-10-23.md)
for the complete profile and race-ceiling tables. The Section-11 Joshua
Shaman has progressed values that differ from the ROM starter template; the
capture does not show how the earlier changes occurred.

The matching native checkpoints are `paid-dialogue/actual.bmp`,
`options/actual.bmp`, and `karma-spend/actual.bmp` under the Release
`qt_runtime/field-hotel-qa/` output. They share the page structure and menu
text; their seeded Samurai/profile values differ, so they are not pixel-equal
screenshots of the longplay state. This is a cross-route visual comparison,
not a claim that the native renderer has been proven against every original
pixel.

## Nine-question visual review

1. **Does it look correct? — Partial.** Independent section 03 footage
   supports the Stoker doorway/fade/panel family. Section 11 shows the room
   offer, paid message, Hotel status/options, and a Karma purchase sequence
   whose displayed stat and Karma changes match ROM rules. These frames
   support the page presentation and behavior, while exact profile-specific
   pixel fidelity remains open.
2. **Does the visible look match behavioral state? — Yes for the tested
   route.** Ordinary input selects collision `0xFFC2`/event 2, the first-visit
   manager offers rent, leave, and Michael's package, payment changes the
   wallet from 20 to 0, the paid-message
   dismissal opens Hotel status, A enters Hotel options and the distinct
   spending page, and the confirmed upgrade changes Body/Karma to 2/48.
   `physical-route.tsv` records the route states and input milestones.
3. **Are the visuals themselves correct? — Partial.** Section-11 frames show
   the same Hotel options, dialogue, and Karma-page structure as the native
   checkpoints, including the selected row and cost/result text. The longplay
   runner state differs from the seeded native QA profile, so this review
   does not claim a pixel-equal full-screen comparison for the same values.
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
   earlier dialogue or Hotel entry is silent. The supplied `audio.wav` is
   stereo 16-bit PCM at 44.1 kHz with 457,022 samples per channel. Nonzero PCM
   ends at sample 126,763 (2.874 seconds); samples 126,764 through 457,021
   are exactly zero, while the numbered images continue through
   `e_09160.png`. The archive provides no audio/video synchronization or
   capture metadata, so this cannot distinguish a game-side audio stop from a
   truncated or silence-padded audio capture. It does not establish the exact
   onset of a menu sound or that the later Hotel pages are silent.
6. **Are the other visible parts of the screen also correct? — Reviewed.**
   The full 320x224 sequence covers the field/HUD, doorway transition, shared
   dialogue panel, Hotel status/options, Karma prompt/result, checkout, and
   returned field. The Hotel screen removes the field HUD; the Karma page does
   not retain Hotel controls; return restores the field. Other unrelated
   scene defects are not closed by this route.
7. **Do multiple frames support the interpretation? — Yes for the native
   route; partial in the independent longplay.** The native capture includes
   separate input moments, idle frames, page changes, the full
   fade-to-black/field-return interval, and the later command overview.
   Section 11 independently shows room offer/payment, Hotel status/options,
   a Willpower preview, a matching stat/Karma change, and a later Throwing
   preview. It has no controller log or RAM record and does not show checkout.
8. **Does the new UI replace the old UI cleanly? — Yes in the tested route.**
   Hotel status replaces the field/HUD; options replace status; the Karma
   spending page contains no old Hotel options; Hotel options are redrawn on
   child return; checkout removes the option rows; and the field-only redraw
   equals the immediate and 30-frame returned full-screen images. No old
   buttons or graphics remain over the next page.
9. **Was the whole time window reviewed? — Yes for the native QA route;
   partial for the longplay.** The QA sequence begins at initialized New Game
   field spawn and runs continuously at one image per controller/update frame
   through checkout, field return, and command-overview open/close. All 414
   full-screen images were reviewed in chronological contact sheets; frames
   0218, 0233, 0235, 0317, 0318, 0321, 0381, and 0411..0413 were inspected at
   native size. Section-11 longplay states were sampled at the named frames
   above; this turn did not review every frame in that independent sequence.

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
  and dialogue panel. Section 11 adds a room offer/payment, Hotel Options, and
  a source-consistent Human/Shaman Willpower purchase. The separate
  `e8_karma_spend.zip` still has no ROM/emulator identity and is used only for
  its own input/RAM correlation.
- **Reported defect resolved:** no code defect was reported or changed in this
  audit. It adds independent source-to-visual evidence; exact Hotel pixel
  fidelity and the independent route's input state remain open.
