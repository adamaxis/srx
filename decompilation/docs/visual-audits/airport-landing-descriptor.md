# Airport landing sprite-selection audit

This audit covers `OPEN-05`, the reported aircraft landing on a building
incorrectly. It records the sampled landing projection and sprite selection;
it does not close the separately audited six-state rear-propeller animation (`OPEN-04`) or the
remaining opening sequence goals.

## ROM behavior

The airport stage enters through `0x5AD1A`, clears the prior presentation,
installs airport resources, resets the landing accumulators at `0x5B198`, and
services the landing pass on entry and once per VBlank. The animation helper at
`0x5B1B6` reads seven `(dx, dy, delay)` records at `0x5B270`. Their values are:

| Phase | dx | dy | delay |
|---:|---:|---:|---:|
| 0 | `0x00F1` | `0x0071` | `0x0024` |
| 1 | `0x006E` | `0x0000` | `0x005A` |
| 2 | `0x0071` | `0x0071` | `0x0048` |
| 3 | `0x002A` | `0x0071` | `0x0012` |
| 4 | `0x0000` | `0x0038` | `0x0024` |
| 5 | `0x0063` | `0x0038` | `0x0012` |
| 6 | `0x0000` | `0x0000` | `0x0000` |

After the `0x226`-VBlank lead-in, the phase events occur at elapsed frames
550, 586, 676, 748, 766, 802, and 820. The routine uses the base SAT list at
`0x1DF3F4` throughout the path. It switches to the alternate list at
`0x1DF3FE` only when computed screen Y is at least `0x110` and less than
`0x11F` (inclusive range `0x110..0x11E`). The phase number is not an artwork
selector. The source path has no sound request at `0x5B1B6`.

## What changed

`opening_intro_source_plan()` previously assigned alternate movement art to
landing phases 4, 5, and 6. This was based on phase index and contradicted the
ROM's Y-window selection. All seven phase records now use the base movement
descriptor; the existing typed `advance_title_intro_landing()` chooses the
alternate descriptor only from computed Y.

The Qt CLI option `--opening-landing-qa <dir>` records 20 whole-screen
checkpoints at the lead-in, each phase boundary, and around the alternate-art
Y window. It writes `actual.bmp`, all checkpoint BMPs, and a TSV containing
elapsed/source frames, phase/deadline, accumulators, computed Y, descriptor
variant, and request coordinates. The checks require no landing sprite before
the lead-in expires; base art at elapsed 550, 612, and 732; alternate art at
792 while Y is `0x11C`; and base art again at 852 while Y is `0x122`.

## Eight-question visual review

1. **Does it look correct? — Yes for the sampled landing sequence.** I
   reviewed every one of the 20 Qt `320x224` checkpoints across the entire
   screen. Independent section 02 frames 001126/001127 show the dark approach;
   001288/001289 show the airport skyline handoff; 001320, 001380, and 001410
   show the aircraft at successive locations; 001415-001435 show the settled
   skyline; and 001440 shows the landing text and skyline. The supplied clip
   runs at 30 fps, so these stills do not establish exact 60-Hz VBlank phase.
2. **Does the look match the behavioral state? — Yes.** The TSV's typed phase,
   accumulator, computed Y, and draw request agree. Selected checkpoints are:

   | Elapsed | Request x/y | Computed screen Y | Art | Qt visible top-left |
   |---:|---:|---:|---|---:|
   | 612 | `0x00A9/0x00EE` | `0x00EE` | base | `(28,102)` |
   | 732 | `0x00DD/0x0108` | `0x0108` | base | `(80,128)` |
   | 792 | `0x00E7/0x011C` | `0x011C` | alternate | `(90,145)` |
   | 852 | `0x00EE/0x0122` | `0x0122` | base | `(97,154)` |

3. **Are the visuals themselves correct? — Yes at the matched samples.** The
   extracted base and alternate crops are `24x8` and `23x13`; their catalog
   origins are `(115,120)` and `(115,117)`. Source-image template checks find
   base art at `(29,102)` in frame 001320 and `(81,128)` in 001380, alternate
   art at `(90,145)` in 001410, and base art at `(97,154)` in 001440. Matching
   Qt crops occur at `(28,102)`, `(80,128)`, `(90,145)`, and `(97,154)` in the
   corresponding elapsed checkpoints. The one-pixel horizontal differences
   in the first two pairs are consistent with the compressed 30-fps reference;
   no color or scale correction was inferred from it.
4. **Should it animate? — Yes, during approach and landing.** The ROM lead-in
   is `0x226` VBlanks, followed by the seven table events above. Position
   accumulators advance once per airport VBlank. Phase 6 has zero deltas and
   holds the landed projection; sprite draw requests continue. The alternate
   art turns on and off only at the computed-Y threshold. The QA includes
   pre-threshold, event, in-window, and post-window frames.
5. **Does it have sound? — No sound call traced for this sprite helper.**
   `0x5B1B6` issues the sprite build and does not request audio. The broader
   opening soundtrack trigger and lifetime remain open under `OPEN-01` and
   `AUDIO-01`; no clip or duration is assigned to the landing animation.
6. **Are the other visible parts correct? — Yes in the reviewed samples.**
   Full frames include the Seattle skyline, rotor/plane layers, story text, and
   the approach-to-airport transition. The scene handoff clears the prior
   presentation; the captures show no cockpit, menu, or stale text layer over
   the airport. The final six-line landing paragraph appears in the same
   skyline composition as source frame 001440.
7. **Do multiple frames support the interpretation? — Yes.** Four independent
   source frames show the changing base/alternate/base art and position; the
   additional adjacent frames 001415-001435 show the completed landing and
   approach to the paragraph. The source is a continuous gameplay recording,
   not an input/VBlank log, so it supports appearance and order rather than
   exact cadence.
8. **Does new UI replace old UI cleanly? — Yes for this scene handoff.** This
   scene has no menu controls. The traced airport entry clears the previous
   scene before installing the new background and sprite layers. The lead-in,
   skyline, and caption checkpoints were inspected as whole screens; no old
   buttons or graphics remain over the airport page.

## Verification and outcome

- **Code changed:** seven landing plan rows now use the base sprite descriptor;
  alternate selection remains a computed-Y rule in typed state.
- **Behavior implemented:** the source delays, accumulator movement, special
  Y window, and settled phase are preserved.
- **Internally checked:** Debug and Release pass
  `shadowrun_ghidra_conversion_qt_title_intro_late_qa` and
  `shadowrun_ghidra_conversion_qt_opening_landing_qa`. Each configuration
  writes 20 checkpoint images plus `actual.bmp`; all 21 BMPs are `320x224`,
  have matching hashes between configurations, and passed full-screen review.
- **Independently visually checked:** section 02 frames 001126, 001127,
  001288, 001289, 001320, 001380, 001410, 001415, 001420, 001425, 001430,
  001435, and 001440. Selected frames match the extracted sprite shapes and
  positions as recorded above.
- **Reported defect resolved:** yes for the sampled airport landing artwork
  and trajectory. Exact VBlank alignment remains unmeasurable from the
  supplied 30-fps capture; other intro and rotor defects remain separate.
