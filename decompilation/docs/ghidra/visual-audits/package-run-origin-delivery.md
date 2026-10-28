# Package run origin and delivery event

Status: **partial**. The run logic is connected to the field-exit
orchestrator's first dispatch stage. Section 11 shows the offer, payment
prompt, apparent acceptance, a Halloweener package-handoff candidate, and the
Little Chiba package-payment page. The text and ROM location tables align with
the selected origin/destination, but the footage has no controller or RAM
telemetry. In particular, the exact handler and live run state behind the
Halloweener page and the actual wallet/progress writes remain unverified. The
script-runtime test proves event matching and once-only payout; it does not
prove that whole playthrough.

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

The later frame `000180` and ROM field records connect that image to Scene 13:
the mode-13 map profile is based at `0x1B3C06`, while event-30 records at
`0x1B3CF8`/`0x1B3D02` route through `0xB7B8` to the Johnson handler at
`0x20990`. This visual match identifies the location; it does not expose the
run-active RAM fields or prove that the player later visits either selected
contract tuple. The later Halloweener and Little Chiba pages correlate with
the two locations named by the offer, but still do not reveal the active tuple
or run-handler state.

### Later pickup and delivery sequence

The same Section-11 extraction continues beyond the initial 180-frame offer
sequence. These later frames show the two named locations' package-related
pages, with a large gameplay interval between them:

| Frame | Observation and source link |
|---|---|
| `frame_001500.png` / `frame_001560.png` | The portrait is a Halloweener. The text says the player is the “mule” sent for them, gives them the package, and tells them to sit. The context-zero offer names “see the Halloweeners,” which maps through the source location table to area 2/location 9; Scene 2 record 94 at `0x173156` publishes event 9. The footage does not show the current tuple, and the exact text/handler has not been matched to a ROM text descriptor. Treat this as a pickup candidate, not proof of the type-2 branch. |
| `frame_001650.png` / `frame_001710.png` / `frame_001800.png` | Black follows the dialogue, then field gameplay resumes. This sequence gives order only; 30-fps extraction does not resolve the source's 60-Hz timing. |
| `frame_002100.png` | The Little Chiba salesman accepts the package and hands over a credstick. The prose exactly matches the extracted `field.mode.little_chiba.package_payment` asset from ROM stream `0x117718`. ROM event 1 at Scene 2 record 93 (`0x17314C`) selects resource scene 26; the mode-exit selector at `0x1F830` selects the payment page when `FFF0D5` is positive. This is strong page-to-source correspondence, but the capture does not expose the active tuple, `FFF0D5`, or the wallet delta. |
| `frame_002400.png` | Full-frame black follows the payment page. The amount credited and progress-byte writes are not on screen. |
| `frame_002430.png`–`frame_002700.png` | Later frames show a Stoker room offer/payment, Hotel status/options, and the Human/Shaman Willpower upgrade sequence. These are subsequent gameplay states, not a directly observed consequence of the package payout; the [Stoker Hotel route audit](stoker-room-to-hotel-karma-route.md) traces their source behavior. |
| `frame_003300.png` | The field view visually matches the ROM-derived Jump House `mode-13.bmp`. It appears later in this long gameplay sequence; ordinary travel, active-run return behavior, and the exact transition are not distinguishable from the video alone. |

The Halloweener origin page is not the currently converted type-3 gang-entry
passage at `0xFE0DE` (“Slot off, frag-face!!!”). Scene 2/event 9 can enter that
type-3 path for a gang-contract offer, so the screenshot's different “mule”
dialogue cannot be assigned to that handler by location alone. The package
offer text and later Little Chiba payment page support a package-route reading,
but an exact origin-dialogue pointer or a capture with live ROM state is still
needed to close the source link.

### The displayed payment across runner presets (2026-10-28)

**[C]** Context-zero offer row 4 at `0xF3ED4` is the six-byte record
`02 01 00 28 00 01`: package-run initializer 2, progress delta 1, base payment
40, and Negotiation scale 1. At `0x1E69C`, the first random draw selects one of
eight offer rows for the current context. After the selected row is known, ROM
reads the first runner's live Negotiation byte at `FF018C` (`FF0100 + 0x8C`),
multiplies it by the row's scale, adds one, and passes that bound to `0xD3A`.
The random helper advances its 16-bit seed at `0xD44`, multiplies the seed by
the requested bound, and returns the high word. Its result is therefore in
`0..(scale * Negotiation)`. ROM multiplies that result by five and adds the
base at `0x1E704..0x1E70C`, storing the negotiated amount at `FFFBF4` and the
unmodified base at `FFF0C0`. `0x21526` formats `FFFBF4` at `$C51E`; `0x2153C`
formats `FFF0C0` at `$C1C0`.

The raw starter records produce these rank-dependent outcomes for that row.
Race and class identify the preset, but neither is read by the payment
calculation; the live `+0x8C` Negotiation rank controls the range.

| Source profile / ROM | Race / class | Negotiation `+0x8C` | Possible negotiated amounts |
|---|---|---:|---|
| Joshua Samurai `0x7291A` | Human / Samurai | 1 | 40, 45 |
| Joshua Decker `0x72A1A` | Human / Decker | 2 | 40, 45, 50 |
| Joshua Gator Shaman `0x72B1A` | Human / Shaman | 3 | 40, 45, 50, 55 |
| Rat Shaman `0x72C1A` | Dwarf / Shaman | 1 | 40, 45 |
| Troll Samurai `0x72D1A` | Troll / Samurai | 0 | 40 |
| Human Mage `0x72E1A` | Human / Mage | 2 | 40, 45, 50 |
| Dwarf Decker `0x72F1A` | Dwarf / Decker | 0 | 40 |
| Bear Shaman `0x7301A` | Orc / Shaman | 0 | 40 |
| Elf Decker `0x7311A` | Elf / Decker | 0 | 40 |
| Human Samurai `0x7321A` | Human / Samurai | 4 | 40, 45, 50, 55, 60 |
| Elf Mage `0x7331A` | Elf / Mage | 4 | 40, 45, 50, 55, 60 |
| Elf Decker `0x7341A` | Elf / Decker | 0 | 40 |
| Human Samurai `0x7351A` | Human / Samurai | 0 | 40 |

These are preset-start values, not fixed class/race payments. The Hotel Karma
skill route can raise Negotiation to rank 12 for every class; Sorcery is the
only class-restricted skill. At live rank 12, this offer can display any
five-unit step from 40 through 100. The fixed-width random helper establishes
the possible range, not a probability claim about each outcome for a particular
seed.

**[C]** Section-11 frames `000075..000105` show base 40 and negotiated 55. A
roll of 3 produces 55, so the screen is source-consistent for a live Negotiation
rank of at least 3. Section-06 frame `002550` identifies a Joshua Gator Shaman
presentation, and the ROM starter at `0x72B1A` begins with Negotiation 3; the
user-supplied `e8_karma_spend` snapshots also show live `+0x8C=3` in their
separate recording. **[H]** None of those artifacts supplies RAM from the
Section-11 offer frames, so its exact live skill rank and RNG result remain
unobserved. The value 55 is valid for the rank-3 starter, but the recording
alone cannot establish that this was the active profile state at that moment.

During frames around `000070`–`000090`, individual green glyph pixels vary
between extracted frames while the same page and choices remain. The ROM path
at `0x1F3C4` renders wrapped text through `0xC134`/`0xC168`, then enters the
VBlank/input wait at `0x1F502` and samples the controller at `0x1F51A`; no
source blink counter has been identified for these glyphs. Treat the partial
glyph dropout as **[H]** capture/compression or sampling behavior, not a
confirmed in-game text animation. The 30-fps extract supports order and broad
fade/black transitions, not exact 60-Hz timing. Audio, the live run flags,
decline-path visuals, exact payout/progress writes, and the Halloweener page's
source text/handler remain open.

## Nine-question visual check

1. **Does it look correct? - Partial.** Independent frame `002100` shows the
   Little Chiba package-payment page and matches the extracted text and
   source-selected positive-result page. No visible wallet/Karma value
   confirms the actual payout.
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
7. **Do multiple frames support the interpretation? - Partial.** Section 11
   shows the offer naming Halloweeners and Little Chiba, a later Halloweener
   package-handoff candidate, and the exact Little Chiba payment page. The
   extracted video has no controller or RAM record tying those later pages to
   the accepted offer's live state.
8. **Does new UI replace old UI cleanly? - Partial.** The captured Little
   Chiba payment page replaces the previous field view. The sampled black
   frame after it establishes a clear transition, but the subsequent route
   and post-payout state are not identified.
9. **Was the whole affected time window reviewed? - No.** The supplied 30-fps
   sequence includes candidate pickup and delivery pages, but the complete
   route, input branches, stable post-payout field state, and source state
   changes have not been recovered. Keep this hill partial.

## Internal verification

`shadowrun_gameplay_script_runtime_qa` passes in Debug and Release. It verifies
that the route generated from the ROM offer data is origin `area_2/location_4`,
destination `area_2/location_5`, negotiated payout 45, and progress delta 1;
pickup arms the package, only the later selected destination completes it, and
the repeated destination cannot pay again. Four focused whole-screen field
route CTests also pass in Debug and Release. They guard adjacent menus and
field screens but do not exercise an active package run.
