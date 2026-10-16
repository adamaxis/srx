# Indian attacker head flash and layer-clear audit

Date: 2026-10-08
Defect: `OPEN-08`, the Indian attacker head does not flash with the body and then
disappears.
Status: source-faithful shared fade and cleanup; the final stage-12 head
palette submission is now included and captured

## ROM and decoded-source trace

The US ROM's two wilderness attackers use independent state sequences and
different map regions. At `0x5A0B2`, the center sequence sets palette line 0 to
`$EEE` at `0x5A124`, steps it every nine source frames at `0x5A16E`, and clears
block `$0826` as a 12-by-11 tile region at `0x5A1BE`. The Indian attacker is
the persistent sequence at `0x5A252`: it requests audio event `0x2F`, sets
palette line 1 to `$EEE` at `0x5A2D0`, steps that line every nine frames at
`0x5A328`, starts the timed effect at `0x5A338`, and clears `$03B8` as a 9-by-8
tile region at `0x5A350`. The persistent head SAT is drawn separately with
palette-line-1 flags. The persistent sequence requests audio event `0x2F` at
`0x5A2BA`; the center sequence requests event `0x46` at `0x5A130`.

Ghidra's `fn_0005A0B2.cpp` and `fn_0005A252.cpp` corroborate the sequence
branches and helper calls; `disassembly.s` confirms the palette words,
nine-frame deadlines, and distinct fill destinations/dimensions. The Qt
conversion already holds both sequences and their color words as typed state.
The implementation uses those named host image assets directly: while a
sequence is in its hit/fade stages, the right map image and its head SAT are
recolored to the current expanded CRAM word. The center image follows its own
line-0 state. The center sequence's `$0826` clear calls a center-layer
callback; only the right sequence's `$03B8` event clears the Indian attacker's
map layer. On the stage-12 call, the ROM sets line 1 to black, clears the map,
and submits the current head SAT once more; the next call returns without a
head submission. Qt previously stopped recoloring at stage 11, so that final
SAT submission could use the original host colors. The palette gate now
includes stage 12. This sequence has no separate head-explosion sprite list:
the head holds its last pose while the shared palette fades and the timed
blast/smoke descriptors play over the actor. No ROM table or VDP renderer was
added to the runtime.

The earlier bug had two connected effects. The Qt SAT head was drawn from its
original image without applying the source line-1 flash, and the center clear
was wired to clear the right map image. The stage-12 edge case also omitted
the final black palette on the last SAT submission. The corrected path keeps
the head and map art synchronized through the shared fade, clears only the
right map layer at `$03B8`, submits the black head once over the smoke, then
omits that SAT on the next frame as the ROM does.

## Connected game route and captures

`shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa` runs the converted
startup/New Game Gator Shaman route into the actual Qt renderer. It saves full
320x224 frames and checks the typed sequence stage, all 15 line-1 color words,
the head SAT pixels, the right map pixels, both independent clears, and the
timed descriptors. Debug and Release CTest both pass.

The Release `--full-intro-capture 2 2700` route wrote 6,782 consecutive
320x224 PNGs from pre-title through the 120-frame stable field hold, with
`frames.tsv` and `boundaries.tsv`. I reviewed all 18 full-screen contact
sheets, which sample every sixth capture frame, and inspected full-size frames
around the hit, palette step, timed blast, smoke, and cleanup. The process
reached `field_ready`, completed the stable hold, and exited. The capture has
no audio track. The main artifacts are under
`decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/`:

- `opening-ambush-flash-qa/actual.bmp` is the complete hit-flash frame.
- `opening-ambush-flash-qa/actor-before-flash.bmp`,
  `actor-hit-flash.bmp`, `actor-flash-step-1.bmp`,
  `ambush-effect-0.bmp` through `ambush-effect-5.bmp`,
  `actor-final-head-fade.bmp`, and `actor-after-clear.bmp` are full-screen
  checkpoints. The stage-12 capture verifies a head request with all 15
  palette words at zero; the following frame verifies that the request ended.
- `full-intro-ambush-fix-review/frame-*.png` is the complete route;
  `contact-00.png` through `contact-17.png` and the phase logs record the
  reviewed sequence.

The independent Genesis recording's section 02 frames `000630`, `000645`,
`000660`, `000675`, `000690`, `000705`, and `000720` show the separate normal,
white-hit, blast, smoke, and cleared-actor states. The 30-fps recording
supports appearance and ordering; it does not establish exact VBlank phase.

## Nine-question visual review

1. **Does it look correct? Yes for the source sequence.** The head receives
   the same white hit and gray fade as the actor plate. It has no separate
   head-explosion animation: the timed blast/smoke art overlays the actor,
   then the last black head submission is visually hidden by smoke.
   Independent section 02 frames 000630-000720 show the same ordering. Exact
   frame alignment is not claimed.
2. **Does the visible look match the behavioral state? Yes.** On entry to
   persistent stage 5, typed line 1 contains fifteen `$EEE` words and the head
   SAT has palette-line-1 flags. The same word drives the right actor image.
   The first timed step is `$CCC` at opening frame 1039, nine frames after the
   white hit at frame 1030. At stage 12, the head SAT is still submitted with
   palette word `$000`; the next source call returns without submitting it.
3. **Are the visuals correct? Yes for this palette and layer behavior.** The
   palette conversion expands Genesis 3-bit RGB exactly (`$EEE` to white and
   `$CCC` to RGB 219), while preserving transparent pixels. Full-screen
   checkpoints retain the foreground runner, text, projectiles, and the
   separate effect layers.
4. **Should it animate? Yes.** The source hit begins at persistent stage 5;
   the palette steps every nine frames while that sequence is active. The
   timed body/blast/smoke descriptors advance every eight frames from opening
   frame 1066 through 1106. The right map clears at the stage-12 `$03B8`
   event; the black head submission lasts through that frame, then stops on
   the next call. These are automatic intro transitions, not input-triggered
   animations.
5. **Does it have sound? No audible clip is present in the ROM resources.**
   The hit paths request events `0x2F` and `0x46`; the audio command path sends
   them to table-2 slots 47 and 70. The ROM-backed extractor produced nine
   chip writes for each slot in a five-second trace: PSG attenuation and YM
   setup/mode writes, with no tone sequence or YM key-on. Neither slot has an
   extracted host asset, consistent with the catalog's silent/no-op entries.
   There is no audible playback duration to reproduce, so no host alias is
   needed for these two requests. Other game audio remains tracked under
   `SOUND-03`.
6. **Are the other visible parts of the screen also correct? Yes for the
   reviewed window.** The full-screen route shows the center block clearing
   independently while the Indian attacker stays visible, then shows the
   stage-12 smoke and the following frame without a head submission. The
   foreground runner, dialogue text, projectiles, and subsequent
   cockpit/airport/cab/field pages remain present without a stale wilderness
   map layer.
7. **Do multiple frames support the interpretation? Yes.** The focused Qt QA
   captures pre-hit, hit, first fade, six timed effect states, and post-clear;
   the full Release route contains every frame. Independent section 02
   frames 000630-000720 provide multiple examples. The intro is automated, so
   there is no player click-through path for this effect.
8. **Does the new UI replace the old UI cleanly? Yes; no menu is involved.**
   The source clears the center and right map regions separately. The host
   callbacks reset only the matching map region. The head SAT stops on the
   call after the stage-12 black submission; the following cockpit scene
   replaces the wilderness layers cleanly.
9. **Was the whole time window reviewed? Yes.** The Release capture starts
   before the title, includes the complete intro and handoff, and ends after
   120 stable field frames. All 18 contact sheets were reviewed in order, and
   full-size affected frames were inspected. The process completed without a
   stall or crash.

## Verification result

- **Code changed:** typed center/right clear callbacks, CRAM-word image
  recoloring, Qt presentation of the right head/body palette state, and the
  final stage-12 black head submission.
- **Behavior implemented:** the Indian head and body share the source fade,
  timed blast/smoke layers retain their eight-frame cadence, the right map
  clears at its source event, and the black head submission ends next call.
- **Internally checked:** Debug/Release
  `shadowrun_ghidra_conversion_qt_opening_ambush_flash_qa` and
  `shadowrun_ghidra_genesis_image_compositor`; adjacent wilderness-pan,
  death-layer, and tri-vid-layer QA also pass in both configurations.
- **Independently visually checked:** Genesis section 02 frames 000630-000720
  and the complete post-fix native Release route. The Genesis recording does
  not establish exact VBlank alignment or audio playback.
- **Audio source checked:** requests `0x2F` and `0x46` select silent table-2
  slots 47 and 70. A five-second ROM-backed extraction found setup/quieting
  writes only and no audible clip or duration; leaving them unmapped in the
  host service is correct.
- **Reported defect resolved:** `OPEN-08` is resolved for the ROM-supported
  shared head/body fade and retirement. A separate head-explosion animation is
  not present in this ROM sequence. The two hit-transition audio requests are
  verified silent; unrelated sound defects remain under `SOUND-03`.
