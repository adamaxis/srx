# Airport rotor and aircraft tail review

Scope: reported defect `OPEN-04`, “the aircraft's rear propeller does not
move.” Status remains **partial** because that description may refer to the
static tail detail or to the separate animated rotor strip.

## ROM-first behavior trace

- The opening coordinator at `0x59C5E` runs the airport stage through
  `0x5AD1A`. That entry installs the airport resources and preserves the
  previous stage's typed `DD08` scroll offset. The airport scroll service at
  `0x5B00A` moves that offset by two pixels per source frame.
- The `0x5B100` pulse routine returns when signed `DD08 < -0x1B8`. While the
  aircraft is in range, it chooses the next descriptor when the current frame
  reaches its deadline, sets the next deadline six source frames later, and
  draws at `DD08 + 0x1FC`, `Y=0xEF`, with palette flags `0x2000`.
- The six descriptor groups come from the table at `0x5B180` and begin at ROM
  addresses `0x1DF2D8`, `0x1DF30A`, `0x1DF344`, `0x1DF36E`, `0x1DF3A0`, and
  `0x1DF3CA`. The native plan maps these to six named
  `intro.sprites.helicopter.*` resource groups. The groups render the thin
  horizontal rotor strip above the vehicle. Its six states repeat every 36
  source frames while the aircraft travels left by two pixels per frame.
- `intro.aircraft.plane.a` supplies the vehicle body and its rear circular
  detail. That image has no per-frame animation list in this scene. The later
  landing movement is a separate seven-phase path at `0x5B1B6`; it must not be
  mistaken for the rotor sequence. The source loop exits the rotor's active
  range below `-0x1B8`.
- `0x5B100` contains no sound-effect request. Opening music and other timed
  intro sounds remain under `OPEN-01`/`SOUND-03`; no sound or duration is
  attributed to this rotor without a source request.

## Eight-question review

1. **Does it look correct? — Partial.** Independent section 02 frames
   `001065`, `001068`, `001071`, `001074`, `001077`, and `001080` show the
   airport vehicle moving left and the thin upper rotor strip changing. The
   screenshots support the broad shape and motion, not pixel-exact color or
   VBlank alignment; the reference video is compressed and runs at 30 fps.
2. **Does the visible look match behavioral state? — Yes for the rotor strip.**
   The typed pulse phase and descriptor advance together through six distinct
   ROM-backed groups at the six-frame deadline. The full-screen checkpoints
   record the phase, descriptor, stage frame, scroll, and pulse position.
   The body and rear circle remain static within the moving plane asset.
3. **Are the visuals correct? — Partial.** The host uses the individually
   named plane and rotor assets at the traced position, and the six rotor
   images visibly differ. Independent frames support the upper-strip motion.
   The exact rear circular detail remains ambiguous in the report; in the
   available reference frames it appears static like the source plane asset.
4. **Should it animate? — Yes for the upper rotor strip; no source animation
   is recorded for the plane plate's tail detail.** The upper strip advances
   every six VBlanks, cycles through six groups in 36 VBlanks, and follows the
   plane until `DD08 < -0x1B8`. It does not depend on player input. The tail
   detail is part of the static plane image. The subsequent landing movement
   has its own seven phase records and starts after the `0x226` lead-in.
5. **Does it have sound? — No call traced in `0x5B100`.** A rotor sound and
   duration are not assigned. The opening soundtrack and other events are
   tracked separately and are not evidence of a rotor effect.
6. **Are the other visible parts correct? — Yes for this sampled screen's
   composition.** All seven native images are complete `320x224` frames. The
   story paragraph remains above the moving plane; no cockpit layer, menu
   button, or old screen control is drawn over the airport scene. The other
   airport landing and caption details remain in the broader `INTRO-03`
   review.
7. **Do multiple frames support the interpretation? — Yes, with limits.**
   Six adjacent independent source checkpoints and seven native full-screen
   checkpoints show the rotor states and a 36-frame repeat. The source video
   has no input or VBlank log, so it does not independently prove the exact
   six-VBlank cadence; the ROM trace does.
8. **Does new UI replace old UI cleanly? — Yes.** This is a scene handoff,
   not a nested menu. `0x5AD1A` clears the prior presentation before the
   airport resources are drawn. The sampled frames contain no old tri-vid or
   title-menu graphics.

## Verification and defect disposition

- **Code changed:** added a Qt airport rotor QA route and expanded the pure
  `TitleIntroPulseState` regression. The existing typed runtime behavior was
  not altered.
- **Behavior covered:** all six descriptor groups, one update per six source
  frames, the 36-frame wrap, the active-range boundary, scroll tracking, and
  sprite placement.
- **Internally checked:** `shadowrun_ghidra_conversion_qt_opening_rotor_qa`
  writes `actual.bmp`, `rotor-phase-0.bmp` through `rotor-phase-6.bmp`, and
  `opening-rotor-qa.tsv`. `actual.bmp` is the complete first-phase screen. The
  QA checks the complete image size, requires the
  rotor strip (`y=100..111`) to change between phases, requires the plane body
  (`y=112..175`) to remain pixel-identical after the 12-pixel translation,
  and requires both to repeat after 36 frames. The six-phase leaf test also
  checks the exact `-0x1B8` active boundary and the stopped state below it.
- **Debug/Release:** both CTests pass in both configurations. All seven BMPs
  and the trace are byte-identical between Debug and Release. Every complete
  Debug frame was visually reviewed; the matching Release hashes cover the
  same frames.
- **Independent visual check:** reviewed the whole independent source frames
  listed above and the native frames at opening frames `1827`, `1833`, `1839`,
  `1845`, `1851`, `1857`, and `1863`. The top rotor strip changes in both
  sequences. The longplay frames are an appearance reference, not a VBlank
  oracle.
- **Reported defect resolved:** **No.** The six-phase upper rotor animation
  is present and source-supported. If “rear propeller” means the circular
  tail detail embedded in `plane.a`, the available ROM data and independent
  frames both show it as static. Keep `OPEN-04` open until the reported part is
  tied to a source animation or a matching reference establishes different
  behavior.
