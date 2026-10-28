# Package run origin and delivery event

Status: **partial**. The run logic is connected to the field-exit
orchestrator's first dispatch stage. Section 11 of the longplay now provides
an independent offer, payment prompt, apparent acceptance, and return-to-map
sequence; it does not follow the accepted run through both selected map
events. The script-runtime test proves event matching and once-only payout;
it does not prove that whole playthrough.

## ROM trace and host behavior

- ROM `0x1E69C` selects the run type and stores its source progress byte.
- `0x1EEDC` dispatches `FFFBED` through the six-pointer table at `0xF3E56`.
  Entries 1 and 2 both point to the shared handler at `0x1EF2C`.
- At the origin, `0x1EF2C` compares `FFFBF8/FFFBF9` to `FFFBEE/FFFBEF` and
  sets bit 0 of `FFFC1C`, which gates the later destination comparison. It
  sets alternate result `FFF0D5` to `0xFF` and bit 9 of `FFFC22` for run type
  2.
- After that bit is set, the handler compares the current tuple to
  `FFFBF1/FFFBF2`. For type 2 it sets `FFF0D5` to 1 and clears `FFFC22` bit 9.
  Common payout at `0x1F00A` adds `FFFBF4` to funds and adds `FFFBF3` to the
  `+0x8D` byte of each of the three runner records.
- The type-2 path contains no sound request, actor spawn, or animation start.
  The Qt adapter now calls Lua at the `0x1EEDC` dispatch stage, applies the
  typed alternate result/active flag, then applies the payout and progress
  bytes. It does not add a panel, sprite, animation, or audio request.

## Independent Section-11 offer sequence

The longplay folder is
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_11_source_1620-1920/`.
These 320x224 frames show the type-2 package offer and its presentation path:

| Frame(s) | Independent visual evidence |
|---|---|
| `frame_000000.png`–`frame_000055.png` | The first page stays pixel-identical in the extracted 30-fps frames. It says to meet the Halloweeners in Redmond, collect a package, take it to Little Chiba, and expect payment upon safe delivery. The dialog shell, speaker portrait, and `< MORE >` prompt are visible. |
| `frame_000075.png`–`frame_000105.png` | The payment page shows base 40 and negotiated 55 with the game's dollar glyph, plus the A/Yes and B/No choices. Frames `000087`–`000101` are pixel-identical. |
| `frame_000120.png` | The next page says Mr. Johnson records the run on the Pocket Secretary and downloads its information. This supports an affirmative path; the capture has no controller log to prove which input selected it. |
| `frame_000134.png`–`frame_000142.png` | The dialog and field image darken. Frames `000143`–`000175` are full-frame black in the extracted video. |
| `frame_000176.png`–`frame_000180.png` | The field view returns from dark to full brightness. Frame `000180.png` matches the visible Jump House layout in the ROM-derived `assets/field/maps/mode-13.bmp`. |

The return-map match and ROM field records connect the final image to Scene 13:
the mode-13 map profile is based at `0x1B3C06`, while event-30 records at
`0x1B3CF8`/`0x1B3D02` route through `0xB7B8` to the Johnson handler at
`0x20990`. This visual match identifies the location; it does not expose the
run-active RAM fields or prove that the player later visits either selected
contract tuple.

### The displayed payment and Joshua's profile

Context-zero offer row 4 is the six-byte record at `0xF3ED4`:
initializer 2 (package delivery), progress delta 1, base 40, and negotiation
scale 1 (raw bytes `02 01 00 28 00 01`). `0x1E69C` reads the first runner's
Negotiation field at `FF018C` and computes `base + 5 * roll`, with the roll
below `scale * Negotiation + 1`. The offer path sends base and negotiated
amounts through the grouped-decimal formatter `0xC216` at `0x21526`/`0x2153C`.
The independent Section-06 frame `002550` identifies the longplay runner as
Joshua Gator Shaman; ROM profile `0x72B1A` has Negotiation 3 at `+0x8C`.
Those inputs make 55 a source-consistent display: `40 + 5 * 3 = 55`, within
the four possible rolls `0..3`. The frame does not show RAM or RNG state, so
the profile-to-offer join and roll 3 are derived from the two screenshots and
ROM data, not directly observed state. Race and class have no direct term in
the payment calculation; the active profile's Negotiation value supplies the
relevant difference.

During frames around `000070`–`000090`, individual green glyph pixels vary
between extracted frames while the same page and choices remain. The ROM path
at `0x1F3C4` renders wrapped text through `0xC134`/`0xC168`, then enters the
VBlank/input wait at `0x1F502` and samples the controller at `0x1F51A`; no
source blink counter has been identified for these glyphs. Treat the partial
glyph dropout as **[H]** capture/compression or sampling behavior, not a
confirmed in-game text animation. The 30-fps
extract supports the order and broad fade/black/return sequence, not exact
60-Hz timing. Audio, the live run flags, decline-path visuals, and the later
pickup/delivery/result sequence remain open.

## Nine-question visual check

1. **Does it look correct? - Unknown for the event result screens.** No
   event-result screen is captured yet. The existing package offer layout is
   supported by independent section-03 frames 003000, 003150, 003300, and
   003450; those show the selected package route, payment prompt, and accepted
   confirmation.
2. **Does the visible look match behavioral state? - Partial.** The runtime
   test checks wrong-location, destination-before-pickup, origin, an
   intermediate tuple, destination, and duplicate destination. It confirms
   state/reward outputs, but the event callback has not been driven in a
   captured field session.
3. **Are the visuals themselves correct? - Partial.** This change adds no
   drawing code or asset changes. The full-screen Release `actual.bmp` from
   `field-jump-house-entry-qa` was compared with section-03 frames 001350,
   001365, and 001380. The blue grid-like lines remain present across those
   independent frames, so they are part of the scene, not a newly drawn debug
   overlay. The runner placement differs slightly in the existing QA capture.
4. **Should anything animate? - Yes: no run-event animation is called by the
   type-2 ROM path.** The grid remains visually stable in reference frames
   001350/001365/001380 while nearby actors and the runner change pose/position.
   No new animation timer or actor was added for package pickup/delivery.
5. **Does it have sound? - Yes: no call traced.** The type-2 origin/destination
   branches at `0x1EF2C` and common payout at `0x1F00A` contain no sound request;
   this change adds no audio mapping or playback call. This is source-call
   evidence, not a playback-duration measurement.
6. **Are the other visible parts correct? - Partial.** Whole-screen Release
   captures `field-jump-house-entry-qa/actual.bmp`, `gunderson-root.bmp`,
   `field-ares-collision-shop-qa/ares-shop-category.bmp`, and
   `field-stoker-collision-to-dialogue-qa/actual.bmp` were reviewed after the
   callback moved. The screens show no new controls or stale overlay. These
   all four Debug/Release pairs are byte-identical by SHA-256. These routes have no active package offer, so they do not cover the event result.
7. **Do multiple frames support the interpretation? - Partial.** Independent
   frames 001350/001365/001380 support the field-map layer; 003000/003150/
   003300/003450 support the package-offer dialog sequence. No supplied frame
   was identified for the player visiting the selected pickup and destination
   as one run.
8. **Does new UI replace old UI cleanly? - Yes for this change.** The event
   callback draws no UI and creates no overlapping controls. A captured
   pickup/delivery transition is still needed to check the following generic
   field prompt after `FFF0D5` changes.
9. **Was the whole affected time window reviewed? - No.** The selected run
   offer through pickup, delivery, payout, and the first stable field frame
   has not been captured continuously. Keep this hill partial.

## Internal verification

`shadowrun_gameplay_script_runtime_qa` passes in Debug and Release. It verifies
that the route generated from the ROM offer data is origin `area_2/location_4`,
destination `area_2/location_5`, negotiated payout 45, and progress delta 1;
pickup arms the package, only the later selected destination completes it, and
the repeated destination cannot pay again. Four focused whole-screen field
route CTests also pass in Debug and Release. They guard adjacent menus and
field screens but do not exercise an active package run.
