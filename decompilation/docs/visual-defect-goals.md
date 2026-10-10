# Shadowrun visual and gameplay defect goals

This is the goal tree for the user's itemized defect report in the 2026-10-05
conversation. A user report creates a goal to investigate and solve; it does
not by itself prove that the current build still reproduces the defect. Each
goal therefore keeps these labels separate: reported, reproduced in the
current build, implemented, independently checked, and resolved.

## Completion gate for every goal

Before changing a screen or behavior, trace the exact US ROM path: entry,
controller/cursor and RAM state, animation and draw sequence, sound request,
and exit/return path. Use Ghidra as a cross-check. Use the numbered independent
gameplay breakdown under `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`
as the visual/audio reference; cite frame numbers and review whole 320x224
frames. A missing matching reference stays an explicit limitation.

Every completed screen/mechanism goal must have typed native state, a
regression CTest, full-screen CLI dumps of entry/active/idle/confirm/cancel/
return states as applicable, and Debug and Release validation. Review every
dump across the entire screen, not just the edited control. A child page must
replace or hide its parent controls before drawing. For animated elements,
record the counter, cadence, loop duration, and start/pause/stop conditions.
For audio, identify the ROM event/resource, verify start and stop behavior,
and record measured duration; an alias lookup alone is not playback evidence.

Apply the project's nine questions from
[`visual-verification-checklist.md`](visual-verification-checklist.md) to
each changed slice. Do not mark a reported defect resolved from a same-source
compositor, internal screenshot, or passing test alone. Record absent
independent references as absent.

### Latest full-screen verification pass

The 2026-10-09 Release intro recapture contains 6,782 consecutive screens and
2,032 distinct full-screen images. Each distinct state was reviewed in
chronological sheets; exact repeats were accounted for by per-frame hashes
and metadata. The sweep revisited the magician effect stop, transparent ambush
patch, field handoff, and Stoker dialogue. See the [complete recapture review](visual-audits/full-intro-capture-2026-10-09-distinct-frame-review.md).

Opening ambush, death-layer cleanup, startup integration, Scene 2 motel-street
collision, Start-menu Attributes/Skills, gameplay-script runtime, field-text
catalog, and command-root screen CTests pass in Debug and Release. The
collision route confirms that the runner can leave the tested motel wall with
other input, but it does not reproduce a permanent lock; FIELD-02 remains open
for the exact stuck sequence and independent Genesis collision evidence. The
Karma cursor focus still does not open the view by itself.

## Itemized report and atomic goals

The rows below preserve every distinct issue in the user's pasted report.
`Reported` means the player observed it; it is not a claim that the current
checkout still reproduces it. Current-build reproduction and independent
Genesis comparison remain separate evidence fields. Detailed ROM paths and
shared acceptance gates are in the matching goal cluster below.
`TITLE-04` and `SEC-JOHNSONS` are additional findings from the current
full-screen/route review; `MAGIC-SPLIT` records the user's later screen
clarification. None changes the original report's itemization.

| Goal | Reported issue, itemized | Evidence check and closure condition |
|---|---|---|
| `BOOT-01` | The shock figure has no sound when it shocks the SEGA logo, and the same effect plays at the wrong time when the Decker intro starts. | **Resolved for sound presence and event placement.** ROM/Z80 tracing distinguishes the boot caller's `0x16` (`FF 12`, stop resource 22) from `0x5C` (`FF 10`, start table-2 resource 92). Resource 22 also has an `FF 10 16` start alias for other callers; the boot callback follows its own service operation. The full Release intro issues `0x5C` during the SEGA lightning sequence, not at Decker entry. Independent section 01 frame 120 shows the lightning; its audio supports resource 92. See [the source and playback audit](visual-audits/boot-shock-audio.md). The previous six-full-file-player mix was later found fragmented and is tracked separately as `BOOT-06`.
| `BOOT-02` | The flame decal remains on the right after the logo is fully revealed. | Reproduced in the pre-fix Qt full-screen title capture. The typed reveal stops at 38 columns: the last active frame shows the flame at title frame 165, the first frame after the 0x1A9E reveal branch hides it at 166, and settled frame 900 has no right-edge overlay. Debug/Release CTest checks projection state and whole-screen right-edge pixels. Independent section 01 frames 000372, 000384, and 000390 show the same active-then-absent transition. Resolved for the reported post-reveal persistence. |
| `BOOT-03` | The animated microchip/circuit background does not move upward. | **Resolved for the reported missing motion.** ROM `0x1D7A` initializes the Plane-B scroll and `0x26D0` advances it on the first title-loop VBlank, then every five VBlanks while the loop runs; Qt uses that typed offset to move only the circuit layer. Debug/Release startup QA checks the source cadence and whole-screen pixels at multiple offsets. Independent section 01 frames 000390→000396 show a two-pixel upward shift, and 000420→000450 show about twelve pixels; corresponding native frames 254→318 shift thirteen pixels. Full-plane wrap and the separate circuit brightness/texture mismatch remain open under `TITLE-03`. See [the title circuit scroll review](visual-analysis/qt-visual-audit.md#applied-review-title-circuit-vertical-scroll). |
| `BOOT-04` | The title circuit appears before the Shadowrun logo has rolled across. | **Resolved for arrival order.** ROM `0x1E4E` completes the 38-column reveal before `0x1C1E` begins three palette steps at countdown values 64, 32, and 0. Typed palette-step state keeps the circuit black through the reveal. Debug/Release whole-screen QA checks title frames 188/189; independent section 01 frames 000384/000390 support logo-only then circuit-visible order. Brightness and texture remain separate under `TITLE-03`. See [title circuit arrival audit](visual-audits/title-circuit-arrival.md). |
| `BOOT-05` | The black interval between the FASA fade-out and the first Shadowrun logo column was missing from the defect inventory. | **Inventory gap closed; no current rendering defect reproduced.** The US ROM title loop initializes `DD2C=0x5A` and calls the logo-column routine only after that countdown becomes negative, so all 90 delay updates are expected to remain black and the first 8-pixel column appears on update 91. Qt Debug/Release boot QA checks the full 320x224 image across the two post-FASA handoff frames and every title-delay update, verifies typed reveal/palette/sprite state, then saves the first-column frame. The complete Release capture shows native frames 540-650 black and the first mark at 651. Independent section 01 frames 000300/000330 show black and 000360 shows the partial mark; 30-fps timing is not treated as VBlank evidence. See [boot-to-title black interval audit](visual-audits/boot-title-black-interval.md). |
| `BOOT-06` | The SEGA shock effect sounds fragmented and does not match the complete cue in the longplay. | **Resolved for the sampled shock cue.** Resource 92 is a complete 44.1 kHz stereo render (2,750 ms total; about 0.8 s audible), not an individual YM voice stem. The ROM still emits all 60 `FF 10 5C` requests every two VBlanks; the host records each request and plays this complete extracted cue once per pre-title sequence. Debug/Release audio CTest checks one active player across the request burst and replay on a new sequence. Four overlapping 200 ms longplay windows sampled 100 ms apart match a single resource-92 render at NCC 0.9479, 0.9957, 0.9831, and 0.9529; fresh Release process-loopback matches the same asset at 1.0000 across four consecutive 100 ms windows. The 30-fps recording bounds timing precision. See [the source and playback audit](visual-audits/boot-shock-audio.md). |
| `BOOT-07` | The Shadowrun logo rolls without its reveal cue, and the title theme does not start after the reveal. | **Resolved for title cue identity, ordering, and Qt playback.** ROM `0x1E4E` requests `0x5D` on even column indices `0..36` (19 source requests), which maps to resource 93; the Qt sequence plays the complete extracted render once. ROM `0x1C1E` requests command `0x00` after the `0x5A` completed-logo countdown, immediately before copyright; this maps to music resource 0. Debug/Release title-loop tests check all 19 requests and theme-before-copyright order. Audio CTest checks playback/durations, one active cue across the repeated requests, and new-sequence replay. A normal Release process-loopback capture matches resource 93 at NCC 0.91–0.99 and resource 0 at 0.96–1.00 across 400 ms windows. Independent section 01 frames 000342/000384/000420 support the logo-to-title order, and short 250 ms audio windows match the assets at NCC 0.831/0.852. See [the title logo and theme audit](visual-audits/title-logo-audio.md). |
| `TITLE-01` | The title's “PRESS START” label does not blink in and out. | Reproduced before the edit: the presenter set `title_prompt_visible_ = true` for both ROM variants. Resolved. ROM `0x1A9E` starts D7 at `0x0E10`, waits once per loop, then draws only when D7 is below `0x0C6C`; its low six bits select visible record A (`0x1F81`) or blank record B (`0x1F99`), both at `0xCA84` with palette `0x2000`. Qt now follows the selected typed text asset. Debug/Release typed and full-screen QA cover two cycles. Independent section 01 frames 000900/000906/000912 show visible/hidden/visible; their coarse 30-fps spacing supports blinking but does not establish absolute VBlank phase. Full review: [title prompt blink audit](visual-audits/title-prompt-blink.md). |
| `TITLE-02` | Pressing Start skips the transition and immediately opens New Game/Load Game. | Resolved for the reported Start route: ROM `0x1A9E` exits only on the Start latch (`0x8000`), then performs 17 palette passes after 34 VBlanks; the `0x075E` caller runs six command-`0x14` audio transactions over 18 more VBlanks before `0x59092` draws Start Options. Debug/Release QA captures every pass and hold frame, proves A alone does not exit, and checks the default New Game selection and clean destination redraw. Independent section 01 frames 001637-001643 show title-to-black and 001670-001674 show the options shell followed by text. The clip has no input/VBlank log; exact input phase remains open. Shell-to-text ordering is defended separately under `TITLE-04`, although exact 60-Hz duration is not measurable from this 30-fps capture. The caller's command-`0x14` resource/playback/duration remain unidentified. See [title Start transition audit](visual-audits/title-start-transition.md). |
| `TITLE-03` | The title circuit background is much brighter and denser green in the native screen than in the independent game capture. | Reproduced in matched whole-screen comparisons after correcting a mislabeled native frame. ROM tracing supports the extracted base palette and identifies a separate seven-phase light update; the compressed reference does not establish the correct brightness of thin circuit lines. Keep open until source-backed color/texture equivalence can be established without a speculative color transform. See [title circuit palette audit](visual-audits/title-circuit-palette.md). |
| `TITLE-04` | Found during full-screen verification: the Genesis capture shows the Start Options shell without text for frames 001670-001671, then shows the text and selector from 001672; the original Qt destination rendered the complete page immediately. | Resolved for the source-supported ordering. ROM `0x10B5A` calls `0x10918`; `0x10952` clears/fills the menu display buffers and `0x1095E` waits for VBlank before menu assets, grid (`0x590B6`), heading/choices (`0x590CC`), and cursor (`0x0C376`) are drawn. Typed native `start_options_shell` renders only the bezel/background for the source-frame checkpoint, then reveals the active menu. Debug/Release QA and CLI `actual.bmp` dumps verify shell-only pixels, default New Game selection, and clean return from empty Load. Independent section 01 frames 001670-001671 show shell only, then 001672-001674 show the active menu. The 30-fps capture supports order but not exact 60-Hz display duration or time spent transferring assets. See [title Start transition audit](visual-audits/title-start-transition.md). |
| `TITLE-05` | With no input, the title fades to black and the Qt game stalls instead of replaying the boot/title attract loop. | Implemented and internally defended against the US-ROM path: 3,601 no-input title updates, the caller's 18-VBlank audio service, a second 17-pass/34-VBlank fade, then a clean pre-title restart. Debug/Release QA checks the full-screen black hold, restart timing, recurring source shock request, and pixel-identical fresh/replay boot and title frames; see [title idle attract-loop audit](visual-audits/title-idle-attract-loop.md). The supplied independent longplay has no matching 3,601-update no-input loop, so this route lacks independent visual confirmation. The command-0x14 audio resource and audible duration also remain unidentified. |
| `OPEN-01` | Opening music starts on the second screen instead of when the opening starts. | **Resolved for the reported late start and stage-boundary audio behavior.** ROM `0x530EA` submits paired `FF 12 id`, then `FF 10 id`; the ROM-driven trace has 6 YM key-ons for `0x10`, none for `0x11`, and 146 for `0x12`. Qt maps `0x10` to stage 1 and `0x12` to stage 3, leaves source-silent `0x11` unmapped, and stops active host channels at the source `FF 16` boundaries. Debug/Release playback and source-order CTests pass. In a visible ordinary Release run, the window advanced through `title_ready` -> `start_options` -> `joshua_archetype` -> `opening_intro`; process-specific loopback captured the stage-1 waveform beginning at 11.538 s (opening phase timestamp 11.588 s), the stream at capture noise floor from 21.76 s through the stage-2 interval, and stage 3 beginning at 33.938 s and playing its 21.005 s one-shot. Aligned track windows score at least 0.99 for stage 1 and 0.93 for stage 3. Independent section 02 audio supports the track identities but not exact VBlank or stage-2 silence; the ROM/Z80 trace establishes the source-silent pair. The 90-second ROM probe found no within-stage restart. See [`visual-audits/opening-music-start.md`](visual-audits/opening-music-start.md). |
| `OPEN-02` | In the Gator Shaman route, the magic-user's head floats instead of staying attached to the character. | **Resolved for the reported head-to-body separation during the wilderness pan.** This is the Gator Shaman/magic-user, not the Decker. ROM 0x59D7C moves the wilderness plane offsets and head-list coordinates together; Qt now translates the named body layers by the typed source offsets. Debug/Release pan QA checks source-coordinate invariants and 1,924/1,924 body pixels at start, midpoint, and settled full-screen checkpoints. Independent section 02 frames 000480, 000510, 000570, and 000630 support the composition. The complete post-fix run is reviewed in [the full intro capture](visual-audits/full-intro-capture-2026-10-06-pan-fix.md); source and nine-question details are in [the pan audit](visual-audits/gator-shaman-head-pan.md). The Intro hill remains partial for other open defects. |
| `OPEN-03` | Michael's death image is superimposed over another image in the opening. | **Resolved for the stale left actor plate under the death pose.** US ROM `0x59F8E` fills the 17x16 `$0600` region with blank tile `$07FF` at elapsed `0x168`, then draws the terminal pose. The Qt controller now clears both persistent left-region layers before that image. Debug/Release whole-screen QA checks the clear at opening frame 1209, the complete remaining pose lifetime, and the tri-vid replacement. Independent section 02 frames 730-780 support the clear and later projectile motion. The complete 6,782-frame Release run and nine-question audit are in [the death-layer audit](visual-audits/michael-death-layer-clear.md). The separate terminal command `0x2E` selects a verified silent table slot and is closed as a non-defect under `OPEN-07`. |
| `OPEN-04` | The aircraft's rear propeller does not move. | **Resolved.** Raw US-ROM bytes at `0x5B100` gate the sprite while signed `DD08 >= -0x1B8` and select six source-backed groups from `0x5B180` every six frames; the groups repeat every 36 frames. The typed Qt pulse state draws them. Debug/Release full-screen QA checks all six groups, stable body pixels, and the repeated state. I reviewed the latest full 4,982-frame boot-to-field capture at 10 fps and all 219 consecutive full-screen frames in the propeller's active interval. Independent section 02 full frames 001065/001068/001071/001074/001077/001080 also show the changing rear strip. The old “ambiguous” interpretation was wrong; the reference supports motion but not exact VBlank phase. See [airport propeller animation audit](visual-audits/airport-rotor-and-aircraft-tail.md). |
| `OPEN-05` | The aircraft lands on the building incorrectly. | **Resolved for the sampled airport landing sequence.** ROM `0x5B1B6` keeps list `0x1DF3F4` as the aircraft's base art and selects alternate list `0x1DF3FE` only while computed screen Y is `0x110..0x11E`; phase index does not select the image. The host plan incorrectly assigned alternate art to phases 4-6. It now supplies base art for all seven table rows, leaving the existing typed Y-window rule authoritative. Debug/Release `shadowrun_ghidra_conversion_qt_title_intro_late_qa` checks the seven ROM `0x5B270` deltas/delays, event frames, positions, and variants. The full-screen `shadowrun_ghidra_conversion_qt_opening_landing_qa` captures 20 event/threshold checkpoints and `actual.bmp`; all 21 BMPs are 320x224 and byte-identical across configurations. Independent section 02 frames 001320/001380/001410/001440 support the four sampled art/position states: base, base, alternate, base. The 30-fps capture does not establish exact VBlank alignment. See [airport landing descriptor audit](visual-audits/airport-landing-descriptor.md). |
| `OPEN-06` | The cockpit/TV skyline is drawn in front of the character; it should sit outside the cockpit and behind the character. | **Resolved for the moving skyline overlay.** ROM `0x5AC36` marks the moving `0x1DF238` list low priority (`0x6000`); Plane A (`0x8000`) and the fixed `trivid-left` (`0xC000`) / timed (`0xE000`) SATs are high priority. Qt now presents the low layer behind the plane and the fixed/timed lists above it. The importer was also corrected from palette line 1 (`0x2000`) to source line 3 (`0xE000`) for the timed sprites, and the six named assets were regenerated. Debug/Release full-screen Qt QA checks the source flags and overlap pixels. A complete 6,782-frame Release recapture was reviewed: all 277 changed cockpit frames and the byte-identical remaining 6,505 frames. Independent section 02 frames 000840/000860/000880/000900/000930/000940 support the cockpit order and timed movement. See [cockpit tri-vid priority audit](visual-audits/cockpit-trivid-layer-priority.md). |
| `OPEN-07` | The terminal Michael death-pose event requests source audio command `0x2E`, but the Qt audio service has no mapping for it. | **Closed as a non-defect.** ROM `0x59F8E` places `0x2E` in `D0` once at elapsed `0x168`; `0x530DE` routes it through `0x530FE`/`0x52F3C` as `FF 10 2E`, a start request for table-2 slot 46. The ROM-backed offline Z80 extractor records only the slot's quieting/setup writes (nine chip writes in five seconds); slot 46 has no extracted sound asset and belongs to the 29 verified silent/no-op entries. It has no audible playback duration to reproduce, so leaving `0x2E` unmapped is correct. The audio playback QA now guards that no asset resolver call occurs for it. See [the death-pose audio audit](visual-audits/opening-terminal-audio.md). |
| `OPEN-08` | During the wilderness ambush, the Indian attacker's body flashes/blasts but his head stays unchanged and then disappears abruptly. | **Resolved for the ROM-supported shared fade and retirement.** ROM `0x5A252` uses palette line 1 for the separate head SAT and right actor plate: `0x5A2D0` fills its 15 visible entries with `$EEE`, `0x5A328` steps them every 9 frames, `0x5A338` starts the timed body/smoke effect, and `0x5A350` clears only block `$03B8` (9x8 tiles). The head holds its final pose; the source has no separate head-explosion list. On stage 12 it submits the head once with line 1 at `$000`, then the next call returns without it. Qt now applies the final black palette on that last submission. Debug/Release QA checks the hit pixels, palette entries/cadence, six eight-frame body/smoke images, the stage-12 black head request, and its absence on the next frame. Independent section 02 frames 000630-000720 show the shared fade followed by smoke; this 30-fps capture supports appearance and ordering, not exact VBlank phase. The two hit-transition requests route to silent table-2 slots 47 and 70: a five-second ROM-backed extraction found only setup/quieting writes and no audible playback duration. No host aliases are needed for these requests; other sound defects remain under SOUND-03. See [the Indian attacker head-flash audit](visual-audits/indian-attacker-head-flash.md). |
| `OPEN-09` | The magician's paired spell/spark sprites keep animating after he is hit. | **Resolved in the connected native route.** ROM `0x5A252` writes four to shared `$FFF146` at the hit transition; `0x5A4C2` submits the paired six-update effect only while that state is below four. The typed pair stops on the persistent hit callback. Debug/Release QA verifies two active phases, no pair on the hit frame, and no reappearance through death cleanup. Independent section 02 frames 000660/000666/000672/000678 support the hit/blast ordering. A final 6,782-frame whole-screen Release capture was reviewed in chronological overview sheets. See [the magic-effect lifetime audit](visual-audits/ambush-magic-effect-stop.md).
| `OPEN-10` | The troll/orc death explosion is surrounded by a visible square border. | **Resolved for the reported square.** ROM `0x5A0B2` reads four map-patch records at `0x5A1D2`; the offline 0x1728 extractor now preserves empty/color-zero cells as transparent and regenerates four named assets. The four phase holds match the ROM's 12/24/60/12 updates. Connected Debug/Release startup QA passes. Native frame `frame-004446.png` no longer has the rectangle; independent section 02 frames 000640/000642/000644/000646/000648 show the same interval without a square. The 6,782-frame native route and all changed full-screen frames were reviewed. See [the intro explosion visual audit](visual-audits/ambush-explosion-border.md).
| `TAXI-01` | The taxi-window callout is missing its top/bottom edges and appears as two parallel side lines instead of a closed white outline. | **Resolved for the missing horizontal edges.** `draw_cab_mode_selection()` used hyphenated aliases while `resources.tsv` names six corner/middle tiles with dot-separated aliases, so only the left/right assets loaded. The renderer now uses all eight catalog aliases. Startup QA first reproduced the missing top-left tile, then passed pixel checks for all eight tiles across five phases and both settled mode holds. Debug/Release startup-QA images match byte for byte; the full Release capture reviewed all 182 cab-map frames and 120 field-handoff hold frames. Independent section 02 frames 001729-001735 and 001800 show closed outlines at multiple map states. The visible map-plate extent still differs and exact 30-fps-to-VBlank alignment remains unknown; both stay separate from this resolved edge defect. See the [taxi outline audit](visual-audits/taxi-window-outline.md). |
| `HUD-01` | The field portrait has unexpected color over the runner's face. | Resolved for the reported Gator face-color symptom: ROM `0x35D4` applies `+0x2C` to the 4x4 face; the old Qt path used `0x3602`. Corrected assets/runtime and per-record QA select Gator line 1 (`+0x2C=0x2000`); independent section 04/06 frames support that palette. See [the eight-question portrait audit](visual-audits/hud-portrait-palette.md). Ammo remains separately tracked as `HUD-02`. |
| `HUD-02` | Ammunition is inconsistent: the report cites 32 bullets initially, 37 on the unequipped hand, and 4 after equipping the gun. | ROM Gator record `0x72B1A` starts at 32 with item code 2; the `0x39F2` field refresh reaches `0x36FC`, which clamps it to item-2 capacity 12 at `0xD4570 + 0x24`. ROM `0x125FA`/`0x121E0` plus item code 3 capacity 11 reproduce `4/11 -> 37 -> 4`. The normal unarmed HUD branch draws `00` while preserving +0x4E as a percentage. Qt previously passed raw +0x4E to the overlay, so low retained percentages appeared as bullets; it now uses the typed `0x36FC` display result. A source-valid partial-magazine regression checks that a nonzero percentage below 64 survives unequip while the full-screen HUD shows `00` and clears the old item icon. All 12 HUD outputs were reviewed and their Debug/Release hashes match; the complete 262-image Release player-action set was reviewed in 14 sheets. The user's visible 37-on-hand route remains unreconciled with the normal ROM unarmed `00` path, so the combined report stays partial. See [ammo capacity and display audit](visual-audits/hud-ammo-capacity-refresh.md). |
| `FIELD-01` | Walking previously used the dying/fallen animation; leftward movement also showed the right-facing walk pose. | The fallen-pose family is separated from ordinary C062 movement. ROM direction 6 (`0xC062` selector `0x38`) shares direction-2 tiles and sets attribute `0x0800`; the Qt runtime applies that horizontal facing to idle and walk images. Release normal-input QA now exercises all eight headings, including six additional full-screen movement/release routes. Exact-scene visual matching and movement audio remain open. |
| `SOUND-01` | Pocket Secretary selection/cancel sounds are absent on some screens or use the wrong sound. | **Partial.** Notebook C now follows `0xF236` → `0x391A` and requests event `0x25`, then resumes the same selected row. Tips C already uses that route. Notebook row movement, A child dispatch, B/Start return, Tips page movement/return, and Tips entry are source-silent. Event `0x25` maps to `audio.effects.resource.037` (2.134 seconds including release tail); shared Debug/Release audio QA verifies decoded playback, default-output state, and duration. Debug/Release Notebook route QA checks event `0x25`, retained page/selection, and full-screen continuity. Mr. Johnsons C requests the same event; its A/B details, movement, and Start return are source-silent. Route loopback, other child-page handlers, and independent Pocket Secretary frames remain open; see [the selection/cancel audio audit](visual-audits/pocket-secretary-selection-cancel-audio.md). |
| `SOUND-02` | Moving between Pocket Secretary menu items has no navigation sound. | **Closed as a non-defect under the US ROM.** `0xC2A6` services the parent loop through VBlank/input, `0xE660`, and the `0xC376` cursor writer; it makes no sound request on Up/Down. The parent command overview requests event `0x24` once when A opens the page (2.484-second extracted clip including its release tail); that entry cue is not a navigation sound. Debug/Release route QA checks the single entry request, four accepted Down moves, and wrap to row 0 with no additional audio events. Navigation has no playback duration because no sound is requested. The root-menu screenshot series is internally reviewed, but the supplied Genesis capture has no matching Pocket Secretary visit; external visual fidelity remains unverified. See [the root navigation audio audit](visual-audits/pocket-secretary-navigation-audio.md). |
| `SEC-MAGIC` | The Magic page leaves its old text/menu content drawn over the new page. | **Closed for the spell-capable page.** ROM `0xFC22` clears Plane A before the spell panel is drawn; Qt presents the named full-page panel. Debug/Release entry, spell, force/posture, return, and reopen captures are clean, and independent section 06 frames 000001/000002/000005/000015/000020/000030 support the page and action subview without stale command controls. The separate Magical Defense fallback is a source-defined overlay on the command/status page; its Genesis appearance is not independently captured. Spell target/effect work remains under Magic. |
| `SEC-CYBERWARE` | Opening Cyberware hides inventory, the runner portrait, posture, and text that should remain visible. | **Fixed in the Qt page projection.** The child now redraws only the center text panel, preserving the command screen's portrait, inventory, posture, selected tab, and right HUD; stale center text is cleared first. Debug/Release full-screen QA checks populated/empty rows, 30-frame hold, pixel-identical parent return, and stable reopen. No independent Genesis Cyberware frame is available; see [the shared-panel audit](visual-audits/cyberware-shared-panel.md). |
| `KARMA-COLOR` | The Start-menu Attributes/Skills Karma page uses the wrong selection color. | ROM `0xD5DC` passes text mode `0x4000` for the selected row (palette line 2). The current Qt captures show `#6D4992` on all 20 selected cells and restore the prior row to green. The reported color fault is not reproduced against the traced ROM palette. No matching independent Genesis page capture is present, so visual fidelity remains unverified. |
| `KARMA-DETAIL` | The Start-menu Attributes/Skills Karma page omits the selected attribute/skill's detail text. | Reproduced before this pass: the Qt presenter skipped the existing `0xD5DC` conversion. It now renders the selected label/value, both Essence values, Magic from `+0x7F`, and the installed-cyberware `c` markers/modifiers at source-derived positions. Debug/Release QA captures all 20 cells and checks empty/populated cyberware plus clean return. The ROM provides these labels, values, and modifiers rather than longer prose descriptions. No matching independent Genesis page capture is present; do not claim visual resolution. |
| `CLIPS-REDRAW` | Clips reused the inventory action row for its selector/trade pages and left that four-action row visible behind reload Yes/No. | Fixed in Qt after tracing ROM row swaps and parent restore: Debug/Release full-screen QA covers top-level Clips, nested Trade, confirmation, decline, reload result, and return. Independent Genesis Clips frames remain unavailable. |
| `POSTURE-FOCUS` | The posture bar has no selector; it should be reachable with Up/Down from the menu buttons and affect attack/defense. | Resolved in the native command-menu route: ROM `0x11506` enters posture focus on command pages 2..6; Up draws the `0x119CE` focus row, Down restores the command row, and Left/Right update `+0x4D` plus Atk/Def bars. Debug/Release full-screen QA passes. Inventory pages 0/1 use their separate inventory cursor path. No independent Genesis capture shows the focused transition, so external visual fidelity remains unverified. |
| `INVENTORY-01` | Moving/trading items between inventory locations does not work. | **Partial.** ROM has runner-to-runner Trade, not same-runner item reordering. The typed transfer and ordinary Trade controls pass Debug/Release QA when another runner is active, but New Game starts with Joshua alone and the Qt executable has no normal recruitment/loaded-save route that supplies a second recipient. Keep open for that reachability path; see [the full-screen Trade audit](visual-analysis/qt-visual-audit.md#applied-review-trade-recipient-and-transfer). |
| `INVENTORY-02` | Using an item draws the runner portrait over other bars. | **Closed as a non-defect for the reported First Aid target/result panel.** ROM `0x123FA` redraws the selected runner's 4x4 panel at `C928` through `0x110E8`; its small portrait crosses the top of the posture strip in both Qt and independent Genesis frames 002610/002611/002630/002640. The `0x121C0` automatic-use acknowledgement does not draw that target portrait. Debug/Release full-screen routes and all transition frames were reviewed; see [item-use portrait placement audit](visual-analysis/qt-visual-audit.md#applied-review-first-aid-target-result-and-return). |
| `ITEMS-01` | Armor, weapons, and their damage values are wrong. | **Partial.** The selected-armor detail preview now follows `0x12A8C -> 0x11352` and displays the selected D4D50 row with six-tile CF884 Wpns/Guns assets in the lower panel; Debug/Release screen checks and independent frame 002560 support its placement. The capture does not show a selected/equipped armor mismatch. Weapon detail rows now have explicit D4540 +0x25/+0x26 and CF84C asset regression coverage, but no independent selected-weapon frame is available. Combat damage values and shop/item presentation remain open; see the [selected armor preview audit](visual-audits/item-armor-detail-preview.md) and [weapon detail and magazine review](visual-audits/weapon-detail-and-magazine-switch-review.md). |
| `FIELD-02` | The runner gets stuck in terrain and cannot enter buildings. | **Partial.** Typed physical routes cover Stoker, Ares, the scene-20 maglock, both scene-26 passkey doors, and the D8 Security Terminal prompt. Scene-2 wall QA verifies the ROM one-side-clear diagonal fallback; a dedicated motel-frontage QA logs held Up, wall settle, diagonal escape, and retreat while checking the ROM +0x36 animation heading. The renderer now consumes that typed heading; exact user-input reproduction and independent Genesis collision evidence remain open. Scene 20 now covers QA-seeded quality-2 passkey success, quality-1 insufficiency, Samurai no-skill Electronics, Decker no-kit partial progress, and kit-assisted Electronics success. The no-kit action reduces selection 2->1, but a fresh physical contact reloads selection 2 at ROM `0xB994`; the QA-seeded kit route then succeeds and `0x2BD2` updates map cell 483 on result acknowledgement. The kit and PRNG are test fixtures; kit acquisition is not connected. Zero-roll QA now verifies the alert page, scene timeout, silent scene-0x14 path, and live world-frame handoff; the downstream entity-target selector remains open. General terrain recovery, grenade consumption/security, building item use, and other doorways remain open. See [field map route audit](visual-audits/field-map-script-routing.md).
| `STOKER-01` | The physical Stoker collision/property that should open his generic dialogue is still unidentified. | Resolved in the normal New Game route: scene-2 record 91 at `0x173138` (`0xC2`) is reached with Up, publishes raw collision `0xFFC2` and event 2, then mode 2/event 2 selects the generic Stoker arrival panel. Debug/Release CTest, full-screen route dumps, nine-question review, and independent section 03 frames support the result. The clip confirms the door/fade/panel sequence but does not record input or event bytes. See [`visual-audits/motel-doorway-scene-entry.md`](visual-audits/motel-doorway-scene-entry.md) and the [connected Hotel route audit](visual-audits/stoker-room-to-hotel-karma-route.md). |
| `PLAY-01` | Shops, building item use, and maglock passkeys cannot be tested from normal Qt gameplay. | **Partial.** Stoker, Ares, and scene-20 lock prompts use ordinary field input. Scene 20 exercises passkey success/insufficiency, no-skill Electronics, a positive-skill partial result, and kit-assisted success from the physical trigger; the kit and PRNG for success are QA-seeded. The event `0x4B` request is verified, but route-specific speaker loopback is not. Story-driven passkey/kit acquisition, downstream alert-entity selection, other passkey qualities, grenade consumption, other building-item routes, independent Genesis maglock visuals, and the scene-2 runner/HUD overlap remain open. See the [field map route audit](visual-audits/field-map-script-routing.md), [scene-26 terminal route audit](visual-audits/scene26-event1-terminal-route.md), [Ares audit](visual-audits/ares-physical-entry.md), and [scene-20 maglock audit](visual-audits/scene20-maglock-physical-entry.md).
| `SOUND-03` | More generally, sound effects across the game are reported to play at the wrong points. | **Ongoing inventory.** The Indian ambush requests `0x2F` and `0x46` select silent table-2 slots 47 and 70, with no audible duration to reproduce; these two calls are closed as non-defects. Continue mapping each remaining reported/connected request to its state transition, trigger, playback, duration, loop, and stop in the owning goal. |
| `SEC-JOHNSONS` | The Notebook Mr. Johnsons row did not have a functional child page in the Qt conversion. | **Partial.** ROM `0xF416` now has typed FFFC27-filtered rows, source-imported names/details, A/B detail waits, C event `0x25`, cursor wrapping, and clean Start return. Debug/Release QA covers all five paths and every frame of a 30-VBlank hold. All-five coverage uses diagnostic FFFC27=`0x1F`; only bit 0 has a verified story writer, other story/save bits remain open, and no matching Genesis frame is available. See [the page audit](visual-audits/notebook-mr-johnsons-page.md). |
| `MAGIC-SPLIT` | The Magic screen differs by user type. | **Partially verified.** ROM `0xFBA6` branches to the yellow spell page for class `<2` with `+0x7F != 0`; other records get the two-bar Magical Defense overlay. Debug/Release QA uses the actual Gator Shaman (class 1, `+0x7F=6`) and unmodified Samurai (class 2, `+0x7F=0`) startup records, and the test covers the zero-resource Shaman edge plus class 2/3 fallback. Independent section 06 supports the magic-user page and nested actions, but no non-magic Defense capture was found. See [the Magic split audit](visual-audits/magic-user-screen-split.md); keep open for independent non-magic visual evidence and spell target/effect handling. |

### Newly verified findings

The current full-screen pass added two independently actionable goals:

| Goal | Reported issue | Status |
|---|---|---|
| `DIALOGUE-01` | Stoker `< MORE >` and closing punctuation render with incorrect directions. | **Resolved for the ROM HFlip glyphs.** The text catalog was already correct. ROM table words `0xC135C` (`>`) and `0xC1332` (`)`) reuse `<` and `(` with HFlip; the offline field-font extractor now applies those bits and the local assets were regenerated. The `shadowrun_graphics` CTest checks both pairs. Debug/Release startup QA writes the whole-screen Stoker arrival and manager pages; they now match section 03 frames `000090` and `000300` in glyph direction and layout. See [dialogue font flip audit](visual-audits/dialogue-font-flip-orientation.md). |
| `OPEN-11` | Start during the opening movie does not take the ROM skip path to Redmond and the automatic Stoker arrival. | **Implemented and internally checked at source gates.** `0x0BF2` latches Start; the opening schedule, stage 2, and tri-vid loops check it at their traced loop boundaries, then `0x59CFE` samples, steps the palette to black, and prepares mode entry before `0x07F0` New Game setup. The airport loop is separate: `0x59C5E` clears its marker and completes the cab route; a new Start can interrupt in `0x134D2`. Qt QA verifies the schedule skip and reaches the Lua scene-entry walk plus Stoker arrival in 34 field frames with no further input. Debug/Release integration tests also check stage-2's second-loop pass, deferred input during palette transitions, and airport marker clear. Independent section 03 supports the field and dialogue appearance but does not show a Start-pressed skip; exact external skip timing remains unverified. See [opening Start-skip audit](visual-audits/opening-start-skip.md). |

### Earlier reported items carried forward

| Goal | Reported issue | Closure condition |
|---|---|---|
| `EQUIP-01` | The yellow item selector was previously reported as a thick rectangular mess instead of the game's slim yellow outline. | The current Qt frames contain exactly 76 `#C4BE27` pixels: a one-pixel perimeter around the 24x16 item image, with no fill. Independent section 06 frames 002560/002580/002610/002640 support the same slim marker. The debug-only 4x crop scales that one pixel to four display pixels. Current geometry is not reproducing the reported thick marker; see [the selector and redraw review](visual-audits/item-selector-and-unequip-review.md). |
| `EQUIP-02` | A black box can remain around an item icon after equipping and unequipping it. | **Open; not reproduced in current internal QA.** On 2026-10-10, clean Debug/Release runs emitted 55 full-screen route images with matching hashes; all were reviewed, including repeated weapon, armor, and gear unequip transitions against fresh baselines. No black box or old child controls reproduced. The rerun corrected a stale test expectation that treated retained `+0x4E=50%` as the visible unarmed ammo count; ROM `0x36FC` draws `00` in that state. The exact transition is absent from independent Genesis footage, so the report is not claimed resolved. See [the item selector and unequip review](visual-audits/item-selector-and-unequip-review.md). |
| `LOADOUT-01` | Weapon clips/ammunition were previously reported as not being tied to the weapons. | Each weapon's `+0x24` capacity, runner `+0x4E` loaded/percentage value, and shared `+0x4F` reserve are bound to live inventory/HUD routes. Ordinary A input now runs the source spawn helper: the primary and secondary records occupy the weapon allocator's source slots 9..14, the primary moves through four live scene-terrain probes per field update, and the secondary plays its finite named directional effect. Source action class and heading select the regular/special effect family; ROM data does not support a unique projectile clip per weapon. The item-2 route reloads to 12 and fires 12->11 with reserve 1. All 261 Debug/Release full-screen captures match and were reviewed. Live actor records, target hits/status/damage, and matching independent firing frames remain open. Do not add a per-weapon reserve unsupported by the ROM; see the [weapon detail and magazine review](visual-audits/weapon-detail-and-magazine-switch-review.md) and [spawn ammo/sound audit](visual-audits/weapon-spawn-ammo-consumption.md). |

### Evidence gathered for this pass

- Independent section 01 frames were inspected in sequence. They support the
  flame lifetime, changing title circuit geometry, copyright appearance, and
  visible/hidden Push Start phases. The title-to-options transition is cut out
  of the video.
- A frame-order bug invalidated the old comparison labeled `actual.bmp`:
  prompt QA had already advanced the title to frame 530 before the checkpoint
  labeled frame 300. The startup QA now captures the title reference sequence
  first, writes `title_300.bmp` at actual frame 300, and records title frame,
  scroll offset, and circuit phase in `title-scroll-qa.tsv`. Debug and Release
  output for `actual.bmp` and `title_300.bmp` is byte-identical.
- The independent section 01 copyright first appears at video frame 418;
  the ROM title loop first draws it at title frame 254. This observed event is
  used only as an alignment anchor, not as a claim that video frame zero is a
  ROM VBlank. Candidate whole-screen pairs are 390/198, 396/210, 418/254,
  420/258, 421/260, 422/262, 423/264, 424/266, 425/268, and 450/318
  (video/title frames). Background geometry aligns at the same vertical
  offset in these pairs, within one pixel on frame 424. The compressed video
  still makes thin-line color an uncertain measure.
- The title circuit remains visibly brighter and denser green in Qt than in
  the independent frames after timing and scroll alignment. The ROM palette
  trace supports the base asset colors and the known palette writer changes
  only its separate light phase. The display difference is recorded as
  `TITLE-03` and left unresolved rather than applying an unsupported
  brightness transform. See the [full title circuit audit](visual-audits/title-circuit-palette.md).
- BOOT-02 was reproduced in the pre-fix full-screen Qt title capture. The
  source-derived flame projection is visible through title frame 165
  (`FFF1E6` reaches `0x26` after the final column), hidden on frame 166 when
  `0x1A9E` takes the `0x0E34` reset path, and remains absent at settled frame
  900. Debug/Release full-screen dumps match by SHA-256. Independent section
  01 frame 000372 shows the transient flame; frames 000384 and 000390 show
  it gone. The larger circuit brightness/texture discrepancy remains open as
  `TITLE-03`.
- The Qt pre-title audio callback routes `0x16` as FF 12 stop-resource-22 and
  preserves all 60 source-timed FF 10 `0x5C` request records while playing the
  complete stereo resource 92 once per sequence. The earlier six-full-file
  player implementation was audibly fragmented and has been superseded. Debug
  and Release QA cover request coalescing and sequence replay; independent
  longplay and fresh WASAPI loopback windows match the single cue. `BOOT-01`
  covers missing/mistimed event placement and `BOOT-06` covers fragmentation.
  Other source audio calls remain open under `global.audio`. The
  90-second ROM trace found no OPEN-01 within-stage restarts; the Release
  process-loopback capture confirms the Qt stage-2 interval is silent. The
  independent Genesis longplay does not isolate that command. Exact-route
  captures for other sounds remain open under SOUND-03.
- Independent section 02 samples show Start Options, archetype selection, and
  later opening-story scenes. They do not yet isolate the reported Decker
  head/body, propeller, landing, or taxi states.
- The current Debug posture QA's full-screen `page-2-up-focused.bmp` visibly
  contains the `POSTURE` selector and bar. That disagrees with the report from
  normal play; the menu goal remains open until the normal route is reproduced
  and compared.
- Stoker's scene-2 trigger is now identified as terrain record 91 at ROM
  `0x173138` (`0xC2`, bounds `0x3C0..0x3DF` by `0x280..0x29F`). New Game starts
  with FFDF40 clear. Ordinary Up reaches its expanded contact margin, publishes
  raw collision `0xFFC2` and event 2, and selects the Stoker arrival record
  through mode 2/event 2. A advances `< MORE >` to the manager, B selects the
  leave response, and A returns to a clean scene-2 field. The unrelated record
  2 candidate at `0x172DBE` still produces event 0 and remains rejected. The
  physical route is Debug/Release covered; independent section 03 frames
  `000028..000054`, `000055`, and `000080..000082` support the doorway, fade,
  blank panel, and populated Stoker panel, but do not log input or event state.
  The native capture goes from full black to the populated panel; exact timing
  for the two sampled blank-panel frames remains unresolved.
- FIELD-01 now uses separately named C062 movement assets and typed selector/
  clock state. Right/left cadence and blocked-reset QA pass in Debug and
  Release. A Release normal-input route now exercises all eight headings; six
  additional routes check accepted movement, facing, selector, position change,
  and C052 release, with 30 full-screen frames reviewed. Independent section 04
  frames 000900-000921 support a moving runner but show another map and contain
  no controller/selector log. Exact placement and movement sound remain open.

## Latest follow-up report: defect goals and current evidence state (2026-10-10)

The pasted report repeats several items that earlier audits marked resolved. A
repeat report reopens verification: those labels below mean the exact normal
Release route must be reproduced again before changing implementation or
claiming closure. Scene-specific assumptions, routes, timings, and population
must be checked against the US ROM first; Lua owns game policy and content,
while C++ supplies typed host services and presentation.

| Goal | Reported issue and scope | Current evidence state | Closure gate |
|---|---|---|---|
| `AUDIO-RECHECK-01` | Fragmented shock cue; missing/mistimed logo explosion cue and title theme (`BOOT-06/07`). | **Recheck requested.** Prior source and loopback audits marked sampled cues resolved; this report says the current experience still sounds wrong. | Capture a clean normal Release boot with process loopback; trace ROM request order and start/stop boundaries; compare cue identity, overlap, timing, and audible duration with the independent longplay. Update the earlier audit if the current route differs. |
| `AUDIO-RECHECK-02` | Intro music and cutscene effects are missing, mistimed, or too short (`OPEN-01`, `SOUND-03`). | **Open/recheck.** Existing stage music has source and playback evidence; full event-by-event cutscene SFX coverage is incomplete. | Inventory ROM audio requests for each intro stage, trace channels and stop calls, then capture stage boundaries and one-shot effects with loopback and full-screen frames. Record audible duration and repeat conditions per cue. |
| `TITLE-LOOP-01` | No-input title returns to a black screen instead of replaying the boot/title sequence (`TITLE-05`). | **Recheck requested.** The internal 3,601-update loop was defended, but no matching independent longplay loop exists. | Run the ordinary Release executable without input through the complete idle interval and replay; verify visible frames and audio at each transition and add a normal-window regression route. |
| `OPEN-SKIP-TIMING-01` | Start skips to Redmond and the Stoker auto-walk, but the pause and walking speed feel wrong (`OPEN-11`). | **Recheck requested.** The route is source-connected and internally covered; user timing/speed complaint remains unverified. | Compare ROM wait/movement cadence to consecutive longplay frames; capture skip, black handoff, field spawn, pause, zero-input walk, and arrival. Preserve the source-defined pause and per-update travel rate. |
| `DIALOGUE-01-RECHECK` | `< MORE >`/action brackets point incorrectly; manager yawn expression and punctuation may be wrong (`DIALOGUE-01`). | **Recheck requested.** Earlier font HFlip audit marked the glyph direction fixed; this report says the displayed result is still wrong. | Verify the actual extracted glyphs and ROM text bytes, including opening/closing glyph identity and punctuation; inspect complete Stoker and manager panels in multiple longplay frames and current Release captures. |
| `FIELD-EXIT-01` | Leaving a building leaves the runner close enough to immediately re-enter. | **Open.** Existing collision routes cover entry, not the reported exit-and-retreat sequence. | Trace each relevant ROM return coordinate, facing, input latch, and collision cooldown. Reproduce ordinary exit plus held/new directional input; verify the runner clears the trigger once and remains on the field map. |
| `SHOP-01` | Ares category buttons and item selection use the wrong focus/activation sequence; the side bar/layout is visibly malformed. | **Open.** The connected shop handles categories and transactions, but this specific nested focus behavior and full layout have not been defended. | Trace ROM category cursor, A activation, item cursor, Up/Down, cancel, and return. Verify a category requires its source activation before item focus, then capture every state and verify the side bar, list, and panel composition across the whole screen. |
| `SHOP-02` | Price/Nuyen symbols overlap digits; unaffordable feedback disappears too quickly. | **Partial.** Universal Gun Clips now right-aligns its amount to the currency glyph embedded in the ROM-derived Lua description; Debug/Release shop QA and a full-screen longplay comparison cover the overlap fix. Unaffordable feedback timing remains open. | Trace the rejected-purchase wait/cadence and any sound. Capture unaffordable and successful transactions across the full feedback interval, including clean return and resulting balance. |
| `SHOP-03` | Weapon stock omits ammunition; Spurs incorrectly display rounds (`HUD-02`, `LOADOUT-01`). | **Recheck requested.** Shared ammo state is connected, but displayed stock and live equipped counts need route-level verification. | Trace each item’s ROM type/capacity and shop detail row, then verify weapon clip values against the active runner record. Spurs and unarmed states must use the ROM’s non-ammunition presentation; inspect shop, inventory, HUD, equip, unequip, and reload frames. |
| `MAP-01` | Little Chiba incorrectly routes to an elevator; Redmond has no elevator route. Building entry/exit routes must come from ROM data. | **Partial source profile.** Scene 2 record 93/event 1 selects numeric resource scene 26; a separate scene-26 event-29 collision opens the Lua elevator selector. The scene-26 spawn is outside that trigger. The ROM profile does not name either scene Little Chiba or Redmond, and existing QA does not join the full route. | Capture a joined ordinary-input route from the reported scene-2 interaction through scene 26 and any elevator contact. Confirm the visible map identity and whether the selector opens only after the event-29 trigger. Remove any unsupported transition; encode verified routes in Lua building scripts and retain unknown names/routes as unresolved. See [the map trigger profile](field-map-trigger-profile.md). |
| `MENU-POSTURE-01` | Posture focus highlights the label but misses the selected region of the attack/defense bar (`POSTURE-FOCUS`). | **Recheck requested.** Existing internal focus QA conflicts with the normal-play report. | Trace Up/Down focus transitions and bar-value rendering; inspect selector placement, colors, animation, and attack/defense effects in the ordinary menu route and full-screen frames. |
| `MENU-CLIPS-01` | Clips child selector does not align over its four normal buttons and leaves a black block (`CLIPS-REDRAW`, `EQUIP-02`). | **Recheck requested.** Prior row-swap QA marked the stale action row fixed, but did not settle the newly described misaligned duplicate buttons/black panel in normal play. | Capture parent, Clips, reload, trade, confirmation, decline, completion, and return. Verify the selected overlay aligns with its base button and that every child fully redraws or hides all parent controls and stale pixels. |
| `MENU-KARMA-01` | Attributes/Skills values, selected color, selected descriptions, Magic, and Essence differ from the expected runner data (`MENU-02`, `KARMA-COLOR/DETAIL`). | **Recheck requested.** Typed source conversion and cell QA exist; the reported normal screen still shows inconsistent values. | Trace each displayed stat to the correct runner offsets and cyberware rules. Compare selected/unselected cells, descriptions below Karma, Essence/Magic, selector palette, and all other screen regions against multiple independent frames. |
| `MENU-REDRAW-01` | Defense bars and child content draw over controls that should be hidden. | **Open/review.** The report identifies a page-composition error; no clean full-screen route audit is recorded for this instance. | Trace the screen’s draw/clear sequence and parent-child lifetime. On entry/return, verify only ROM-shared elements survive; no old buttons, text, portraits, or bars remain underneath. |
| `MENU-CANCEL-01` | B should cancel/leave every applicable menu; Start is not a substitute cancel button. | **Open/profile required.** Current handlers need a complete button-to-transition inventory. | Trace ROM input masks and exits for root, child, confirmation, and field menus; test B and Start independently on each route, including B from the Start-menu root. |
| `MENU-BLINK-01` | Notebook/Pocket Secretary lower buttons have wrong blink frames/colors/background, including black flashes. | **Open/recheck.** Static and animated menu elements have not been verified together across a full source cadence. | Trace the ROM button frame counter, phase duration, and page conditions; capture consecutive frames for one full cycle plus idle and return. Compare every full-screen frame and verify the animation never clears unrelated UI. |
| `HUD-CURRENCY-01` | Nuyen balance is not visible where expected, including the Pocket Secretary/shop. | **Open/profile required.** | Trace ROM currency draw callers and visibility conditions; bind the live balance to Lua-defined page state and verify it updates after buy/sell/leave while keeping its source placement and formatting. |
| `COMBAT-FIRE-01` | One fire input can emit multiple shots; magazine count/reload timing, attack pose, and projectile trails are wrong. | **Open.** Connected projectile code is partial and does not yet prove the complete live combat action. | Trace controller edge/repeat handling, actor-service cadence, weapon action record, ammo decrement, pose/effect sequence, and reload threshold in the ROM. Reproduce ordinary input with full-screen frame-by-frame counts and audio; one press must match the ROM’s one action. |
| `COMBAT-STATE-01` | Spurs/unarmed states show ammunition, and unarmed attacks lack their attack animation. | **Recheck requested.** The HUD now suppresses percentage-as-ammo for one unequip route; live shop/combat transitions remain open. | Trace item type and unarmed action class across equip/sell/unequip/attack. Assert zero displayed rounds for non-ammunition, preserve any source percentage state, and verify the unarmed pose and cadence from full-screen frames. |
| `NPC-GATES-01` | The prison is accessible before its story gate; guards appear dead or move as corpses. | **Open.** No scenery corpse should be synthesized from footage; NPC death records must follow gameplay state. | Trace prison collision/event prerequisites and actor initialization/death records. Test blocked and authorized story states, normal living guard AI, and only gameplay-produced corpses; compare whole-map population and layering. |
| `NPC-EVENTS-01` | NPC population, AI packages, and random encounters need ROM-derived conditions and Lua ownership. | **Open.** Current profiles/heartbeats are partial. | Profile ROM spawn tables, service cadence, AI/action classes, encounter probabilities, quest flags, and despawn/death rules. Move behavior decisions and per-instance state to Lua; native services provide queries/render/audio. Test the conditions and cadence without invented actors. |
| `LUA-CONTENT-01` | Continue connecting dialogue, menus, shop text, building routes, and quest/event choices through Lua. | **In progress/partial.** Several shared scripts are connected, but source behavior remains in native paths and content coverage is incomplete. | For each sourced hill, identify remaining C++ policy/state, define typed Lua inputs/results, migrate one complete route, remove duplicate native policy, and add normal-input Debug/Release regression plus full-screen review. |
| `LUA-CUTSCENE-01` | Make scripted scene sequencing expandable, including actor placement/movement, animation, and sound. | **Open/design from source.** | Trace a concrete ROM cutscene into typed script commands and cadence; implement a Lua-owned sequence using narrow host services for rendering/audio/input. Validate every phase and ensure actor motion/animation are not run by a competing C++ state machine. |

Every goal above inherits the completion gate at the top of this document and the project's visual-verification checklist. In particular, screenshots must be reviewed as complete screens, child interfaces must replace or hide parent controls, and “implemented,” “tested,” “matches another internal renderer,” and “independently verified” remain separate status claims.

## Goals

### INTRO-01 — SEGA shock and logo reveal (`BOOT-01` through `BOOT-07`)

**Reported:** the shock sound is missing at the logo shock, plays at the wrong
intro point, and sounds fragmented; the Shadowrun logo reveal cue and title
theme are missing; the flame decal remains after the logo is fully revealed;
the chip background should scroll upward but does not; the title circuit must
stay hidden until the Shadowrun logo has rolled across.

**Solve:** trace the reveal/effect layers, palette transition, and sound
requests in the ROM; remove the decal at the source transition; implement the
chip scroll and circuit arrival with the source direction, cadence, and screen
lifetime; start the correct shock resource at the shock event; and connect the
alternating logo cue and delayed title theme to their source requests.

**Done when:** full intro captures show shock, decal removal, upward scroll,
logo reveal, and delayed circuit appearance at source-derived states; an
independent section 01 sequence supports the visual order; audio playback is
associated with the shock and title reveal rather than unrelated startup
phases, and resource identity, playback duration, and transition order are
recorded.

**Status:** partial. `BOOT-02`'s post-reveal flame persistence is resolved with
source-state retirement, a Debug/Release full-screen pixel check, and the
independent frame 000372-to-000384 transition. `BOOT-03` is resolved for its
reported missing upward motion: the native typed timer scrolls only the
circuit layer at the ROM cadence, Debug/Release startup QA checks its full
screens and pixels, and independent section 01 frames 000390/000396/000420/
000450 show the same upward movement. Full-plane wrap and circuit
brightness/texture remain open separately. `BOOT-04` is resolved: the native
title keeps the circuit black through
the 38-column logo reveal and uses the typed 0x1C1E palette-step counter; full
screens at title frames 188 and 189 check the state and pixels. Independent
section 01 frames 000384 and 000390 support the logo-only-to-circuit order.
`BOOT-01` is resolved for the missing/mistimed shock effect. `BOOT-06` is
resolved for the fragmented sound: the runtime now plays the full stereo cue
once per source sequence, and consecutive independent longplay windows plus
fresh Release process-loopback support that mapping. Exact source-to-device
latency remains bounded by the 30-fps recording. The title color/texture
mismatch stays separate under `TITLE-03`. `BOOT-07` is resolved for the title
audio route: all 19 ROM logo-cue requests are preserved while the complete
resource-93 render plays once, and command 0 starts resource 0 before the
copyright row. Debug/Release source tests, Qt playback CTests, the normal-route
process-loopback capture, and the independent longplay support the cue order
and asset mapping. See the [title logo and theme audit](visual-audits/title-logo-audio.md).

### INTRO-02 — Title loop, Push Start, and title-to-start transition (`TITLE-01`, `TITLE-02`, `TITLE-05`)

**Reported:** Push Start does not blink, Start skips the transition into
New Game/Load Game, and the title stalls at black after its no-input timeout.

**Solve:** identify the source blink counter/visibility interval and the
Start-edge transition/fade or wipe, then bind New Game/Load Game input only
after the transition reaches its destination state. Also trace the natural
no-input countdown through its caller-side audio/fade path and restore the
attract loop through the same startup entry, clearing stale presentation state.

**TITLE-01 result:** the Push Start blink is resolved. ROM-derived typed state
selects a visible 34-frame opening phase, then blank for 10 frames, followed by
54 visible and 10 blank frames per 64-frame cycle. Five Debug/Release full
screens check visible/blank/visible across two cycles and assert that the
copyright row survives. Independent section 01 frames 000900, 000906, and
000912 support the blink, but not its exact VBlank alignment. See the
[eight-question audit](visual-audits/title-prompt-blink.md).

**Done when:** dumps include visible/hidden Push Start phases, Start pressed
and released, every transition phase, the clean destination menu, the natural
idle timeout, the black service interval, and the first frames of the restarted
boot/title. The independent section 01 frames support the blink and Start
transition, but the supplied video has no matching idle-timeout replay.

**Status:** partial. `TITLE-01` and the reported Start-route skip (`TITLE-02`)
are source-traced and covered by Debug/Release whole-screen QA; independent
video supports the visible ordering but cannot establish exact input/VBlank
phase. `TITLE-05` now follows the ROM's idle caller path and is internally
defended by Debug/Release timing and full-screen QA, including a clean replay
of the boot/title. No matching independent idle replay is present, so that
route is not claimed as independently visually confirmed. The command-0x14
resource identity and audible playback duration remain open. See the [Start
transition audit](visual-audits/title-start-transition.md) and [idle
attract-loop audit](visual-audits/title-idle-attract-loop.md).

### TITLE-03 — Circuit background color and texture (added by full-screen audit)

**Observed:** the full-screen Qt title shows substantially more bright green
circuit detail than the independent title frames. Earlier pixel statistics
used a mislabeled Qt frame and unaligned reference frames, so those values are
withdrawn as comparison evidence. Correctly labeled whole-screen pairs now
align by the observed copyright transition and circuit geometry, and still
show a mismatch; video compression remains a limit on thin-line color.

**Solve:** trace the ROM background palette and seven light phases, inspect
the offline extracted assets, and compare full frames at matched scroll and
light states. Correct the native palette/resource mapping only when the ROM
and independent frames support the change. Do not derive a color transform
from compressed video statistics alone.

**Done when:** consecutive independent title frames and whole-screen Qt dumps
show the same circuit geometry, brightness range, and phase behavior while
the logo, prompt, and copyright remain in the correct layers.

**Status:** partial. The ROM palette and phase writers are traced, and the
frame labels and event-based reference alignment are corrected. The native
screens remain brighter and denser green than the independent capture. ROM
palette evidence supports the current extracted base colors, while video
compression prevents a confident colorimetric correction. Keep the visual
defect unresolved. See the [eight-question audit](visual-audits/title-circuit-palette.md).

### INTRO-03 — Gator Shaman, wilderness ambush, aircraft, landing, and taxi frame (`OPEN-01` through `OPEN-08`, `TAXI-01`)

**Reported:** intro music starts late; the Gator Shaman's head floats; the
Michael death image is superimposed; the cockpit skyline is in front of the
character; the rear propeller is static; the aircraft lands incorrectly; the
taxi window is drawn as two side lines instead of a complete white outline;
and the Indian attacker's head does not share the body hit flash.

**Solve:** verify Gator Shaman and Michael actor/layer ownership, put the
cockpit skyline behind the character, start music at the source scene entry,
trace the airport propeller cadence and landing position, redraw the taxi
window as a closed outline, and trace the two wilderness attacker palette rows
and their separate cleanup regions. Review the full frame for stale layers on
every handoff.

**Done when:** source-derived layer state and landing sequence are represented
as typed scene state, full intro captures cover the actors, cockpit, propeller,
and landing, matching independent section 02 frames support placement/order,
the Indian attacker's head and body follow the same source palette sequence,
and music timing is verified from the source request through playback.

**Status:** partial for the intro hill overall. `OPEN-01` is resolved for its reported late start and stage-boundary behavior: a visible ordinary Release route reached `opening_intro`, and process-specific loopback matched stage 1 at entry, silence through stage 2, and stage 3 at its entry. The source-paired `0x11` trace has no audible note sequence, and the 90-second ROM trace found no within-stage restart. The supplied Genesis longplay does not isolate stage-2 silence or exact VBlank timing; other sound routes remain open under SOUND-03. `OPEN-02` is resolved for the reported Gator Shaman head-to-body separation during the wilderness pan. `OPEN-03` is resolved for the stale actor plate beneath the death pose; its `0x2E` audio request selects a verified silent table slot and is closed as `OPEN-07` (non-defect). `OPEN-04` is resolved for the six-phase rear propeller animation, and `OPEN-05` is resolved for the sampled landing trajectory (see the [pan audit](visual-audits/gator-shaman-head-pan.md), [death-layer audit](visual-audits/michael-death-layer-clear.md), [propeller audit](visual-audits/airport-rotor-and-aircraft-tail.md), and [landing audit](visual-audits/airport-landing-descriptor.md)). `OPEN-06` is resolved for moving-skyline priority. `TAXI-01` is resolved for the missing top/bottom outline edges; the map-plate extent remains a separate open discrepancy. See the [taxi outline audit](visual-audits/taxi-window-outline.md). The tri-vid priority and full-run evidence are in [the cockpit audit](visual-audits/cockpit-trivid-layer-priority.md).
**Status:** partial for the intro hill overall. `OPEN-01` is resolved for its reported late start and stage-boundary behavior: a visible ordinary Release route reached `opening_intro`, and process-specific loopback matched stage 1 at entry, silence through stage 2, and stage 3 at its entry. The source-paired `0x11` trace has no audible note sequence, and the 90-second ROM trace found no within-stage restart. The supplied Genesis longplay does not isolate stage-2 silence or exact VBlank timing; other sound routes remain open under SOUND-03. `OPEN-02` is resolved for the reported Gator Shaman head-to-body separation during the wilderness pan. `OPEN-03` is resolved for the stale actor plate beneath the death pose; its `0x2E` audio request selects a verified silent table slot and is closed as `OPEN-07` (non-defect). `OPEN-04` is resolved for the six-phase rear propeller animation, and `OPEN-05` is resolved for the sampled landing trajectory (see the [pan audit](visual-audits/gator-shaman-head-pan.md), [death-layer audit](visual-audits/michael-death-layer-clear.md), [propeller audit](visual-audits/airport-rotor-and-aircraft-tail.md), and [landing audit](visual-audits/airport-landing-descriptor.md)). `OPEN-06` is resolved for moving-skyline priority. `TAXI-01` is resolved for the missing top/bottom outline edges; the map-plate extent remains a separate open discrepancy. See the [taxi outline audit](visual-audits/taxi-window-outline.md). The tri-vid priority and full-run evidence are in [the cockpit audit](visual-audits/cockpit-trivid-layer-priority.md).

`OPEN-08` is resolved for the Indian attacker's head flash and cleanup: the
line-1 head SAT now follows the same `$EEE` hit and nine-frame fade as the
right actor plate, and the center `$0826` clear no longer erases the right
`$03B8` layer. Debug/Release ambush QA and the complete post-fix Release
capture are documented in the [head-flash audit](visual-audits/indian-attacker-head-flash.md).
The two source audio requests at these hit transitions remain unverified in
host playback under SOUND-03.

`OPEN-09` is resolved for the magician's paired spell sprites: ROM `0x5A252`
sets the shared stop state and connected Debug/Release QA confirms the pair
does not return after impact. `OPEN-10` is resolved for the square behind the
center explosion: the four ROM patch records now retain transparent empty
cells, with source holds of 12/24/60/12 updates. Independent section 02 frames
`000640-000648` and the final 6,782-frame native route review support these
fixes. The intro hill remains partial for the unrelated open behaviors; see
the [spell-stop audit](visual-audits/ambush-magic-effect-stop.md) and
[explosion-layer audit](visual-audits/ambush-explosion-border.md).

### FIELD-01 — Field actor movement pose and animation

**Reported:** held movement appears to use a dying/fallen pose; movement left
shows the right-facing walk pose.

**Current source finding:** the previous Qt runtime selected its extracted
`0xC0B2` family on every accepted movement. ROM `0xB6DE` instead installs a
direction selector from `0xC062` after an accepted movement step; its positive
four-frame list loops. `0xB442` restores the directional idle selector from
`0xC052`. `0xC0B2` is an eight-descriptor finite behavior transition selected
by `0xB330`/`0xAFF2`, which explains the fallen final pose when it was used as
the walk cycle. For the kind-3 runner, `0xBABC` writes `0x28` to +0xCA;
`0x600E` sets +0x18 to `0x280` against the `0x1000` period, so the pose steps
at six- or seven-update intervals depending on phase. The section 04 recording
shows multiple field movement poses but has no synchronized input or selector
log.

**Implementation:** the Qt runtime consumes separately named C062 frames
for ordinary accepted movement and C052 frames for idle/reset. ROM direction
6 (left) sets attribute `0x0800`, which mirrors the direction-2 art; directions
5 through 7 use the same source transform pattern. The Qt runtime now applies
that horizontal transform to idle and walk images for directions 5 through 7.
Selector, direction, frame index, accumulator, and period remain typed native
state; C0B2 stays a distinct finite behavior transition. The Qt CTest checks
all eight selector pairs, all four extracted frames, and extended live
right/left routes. Release QA additionally drives the other six source
controller inputs through ordinary movement, verifies their target/current
headings, directions, selectors, facing transforms, position changes, and C052
release, and writes `field-movement-headings.tsv` plus 30 whole-screen BMPs.
Those new frames were reviewed in full. The earlier right/left captures remain
byte-identical between Debug and Release. The independent corridor sequence
supports changing original-game movement poses but does not show this same map
or input state.

**Remaining:** find an independent same-scene comparison for exact placement
if available; keep that limitation explicit if the corpus has no match. The
normal-input scene-2 wall route checks that blocked movement returns to C052
with frame zero in Debug and Release. Movement audio is a separate open
question.

**Done when:** ROM selector and cadence are documented; the Qt runtime consumes
the corresponding named assets directly; normal input exercises all eight
headings, cadence, blocked reset, and release; Debug/Release full-screen dumps
pass review; independent frames support the pose and exact-scene limits are
recorded. The separate building/property routes do not get folded into this
visual claim.

**Status:** partial. The source-backed C062 selector/assets/clock and facing
corrections are implemented. Right/left cadence and wall-reset QA pass in
Debug/Release; Release QA exercises all eight headings with accepted movement,
correct C062 selectors/facing, full-screen captures, and release-to-idle. The
reported fallen and left-facing poses are corrected in the native build.
Independent video supports changing movement poses but cannot identify input,
selector, or exact map placement, so the same-scene visual comparison remains
open.

### FIELD-02 — Terrain collision, building access, and physical Stoker/Ares triggers

**Reported:** movement gets stuck in terrain, buildings cannot be entered,
and property-based systems such as building item use and maglock passkey
outcomes cannot be tested through normal Qt gameplay.

**Lua map-event and location routing:** `field_events.lua:resolve_collision()` selects a named action for each accepted collision event, and `building.lua:resolve_field_location_route()` selects the scene-2 Stoker/Ares routes or reads the active map's ROM event-table row. The scene-2 C0 doorway selects numeric scene 20; event 1 selects scene 26. Scenes without an event table return `no_route`. Scene 26's physical D8 trigger opens the typed Security Terminal prompt from its descriptor spawn. Elevator floor-group destination policy and downstream activity results still use native route logic, and complete building entry/exit coverage remains open. See the [full-screen route audit](visual-audits/field-map-script-routing.md).

**Physical route results:** scene-2 record 91 (`0x173138`, value `0xC2`) at
the visible motel doorway is reached with normal Up input. It publishes raw
collision `0xFFC2`, event 2, and Stoker's mode-2/event-2 arrival prompt. The
connected Hotel route continues from this physical doorway: ordinary A
input opens the manager's room offer, pays the 20-nuyen source charge from the
New Game wallet, dismisses the paid message into Hotel status, enters the
distinct Hotel Karma upgrade page, and checks out back to scene 2. The route
does not load a motel interior map. Independent Hotel screenshots remain
unavailable; see the [nine-question route audit](visual-audits/stoker-room-to-hotel-karma-route.md). Scene-2 record 92 (`0x173142`, value `0xCB`) reaches the Ares
doorway with ordinary
Right/Up input, publishes `0xFFCB`/event 11, displays the generic Ares prompt,
then opens the shop with A and restores the field with Start. Paired scene-2
records 89/90 at `0x173124` (`0xC0`) publish event 0; event-table row 0 at
`0x172CDC` enters scene `0x14`. In scene 20, record 117 at `0x1D9724` (`0x82`) is reached with Up,
publishes projected `0xFF82`/event 2, and opens the generic maglock prompt. A
reveals the three source-backed choices and C selects Walk Away. The no-grenade
choice route verifies that the New Game party lacks item `0x0D`, uses ordinary
B to show the generic unavailable-item message, waits 30 frames, and dismisses
with A to a pixel-clean field image. A QA-seeded quality-2 passkey route uses
ordinary A, requests `0x4B`, preserves item `0x14`, changes map index 483 from
`0x82` to `0x81`, and returns to a clean field-only image. The quality-1 route
compares against selection 2 and retains the insufficient-quality choices with
key/map unchanged and no added action sound; C returns to the field.

The ordinary New Game Samurai has Electronics 0 at `+0x8A`; ROM `0x21692`
selects the no-skill string before the random trial. Its no-choice result stays
static through 30 frames, then ordinary A closes to a clean field with
inventory/map/selection and action-sound count unchanged. The ordinary New Game
Decker has Electronics 1 and no kit. QA seed `0x0006` yields one successful
no-kit roll, changes the current interaction selection 2->1, and shows the
source partial-result page without sound or map change. After A returns to the
field, holding Up at the contact edge slides along the lock face instead of
submitting a new collision. The corrected ordinary-input route clears the
contact envelope with Down, navigates the corridor with Up+Right and
Down+Left, aligns with Left, then physically recontacts record 117 with Up;
ROM `0xB994` reloads selection 2 from that collision event. The QA then seeds kit item `0x15` into the live
Decker record and uses seed `0x0004`; two of three target-3 rolls succeed,
requesting `0x4B` and selecting the success page. The result page keeps map
index 483 at `0x82`; ordinary A dismisses it, after which `0x2BD2` changes the
cell to `0x81`. The test-seeded kit does not establish gameplay acquisition.
The route writes 62 BMPs/67 artifacts, including 43 distinct full-screen
images; all Debug/Release files match by SHA-256. The seven active scene-20
route directories contain 352 BMPs per configuration and match across
Debug/Release, with 82 distinct images across the route set. The six routes
before the alert contain 302 BMPs and 81 distinct images; the alert route
contains 50 BMPs and 37 distinct frames. The downstream alert actor/effect
remains unresolved.

Scene 26's descriptor-spawn route separately uses QA-seeded passkeys at
records 117 and 110, crosses both passages, and reaches the D8 Security
Terminal prompt. These routes verify physical handler wiring, not how story
gameplay awards the items. The scene-20 route also fixed the stale B7B8 retry
bit, connected world-activity timeout, and releases the display gate across
scene re-entry. General terrain recovery and downstream Electronics alert-entity selection,
kit and passkey acquisition, other passkey qualities, grenade-consumption
results, the terminal's world-activity outcome, building item use, and other
building routes remain open.

The motel-frontage follow-up starts from the Scene-2 New Game position and
follows normal controls into records 32-38 at the west wall. Holding Up uses
the source's one-side-clear route for several steps, then settles in C052 while
the button remains held; Up+Left and South input leave the wall cleanly. The
trace distinguishes this bounded wall response from a persistent movement
lock. It also exposed that the Qt actor ignored the typed `+0x36` fallback
heading; the renderer now uses it on diagonal fallback steps. Debug/Release
QA writes full-screen checkpoints and a per-frame movement/animation/audio
trace. The inspected longplay shows the same street at rest, but does not
contain matching controller input. This narrows the report without closing
FIELD-02: the user's exact stuck sequence and independent Genesis collision
trace remain unavailable.

The Stoker route now also exercises leaving the scene-2 C2 contact after the
farewell page. The first version redispatched event 2 on Down because the
native collision callback set `+0xB0` bit 5, then Lua copied its stale route
flags over that result. The typed recovery result now returns the post-service
flags to Lua, and the script merges the retry bit while retaining its other
route state. Across the 16-update Down hold, the retry bit stays set while
the candidate terrain is `0xFFC2`, the event dispatch count stays at one, and
the bit clears on the first clear-terrain sample (`0xAC5E`, frame 16). The
runner remains on scene 2 and reaches clear terrain without reopening the
dialogue. The focused Debug/Release CTest cases pass; the full suites retain
other open failures tracked with their respective defects. All 21 full-screen
checkpoints match by SHA-256 and were reviewed. This resolves the reproduced
Stoker exit retry defect; physical second entry, other building exits, general terrain recovery,
and an independent Genesis controller trace remain open.
The distinct scene-2 record-2 candidate at `0x172DBE` remains unrelated and
still produces event 0; see the [candidate audit](visual-audits/stoker-collision-candidate.md),
[Ares audit](visual-audits/ares-physical-entry.md), and [scene-20 maglock audit](visual-audits/scene20-maglock-physical-entry.md).
General terrain recovery, other passkey quality values and failure branches,
Electronics and grenade-
consumption results, the terminal's world-activity outcome, building item use,
and other building routes remain open.

**Solve:** continue tracing movement probes, collision margins, terrain gates,
and remaining property/event routes against ROM callbacks at `0xB7B8` and
related tables. The Stoker, Ares, and scene-20 maglock entry records are now
connected to typed handlers. Keep dialog windows generic; speaker properties
and the triggered event choose their text and choices. Route remaining shops,
maglock actions, and item use from ordinary Qt controls only where ROM behavior
exposes those actions.

**Done when:** each route is reachable from gameplay with normal input; a
source-backed typed state transition, Debug/Release CTest, and full-screen
entry/result/return captures exist; adjacent terrain and HUD behavior also
survive; Stoker's dialog is reached by the physical source property without
an injected event tuple.

**Status:** partial. `STOKER-01` is resolved for the physical motel doorway;
the Stoker post-farewell exit retry path is also resolved. Scene-2 Ares entry
and scene-20 Walk Away, unavailable-grenade, passkey,
no-skill Electronics, positive-skill partial, and QA-seeded kit-success
branches are verified in Qt. The Electronics partial result reduces selection
only for the current interaction; collision re-entry resets it to 2. The success
route checks sound request `0x4B` and the `0x82 -> 0x81` map update after
acknowledgement. Scene 26 reaches both passkey doors and the D8 terminal prompt
from the descriptor spawn with QA-seeded passkeys. Full map/path coverage
remains incomplete. The scene-2 wall route now verifies the ROM one-side-clear diagonal fallback: the last orthogonal probe supplies the +0x36 direction and committed D5/D6 position, and the actor settles after input release. A fully blocked wall still takes the 0xB442 reset. This specific fallback path is implemented and internally verified, while general terrain recovery and downstream alert-entity selection,
item acquisition, other passkey qualities, grenade consumption, the terminal's
world-activity outcome, building item use, and other building/property routes
remain open.

### STOKER-01 — Physical Stoker property and collision-to-dialogue route

The negative-candidate QA is complete in commit `c411247cb5`: scene-2 terrain
record 2 at `0x172DBE` is unrelated to the motel entrance and produces event 0
without Stoker dialogue. The positive physical route is now covered by
`shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`.
New Game begins in scene 2 with FFDF40 clear at `(0x3CE0,0x2D00)`. Ordinary Up
reaches terrain record 91 at `0x173138` (`0xC2`, bounds `0x3C0..0x3DF` by
`0x280..0x29F`), publishes raw collision `0xFFC2` and event 2, and enters the
mode-2/event-2 Stoker prompt at `0xF4AA0`. Ordinary A/B input advances through
the manager and Leave pages; A returns to a clean scene-2 field. The QA uses a
fixed source-PRNG seed only to make the ordinary arrival command choose the
manager branch repeatably. It does not inject collision, mode, event, or prompt
state.

Independent section 03 frames `000028..000054` show the runner at the motel
doorway, `000055` shows the dark transition, `000080..000081` show the blank
shared panel, and `000082` shows the Stoker arrival text and portrait. These
frames support the screen sequence and panel appearance, but contain no input
or event log. The native sequence currently goes from full black to the
populated panel; the exact duration represented by the two sampled blank
frames is not established by the ROM timing trace and remains an open visual
timing check.

**Done when:** ordinary field input contacts the source-confirmed property;
typed game state publishes the ROM-confirmed speaker/event state; the generic
dialog selects Stoker content from that state; a Debug/Release CTest and
whole-screen entry/dialogue/return dumps pass; no injected event tuple is
used.

**Status:** resolved for the scene-2 motel arrival route. The ROM collision,
normal-input publication, event-selected Stoker content, full-screen draw and
clean return are defended in Debug and Release. Independent footage confirms
the visible doorway/fade/dialogue order but not the controller bytes; the
two-frame blank-panel timing and dialogue audio remain unverified.

### HUD-01 — Runner portrait palette

**Reported:** field portrait color looks wrong.

**Source trace:** At `0x3560`, the face stamp is 4x4 at F1C6 and uses live
record `+0x2C` (`0` falls back to `0x2000`). The item stamp is 3x2 at F448 and
uses `0x3602`, whose branch reads `+0x57/+0x56/+0x9B`. The 13 selectable face
records use palette lines 1, 2, or 3.

**Code changed:** typed Qt state carries the raw `+0x2C` word; offline assets
include all three named portrait palettes; Qt selects item and unarmed palettes
with `0x3602`; the static panel plan uses the ROM's 4x4 face and 3x2 item sizes.

**Internally checked:** the zero-word fallback and all three face palette
indexes have regression coverage. Debug and Release graphics, typed HUD, field
action, and Qt QA pass. The Gator QA compares the rendered face against the
exact asset selected from its live record. All 12 full-screen HUD captures and
four full-screen player-action captures were reviewed; Debug/Release hashes
match.

**Independently visually checked:** section 04 frames 000900/000906/000912 and
section 06 frames 000001/000015/000030 show the same Gator face palette. The
full 320x224 frames were reviewed. Against the opaque 32x32 crop, line 1 has
17.713 mean absolute RGB error, versus 34.191 for line 2 and 37.849 for line 3;
the remaining difference is consistent with video compression.

The full eight-question review and Debug/Release dump list are in
[`visual-audits/hud-portrait-palette.md`](visual-audits/hud-portrait-palette.md).

**Status:** resolved for the reported Gator face-color symptom. Recruited-party
portrait fidelity remains unverified because no recruited HUD appears in the
supplied recording.

### HUD-02 — Ammunition state across equip and reload

**Reported:** the user observed 32 bullets initially, 37 while the gun was
unequipped, and 4 after equipping it.

**Source trace:** ROM field refresh `0x39F2` calls `0x38FA`, which iterates the
three runner records and calls `0x36FC` with `D1=0`. For clip-using item codes
below `0x0D`, `0x36FC` caps `+0x4E` to that item's
`D4540 + (item_code - 1) * 0x30 + 0x24` capacity before drawing the counter.
The Gator archetype template at `0x72B1A` has
item code 2 and raw `+0x4E=32`; item row 2 at `0xD4570` has capacity 12. The
ROM refresh therefore displays 12, not the unrefreshed template value 32.
`0x125FA` converts an equipped magazine to a percentage on unequip, and
`0x121E0` scales that percentage to the next weapon's capacity. For item code
3, ROM capacity is 11 (`0xD45A0 + 0x24`). Four of eleven rounds
become 37% on unequip under the source ceiling rule; 37% scales back to four
rounds when that item is equipped again. A 12-round weapon also maps 37% to
four by the source half-up scale. This establishes concrete source arithmetic,
not the user's exact weapon/state. The independent recording has no matching
equip/unequip sequence. Reserve `+0x4F` is tracked separately by `LOADOUT-01`.

**Code changed:** `FieldSceneRuntime::refresh_field_hud_runner_slot()` applies
the typed zero-delta ammo-meter refresh to each live record before building
its HUD slot snapshot. `field_hud_slot_state()` also projects the visible
counter through `advance_field_hud_ammo_meter()` and records the result as
`ammo_display_value`; it no longer sends raw +0x4E to the overlay compositor.
An unarmed live record retains its percentage while the HUD draws the
source-selected `00` counter.

**Internally checked:** a typed regression asserts item code 2, 32 rounds,
and a zero-delta refresh produce 12 and the correct counter tiles. It also
asserts an unarmed record retains +0x4E=18 while its display plan and overlay
select counter zero. Qt HUD QA starts with one loaded round, applies the typed
unequip, checks that the retained percentage is nonzero and below 64, then
checks the full-screen output draws `00`, not the percentage asset, and clears
the old item icon. The inventory-action regression checks code 3 capacity 11
and the exact 4/11 -> 37 -> 4 equip/unequip round trip. The direct ROM-backed
graphics test preserves Gator's raw template value 32 and asserts item-3
capacity 11. Qt field-HUD and field-player-action CTests pass 2/2 in Debug and
Release; the workbench field-HUD CTest passes 1/1 in both. All 12 full-screen
HUD images were reviewed and Debug/Release hashes match. The full 262-image
Release player-action set was reviewed in 14 chronological contact sheets;
Debug hash parity is not claimed for that larger set.

**Independently visually checked:** section 06 frames `002550`, `002560`,
`002580`, `002610`, `002640`, and `002699`, plus section 07 frames `000030`
and `000120`, were reviewed as complete 320x224 images. They show neighboring
field HUD, inventory, Magic, and First Aid states, but none shows the reported
weapon transfer. Independent confirmation of the precise 32/37/4 sequence is
unavailable.

**Status:** partial. The stale unarmed percentage-as-bullets rendering is
fixed in the host and covered by a source-valid transition. The initial
capacity refresh and ROM arithmetic explain a plausible percentage-to-rounds
conversion, but the reported visible 37-on-hand state is not shown in the
independent capture and does not match the traced ordinary unarmed draw path.
Keep the combined sequence open until a normal-play reproduction or matching
independent capture identifies the weapon, capacity, and live values.

### MENU-01 — Pocket Secretary navigation, Magic, and Cyberware pages (`SOUND-01/02`, `SEC-MAGIC`, `SEC-CYBERWARE`)

**Reported:** moving between options lacks its navigation sound; selection
and cancel sounds are missing on some pages; Magic leaves old menu text
visible over its own content; Cyberware hides inventory, portrait, posture,
and text.

**User reference:** magic users see the yellow Magic screen with an advanced
breakdown; non-magic users see the Defenses screen with two bars. Preserve
these as separate visual states and entry branches. ROM `0xFBA6` now has a
source trace and separate real-record Debug/Release QA; independent visuals
support the Shaman page, but the supplied recording has no matching
non-magic Defense frame.

**Solve:** trace the Pocket Secretary root selection events and each child
page's source draw/clear path. Bind move/select/cancel to the correct audio
requests. Model Magic and Cyberware as typed pages that redraw their own
panels and preserve only the source-shared HUD.

**Done when:** full-screen captures cover root, every relevant child,
selection, return, and reopen; no old text/buttons survive page replacement;
all visible areas match the relevant independent section 06 frames; audio
events, playback start, and durations are recorded per action.

**Status:** partial. The reported stale-content defect is not reproduced in the
spell-capable Magic page; ROM clears that page before drawing it, and the Qt
screen replaces the command center cleanly. The Cyberware child previously
dropped shared portrait, inventory, and posture layers; that Qt redraw defect
is fixed and covered in Debug/Release with parent, child, held, return, and
reopen screenshots. Under `SOUND-01`, Notebook and Mr. Johnsons C request event `0x25`; Mr. Johnsons A/B details
and Start are source-silent. Other child-page actions and route-level
loopback are open. Root-menu navigation is closed as a
non-defect under ROM: movement requests no audio, while opening the page
requests event `0x24` once. Cyberware's independent Genesis appearance and
route-level Windows loopback audio remain unverified.

### MENU-01B — Notebook Mr. Johnsons page (`SEC-JOHNSONS`)

**Gap found:** the Notebook exposed the Mr. Johnsons row, but Qt had no
functional child page.

**Source trace:** ROM `0xF416` filters five named entries by FFFC27 bits 0..4.
A/B replace the lower panel with the selected ROM detail and wait through
`0xF312`; the next controller input rebuilds the list. C calls `0x391A`,
requests event `0x25`, and stays on the selected list. Start returns to
Notebook. Only FFFC27 bit 0 currently has a verified story writer: the Stoker
Jump House response at `0x213E2`.

**Changed:** typed list/detail selection, names and descriptions imported into
the direct text catalog, source-ordered filtering, cursor wrap, A/B wait,
C action, Start return, and a full lower-panel clear before each page redraw.

**Internally checked:** Debug/Release CTests cover the five diagnostic entries,
all five detail returns, both list wraps, C event, Start return, and all 30
VBlank frames of a held detail screen. Full-screen dumps include the five
details, rebuilt lists, and parent return. The all-entry state `0x1F` is a QA
fixture; story writers for bits 1..4 and loaded-save state remain unresolved.

**Status:** partial. The supplied Genesis longplay has no matching Pocket
Secretary visit, so the page is not independently visually verified. See
[the nine-question page audit](visual-audits/notebook-mr-johnsons-page.md).

### MENU-01C — Magic-user and non-magic Magic pages (`MAGIC-SPLIT`)

**Source trace:** at ROM `0xFBA6`, class `<2` with runner byte `+0x7F`
nonzero enters the yellow spell page; zero resource falls through to Magical
Defense. Classes 2 and 3 go directly to that fallback. The spell route clears
Plane A at `0xFC22`; fallback draws two separate defense strips over the
command/status page without clearing the whole plane. Startup templates at
`0x7291A` show Samurai class 2/`+0x7F=0`, Decker class 3/`+0x7F=0`, and Gator
Shaman class 1/`+0x7F=6`.

**Changed:** the old fallback screenshot used the Gator field composition
with its resource byte forcibly zeroed. QA now exercises the unmodified
Samurai record and asserts class, resource, selected route, both source-derived
values, both bar assets, parent HUD, and clean return. The spell route remains
separate and uses Gator's unmodified record. The conversion test also checks
that class 3 falls back even with a nonzero resource byte.

**Internally checked:** Debug/Release builds and CTests
`shadowrun_ghidra_conversion_qt_field_magic_qa`,
`shadowrun_ghidra_conversion_qt_field_magic_spell_qa`, and
`shadowrun_ghidra_magic_menu_conversion` passed. All four non-magic and all 21
spell-route Debug BMPs were reviewed full-screen; their Release SHA-256 hashes
match. The Defense screen is static across a 30-frame wait. Spell selection
cursor phases are captured at seven-frame intervals; the force/posture screen,
equipped HUD icon, parent return, and reopen are also captured. Event `0x24`
occurs once when the parent Magic page opens (resource metadata 2.484 seconds);
no additional sound call is traced for the fallback/action loop, and this
route's output-device playback was not measured.

**Independent visual check:** section 06 longplay frames 000001/000002
(Rockskin), 000005 (Mana Zap), 000015/000020 (force/posture), and 000030
(equipped HUD icon) support the spell-capable states. No matching Samurai or
Decker Defense screen occurs in the supplied longplay. The native captures
therefore verify internal source consistency, not Genesis appearance, for the
two-bar fallback.

**Status:** partial. The incorrect QA fixture is fixed and both entry branches
are source-tested. Keep the external non-magic visual claim open until a
matching game capture is available; spell target/effect routing also remains
open. See [the nine-question audit](visual-audits/magic-user-screen-split.md).

### MENU-02 — Start-menu Attributes/Skills Karma selection (`KARMA-COLOR`, `KARMA-DETAIL`)

**Reported:** selection color is wrong and specific selected attributes/skills
lack their detail text.

**Source trace:** command entry `0xD01C` sets `FFF116=-1`; A/B/Start return
through `0xD33A/0xD4A0` without mutation. `0xD5DC` draws the selected cell in
text mode `0x4000`, with the six regular attributes, Essence's `+0x7E/+0x8E`
pair, Magic at `+0x7F`, and skills. It reads cyberware flags at `+0x90`: the
first set bit in 5..8 draws the literal `c` marker and Quickness/Strength
modifier values; the first set bit in 9..11 draws the Body marker/value.
`0xD82A` adds the lower selected label and current Karma. Hotel entry at
`0xD024` is separate and enables the upgrade/cost preview.

**Changed:** the Qt presenter now calls the typed `D5DC` conversion, paints
the selected label/value in palette line 2, puts the two Essence values at
their separate source positions, and draws conditional cyberware details in
the grid's gaps. It skips the base selected cell before drawing the focus so
transparent font pixels cannot leave a green underlay. The full-page
background is repainted on entry; parent controls do not remain underneath.

**QA:** `--field-attributes-skills-dump <dir>` writes full 320x224 images for
each of the 20 cells, focus-only command state, 30-frame idle, populated and
empty cyberware, A/B/Start return, and the restored command page. Each cell is
reached with ordinary d-pad input; tests assert selected label/value color,
previous-row clearing, Essence's second value, modifier appearance/removal,
read-only runner state, and pixel-identical parent return. Debug and Release
produce 30 matching images each. The 54-test Qt suites pass in both builds.

**Done when:** all selectable rows and return transitions have full-screen
dumps, source-derived selection colors/detail text, no Hotel spending controls,
and tests proving hover does not open the page or spend Karma. Use matching
independent video evidence if present; otherwise keep visual fidelity
unverified. The supplied longplay contains no matching Attributes/Skills page.

**Status:** conversion hill defended internally. `KARMA-COLOR` is not
reproduced against the ROM-derived palette; `KARMA-DETAIL` is implemented and
covered by source-rule and pixel QA. Both stay visually unverified until a
matching independent Genesis page sequence is available.

### MENU-03 — Clips child and posture selector (`CLIPS-REDRAW`, `POSTURE-FOCUS`)

**Reported:** the Clips child pages reused the wrong button rows, and the
reload prompt left the four-action row visible behind Yes/No. The posture bar
also has no selector.
Carried-forward requirement: posture is a menu reachable with Up/Down from
the menu buttons; its selection affects attack/defense and uses the proper
selector.

**Solve:** use the ROM's distinct top-level Clips, nested Trade, and reload
confirmation rows, then restore the parent cleanly on B and Cancel. Trace
posture focus entry, Up/Down navigation, selector animation/attributes,
selected value, and attack/defense consequences.

**Done when:** parent, top-level Clips, nested Trade, reload No/Yes, decline,
success, and return frames show their source-specific controls with no stale
buttons; tests check row/focus pixels, state changes, and pixel-identical
parent restoration. Posture QA covers full-screen entry/selection/return,
selector state, and posture-dependent combat-bar changes; independent Genesis
focus frames remain necessary for an external visual-fidelity claim.

**Status:** `CLIPS-REDRAW` is fixed in the current Qt build and covered by
Debug and Release full-screen QA. No matching independent Genesis Clips
sequence has been identified, so external visual fidelity remains
unverified. `POSTURE-FOCUS` behavior is also implemented: the ROM accepts Up
from command pages 2..6, Down deselects, and horizontal input changes the
five-state posture and both combat bars. The Debug and Release posture QA
passes, including full-screen parent restoration. Inventory pages 0/1 remain
their source-defined cursor menus. No independent Genesis posture-focus
transition was found, so exact focused-state appearance remains unverified;
the separate negative-equipped-slot combat path and broader combat/AI posture
consumers remain open conversion work.

### INVENTORY-01 — Trade between active runners

**Reported:** items cannot be moved or traded between inventory locations.

**Source contract:** the `0x11740` inventory action row dispatches Equip,
Discard, Trade, Cancel, and First Aid; it has no same-runner Move/Reorder
action. Trade at `0x12836` rejects item codes `0x17..0x2B`. `0x1288E` scans
the three live runner records after the current runner and selects another
active recipient. `0x1296A` starts on No; confirming No or a full recipient
advances to the next active runner, even when Yes was selected. Confirming Yes
transfers the item and applicable phase to the recipient's first free cell via
`0x129FE`, then clears the source cell and any matching equipped state.

**Implemented and QA-covered:** the Qt uses one typed `FieldPartyRosterRuntime`
and one `InventoryTradeState`. The route uses ordinary controller inputs after
the QA fixture loads source-template runner records. Debug/Release tests cover
an equipped weapon, no recipient, full recipient with No and Yes, next runner,
wraparound, Yes transfer to the first free cell, item phase, equipment cleanup,
and a clean parent return. All ten 320x224 screenshots were reviewed; Debug and
Release hashes match. The recipient portrait is placed at source destination
`C928` -> `(160,144)`. The supplied longplay has no Trade frame, so exact
screen fidelity remains unverified.

**Current gameplay gap:** New Game activates only Joshua. The Qt executable
does not yet connect a normal recruitment or restored-save path that activates
a second runner, so the source-correct Trade action returns without mutation
in a fresh game. This is why the report remains open; the diagnostic roster
does not prove that a player can reach a recipient through normal gameplay.

**Done when:** a source-backed normal route activates or restores another
runner, then the user can Trade a valid item through the actual menu controls
and see both typed records and panels update. Same-runner reorder is not a goal
because the ROM does not implement that action.

**Status:** trade state mutation and menu behavior are implemented and
internally verified; overall `INVENTORY-01` remains partial on normal recipient
reachability and independent Trade visuals.

### INVENTORY-02 — Item-use portrait and HUD redraw

**Reported:** using an item draws the runner portrait over other bars.

**Source trace:** First Aid target selection at `0x12298` lists active runners.
When the target cursor changes, `0x123FA` calls `0x110E8` with destination
`C928`; that routine draws the selected runner's four-by-four HUD/portrait
panel. The source leaves this target panel visible when the result text replaces
the target list, then `0x11928` redraws Inventory on return. The small target
portrait crosses the top edge of the posture strip by design. Utility items in
`0x12168..0x121C0` use a different three-line acknowledgement and do not draw a
target portrait.

**Verification:** Debug/Release First Aid and automatic-use CTests pass; all 13
full-screen BMPs per build are 320x224, with matching hashes between builds.
The target/result/return sequence was compared with independent section 06
frames 002610/002611/002615 and 002630/002640/002699. Both show the same small
portrait placement and clean Inventory return. The Qt fixture uses Samurai
while the longplay uses Joshua's Gator Shaman record, so this closes placement
and redraw behavior, not same-archetype portrait color fidelity. Full review is
recorded in the First Aid and automatic-use sections of
[`qt-visual-audit.md`](visual-analysis/qt-visual-audit.md#applied-review-first-aid-target-result-and-return).

**Status:** closed as a non-defect. The reported overlay is source-correct in
First Aid; the separate automatic-use route does not draw an extra portrait.

### EQUIP-01 — Item selector thickness; EQUIP-02 — Equip/unequip cell cleanup

For `EQUIP-01`, trace the selected inventory coordinates and the source
outline attributes separately from the equipped-item cell treatment. The
current Qt action frames and CTest show a one-pixel `#C4BE27` edge around the
24x16 item art; section 06 frames 002560/002580/002610/002640 show the same
slim marker. The earlier debug-only crop is enlarged 4x and is not a
source-size view.

For `EQUIP-02`, repeat equip, unequip, and replacement, and compare the whole
inventory grid with a freshly rendered baseline. Verify the icon, equipment
frame, selector, other panels, and return state together.

The expanded Debug/Release route now covers weapon 1/2, armor 1/2, and gear
`0x15`. ROM `0x1219C` clears any active armor on the first action, even when a
different armor is selected; `0x121B0` equips the selected armor on the next
action. The typed mutation and unit test follow this two-step replacement. The
gear route checks that the icon asset variant and equipped-cell marker follow
typed equipment state. Full-screen QA verifies that the coordinate-based
selector remains on an empty cell after discard, armor/gear holds stay static
for 30 frames, expected action audio occurs while cursor movement stays silent,
and each final grid matches a fresh render. All 54 Debug/Release frames match
by SHA-256 and were reviewed as full screens for stale controls and overlap.

**Done when:** source selection and equipment state are separate typed state;
tests cover the selector perimeter and repeated equipment changes;
Debug/Release full-screen captures show no stale cell treatment after
unequip; and a matching independent transition is cited or explicitly
unavailable.

**Status:** `EQUIP-01` geometry is not reproduced as a current defect and is
supported by independent frames. `EQUIP-02`'s reported stale black box is not
reproduced in the current full-screen weapon, armor, or gear routes. The
ROM-derived armor replacement mismatch is corrected and covered; an exact
independent equip/unequip transition remains unavailable, so the black-box
report is not marked resolved.

### LOADOUT-01 - Weapon magazine and shared reserve relationship

**Source finding:** equipped weapon capacity comes from its own item row at
`D4540 + (item_code - 1) * 0x30 + 0x24`. The runner's `+0x4E` holds the loaded
round count while equipped. `0x125FA` converts that count to a percentage
using the current weapon's capacity; the unequipped runner state retains that
percentage in `+0x4E`, and `0x121E0` scales it to the selected weapon capacity
on equip. The reserve is one runner-wide Universal Gun Clips count at `+0x4F`,
consumed by the reload handler. The source does not keep a separate spare-clip
reserve for every weapon. The firing effect list is selected by action class
and heading, not by a unique clip for every individual gun.

**Current QA:** ordinary held-A input reloads item 1 to 6, then live Start,
Left, Right, A, Start input equips item 2 and reloads it to 12 from the same
runner reserve. The typed ammo percentage still transfers at 3/6 -> 6/12 ->
3/6. A separate source-template route uses C to select Ricky and A to reload
his active record. Ordinary A on Joshua's live field record calls the typed
spawn cycle, consumes the equipped item's rounds, submits the source-selected
sound, selects C042, and updates the HUD. It now retains both spawned records
in source slots 9..14; the primary makes four fixed-point movement/recovery
probes against the current map's collision data per field update, while the
secondary advances its stationary finite effect. Terrain collision stops and
retires the primary. Static default actor visuals from the profile lists for
scenes 2 and `0x14` now load as named assets and draw in the live Qt field
compositor; they are not live actor records or weapon targets, so hit/status/
damage behavior remains open. Eight extracted C042 poses replace the
walk pose for the source action delay. The 261 full-screen Debug/Release action,
menu, runner, equip, reload, and return screenshots match by SHA-256 and were
reviewed across 17 contact sheets. Independent sections 06/07 contain no
identified matching firing shot or pose. Event `0x1C` maps to resource 028 and
measures 2.483 seconds; terminal event `0x3C` maps to omitted silent resource
060, so it has no audible host clip or duration.

**Done when:** equipped item capacity, loaded rounds, and shared reserve follow
the source through equip, reload, fire, and unequip using ordinary controls;
the item's source action code selects the correct shared action-class/heading
effect and sound; typed state and HUD agree; tests cover multiple capacities
and reserve updates in Debug and Release; whole-screen action/hold/return
captures are reviewed. Keep independent Genesis comparisons explicit when no
matching shot is present. Actor hit/damage is a separate combat boundary and
must not be fabricated to close this ammunition goal.

**Status:** partial. Magazine capacity, percentage transfer, shared reserve,
ordinary-input reload/fire, source sound selection, C042 pose, primary movement
against live terrain, secondary effect animation, slot retirement, and HUD
refresh are connected to the Qt field game and internally verified. The ROM
selects spawn effects by heading/action class rather than every individual
weapon, so no unsupported unique-per-gun animation is being added. Default
scene-2/scene-`0x14` actor idle images and positions now reach the live Qt
compositor, but actor updates, collision, and target hit/status/damage remain
unimplemented. No independent Genesis firing sample was found. LOADOUT-01
remains partial pending the full route review against additional item records
and independent visual evidence.
The weapon target blocker now has a source trace: mode transition `0x2CE0`
selects the pending `0x1E552` actor list for its `0x13` result, which maps to
scene `0x2F`; the list contains five named `eye5r 1` records. Two other
pending-list pointers contain Strike Team/bodyguard/corporate-guard records.
These pending combat spawn lists are not yet extracted into runtime assets or
connected to a Qt actor, collision, and damage path. They are separate from the
new static default visuals for scenes 2 and `0x14`. The source trace and the
distinction from the separate pedestrian selector table are recorded in the
[actor population and combat handoff audit](visual-audits/field-actor-population-and-combat-handoff.md).

### PLAY-01 — Normal-control reachability for reported game systems

Provide end-to-end Qt input routes from field play to shops, item use inside a
building, and maglock passkey checks, including their source-backed entry,
result, and return transitions. Reuse the generic dialog view: speaker
properties and the triggered event determine the prompt and choices.

**Done when:** each route is reachable without injected event tuples; valid
and invalid choices update typed game state correctly; full-screen captures
show clean entry/result/return and adjacent field/HUD state; Debug/Release CTest
covers the full route.

**Status:** partial. Ordinary Up reaches the physical Stoker doorway, resolves
the manager room offer, charges 20 nuyen, enters Hotel status/options, spends
Karma on the distinct Hotel upgrade page, and checks out to scene 2. Independent
Hotel imagery and restored-save wallet binding remain open. Normal Right/Up
also reaches the Ares doorway, opens its generic prompt and shop, and returns
with Start. A separate ordinary New Game route follows paired scene-2 C0
records into scene 20, contacts record 117, opens the generic choices, and
returns with C/Walk Away. B with no party Frag Grenade shows the unavailable
result through its 30-frame wait and returns with A. QA-seeded passkey routes
cover quality-2 success and quality-1 insufficiency against selection 2.

The ordinary Samurai path reaches the zero-skill Electronics page and returns
cleanly. The ordinary Decker path reaches a partial no-kit result at selection
1 and acknowledges it. A single Up hold slides along the lock face; the
verified retry clears the contact envelope and physically approaches record
117 again, where ROM `0xB994` resets the selection from event 2. A QA-seeded
kit in the live Decker inventory and
source-seeded PRNG exercise the positive Electronics success branch through
the same physical lock; success requests `0x4B`, then A returns after map cell
483 changes on the source `0x2BD2` transition. Debug and Release each produce
352 matching scene-20 BMPs across the seven active route directories, with 82
distinct images across the set. The six routes before the alert contain 302
BMPs and 81 distinct images; the alert route has 50 captures and 37 distinct
frames. The downstream alert actor/effect remains open. The test kit does not verify item acquisition, and the
downstream security actor/effect, independent Genesis visuals, other passkey qualities,
and grenade consumption remain open. Scene 26 separately reaches the D8
terminal prompt after two QA-seeded passkey successes; the terminal's ordinary
world-activity result remains open. Restored-save wallet binding, other
building-item routes, and reverse map routes also remain open; see the [Ares
audit](visual-audits/ares-physical-entry.md) and [scene-20 maglock
audit](visual-audits/scene20-maglock-physical-entry.md).

### AUDIO-01 — Source event placement and duration across the game

**Reported:** sound effects are missing or play at the wrong times, including
the SEGA shock, intro music, and menu navigation/selection/cancel actions.

**Solve:** audit each event in its owning goal from ROM callsite to audio
resource, trigger condition, playback, duration, looping, and stop. Do not
solve this by globally playing an event at screen entry.

**Done when:** every reported sound is tied to a source event and correct
state transition; player-state and available output-device evidence confirm
playback; measured duration and stop/loop behavior are recorded. Unknown
events remain explicitly open.

**Status:** pending; overlaps the owning screen goals by design.

## Dependencies and sequence

1. `STOKER-01` is resolved for the physical scene-2 motel doorway by ordinary
   input. `FIELD-02` remains open for general terrain recovery and other
   building/property routes; event injection does not close those routes.
2. Finish and verify `FIELD-01`'s source-backed movement family before using
   its screenshots as proof of field visual fidelity.
3. `INTRO-01/02/03` cover logo, title, timed opening, aircraft, and taxi
   defects; compare consecutive whole-screen frames before changing each
   layer or timeline.
4. `HUD-01`, `HUD-02`, `ITEMS-01`, and `LOADOUT-01` provide authoritative
   runner/item state to inventory, combat bars, and shop routes.
5. `MENU-01/02/03`, `EQUIP-01/02`, and `INVENTORY-01/02` are separate screen
   routes. Each must replace old controls cleanly and be validated in normal
   play as well as by its focused regression route.
6. `PLAY-01` depends on field collision/property discovery. Ares and scene-20
   passkey success and one insufficient-quality result now have normal-control
   routes with explicit QA-seeded inventory; the Samurai no-skill Electronics
   branch also follows ordinary input from New Game. Connect story-driven
   passkey acquisition, positive-skill action results,
   the Security Terminal outcome, and building-item routes without event
   injection.
7. `AUDIO-01` and `SOUND-01/02/03` track source events across screens. Close
   each sound in the goal that owns its trigger; do not mark the audio group
   complete while any reported event remains unresolved.
