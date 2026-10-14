# Airport rear propeller animation audit

Scope: reported defect `OPEN-04`, “the aircraft's rear propeller does not
move.” The earlier audit called the reported part ambiguous. That finding was
wrong and is withdrawn. The ROM has a six-state animation for this propeller;
the static circular hub in the vehicle body is a separate graphic.

## ROM-first behavior trace

- The opening coordinator at `0x59C5E` runs the airport stage through `0x5AD1A`.
  The airport scroll service at `0x5B00A` moves the scene offset by two pixels
  per source frame.
- ROM routine `0x5B100` exits after signed `DD08 < -0x1B8`. While the vehicle
  is in range, it selects the next sprite descriptor when its deadline is
  reached, sets the next deadline six source frames later, and draws at
  `DD08 + 0x1FC`, `Y=0xEF`, with palette flags `0x2000`.
- The six frame groups come from the table at `0x5B180`; their ROM starts are
  `0x1DF2D8`, `0x1DF30A`, `0x1DF344`, `0x1DF36E`, `0x1DF3A0`, and `0x1DF3CA`.
  The named `intro.sprites.helicopter.*` assets make the animated rear
  propeller visible as a thin strip over the moving vehicle. The phase advances
  every six source frames and returns after 36 frames. It needs no player input.
- `intro.aircraft.plane.a` contains the vehicle body and a separate static
  circular hub. The hub is not the animation described in `OPEN-04`. Landing
  at `0x5B1B6` is a different seven-phase motion path.
- No sound call occurs in `0x5B100`; no rotor sound or duration is attributed.

## Nine-question visual review

1. **Does it look correct? — Yes for the reported motion.** Independent section
   02 full frames `001065`, `001068`, `001071`, `001074`, `001077`, and `001080`
   show the vehicle moving and the rear propeller strip changing. The recording
   is compressed and sampled at 30 fps, so it cannot establish exact VBlank
   phase or pixel color.
2. **Does the visible look match behavioral state? — Yes.** The typed pulse
   phase selects six distinct ROM-backed groups at the six-frame deadline. The
   whole-screen QA trace records phase, descriptor, stage frame, scroll, and
   sprite position.
3. **Are the visuals themselves correct? — Supported.** The host composes the
   named vehicle and propeller assets at the traced position. The six images
   visibly differ; the independent frames support motion. Pixel-exact color is
   limited by source video compression.
4. **Should it animate? — Yes.** It advances every six VBlanks, cycles every
   36, follows the vehicle while `DD08 >= -0x1B8`, and stops outside that range.
   The independent clip shows motion; ROM and typed-state QA establish cadence.
5. **Is there sound? — No call traced.** `0x5B100` contains no sound request.
   The opening soundtrack and other timed cues are separate.
6. **Are the other screen parts correct? — Reviewed in full frames.** The
   story paragraph remains above the aircraft. These checkpoints show no old
   tri-vid layer, title menu, or other stale control. Landing and taxi details
   remain separate goals.
7. **Do multiple frames support this interpretation? — Yes, with limits.** Six
   adjacent independent frames and seven native checkpoints show phase changes
   and a repeat. The 30-fps video does not log controller input or VBlank.
8. **Does new UI replace old UI cleanly? — Yes.** This is a scene handoff, not
   a nested menu. ROM `0x5AD1A` clears the previous presentation before the
   airport resources are drawn.
9. **Was the complete time window reviewed? — Yes for this verification pass.**
   The native capture covers pre-title, title, startup menus, all Gator Shaman
   opening scenes, the field handoff, and a two-second field hold. The dedicated
   rotor QA captures all six animation states and the 36-frame return. Every
   saved frame is a complete 320x224 screen.

## Verification and defect disposition

### Full-run re-review (2026-10-07)

I rechecked the primary US ROM bytes rather than relying on the earlier
description. At `0x5B100`, `MOVE.W ($FFDD08),D0` and `CMPI.W #$FE48,D0`
gate the sprite path at signed `DD08 >= -0x1B8`; the routine then reads a
phase-indexed longword from the six-entry table at `0x5B180`, sets its next
deadline to `FFDF88 + 6`, and calls `0x1378` to build the sprite records.
The table's six pointers are `0x1DF2D8`, `0x1DF30A`, `0x1DF344`,
`0x1DF36E`, `0x1DF3A0`, and `0x1DF3CA`. Ghidra's `fn_0005B100.cpp` agrees;
the Qt runtime consumes the six named extracted groups through typed pulse
state rather than interpreting those ROM addresses.

The latest Release run is the complete 4,982-frame capture under
`archive/qa-artifacts/visual-review/taxi-full-intro-release/`, from pre-title through the
field hold. I reviewed 831 full-screen samples at 10 fps across 13 contact
sheets covering the entire run. I also reviewed every one of the 219
consecutive full-screen frames in the active propeller window (clip frames
3359-3577; opening frames 1727-1945) across four contact sheets. The seven
dedicated rotor checkpoints were reviewed separately at native 320x224 size.
They show the aircraft moving left, the rear strip changing through each of
the six descriptor groups, and the sequence returning to its first group.
No old title, cockpit, or menu graphics persist into the airport scene; the
run continues through cab-map selection and reaches the field hold.

The independent section 02 frames `001065`, `001068`, `001071`, `001074`,
`001077`, and `001080` were also reviewed as complete 320x224 images. The
aircraft changes position and the rear strip's appearance changes across
them. This confirms visible animation, while the 30-fps recording still
cannot establish its exact VBlank phase. The current full-run image is a
runtime review, not an independent Genesis reference.

- **Code changed:** no rotor renderer change was needed in this correction;
  the existing native typed pulse path already selects and draws the six
  source-backed groups. This audit corrects the earlier misidentification.
- **Behavior implemented:** six descriptor groups, six-frame cadence,
  36-frame cycle, scroll tracking, and the `-0x1B8` active-range boundary.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_opening_rotor_qa`
  writes `actual.bmp`, `rotor-phase-0.bmp` through `rotor-phase-6.bmp`, and
  `opening-rotor-qa.tsv`. It checks full-screen dimensions, changes in the
  propeller strip (`y=100..111`), stable vehicle-body pixels (`y=112..175`)
  after the 12-pixel scroll, and the 36-frame repeat. The leaf test checks the
  active boundary. Debug and Release CTests pass; their full-screen artifacts
  match byte for byte and were reviewed.
- **Independent visual check:** whole independent section 02 frames listed
  above and native opening frames `1827`, `1833`, `1839`, `1845`, `1851`,
  `1857`, and `1863` show the propeller's appearance changing. All 219 frames
  in the active animation window of the current full intro run were also
  reviewed, in addition to the 831 full-run samples. The longplay is an
  appearance reference, not a VBlank oracle.
- **Reported defect resolved:** **Yes.** The reported propeller is the animated
  six-group sprite sequence, not the static circular hub. The earlier
  “ambiguous” disposition was incorrect.
