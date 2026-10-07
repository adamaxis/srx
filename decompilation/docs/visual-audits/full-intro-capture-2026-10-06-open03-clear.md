# Full intro capture after death-layer clear

Date: 2026-10-06
Build: Release Qt executable in `build/conversion/qt/release/`
Route: Gator Shaman, 2,700 title-idle frames, complete opening, field handoff,
and 120 stable field frames.

## Capture and whole-run review

Command:

```text
shadowrun_ghidra_conversion_qt.exe --full-intro-capture 2 2700 <output-dir>
```

The run exited with code 0 and wrote 6,782 consecutive 320x224 PNGs
(113.0 seconds at 60 fps), the phase-boundary log, and `frames.tsv` under
`build/visual-audits/intro-full/release-open03-clear-20261006/`. The process
reached the stable field hold without stalling or crashing. Eighteen contact
sheets sample every sixth frame from boot through field hold; all 18 were
reviewed in order. Full-size checkpoints were inspected around the title/menu
handoffs, story transition, death-pose clear, cockpit replacement, airport,
cab-map transition, and field arrival.

The intro proceeds through the black/logo intervals, title and menus, Seattle,
wilderness, tri-vid, airport, and cab-map scenes before field arrival. The
death-pose change is at clip frames 4642-4645 (opening frames 1208-1211): the
old actor pose is visible at 1208; the ROM clear and death pose are visible at
1209; the corrected layer stays absent at 1210-1211. The display then continues
through moving projectiles, cockpit replacement, the airport route, and the
field handoff. The known title-circuit brightness/density mismatch remains
`TITLE-03`; this capture does not close it.

## Death-layer findings

The ROM trace and typed clear are documented in the
[nine-question death-layer audit](michael-death-layer-clear.md). The earlier
implementation omitted the source's blank fill of the `$0600` left actor
plate, leaving that art visible under the final pose. The new callback clears
the persistent left map and overlay before the final death image is recorded.

The independent whole-screen reference frames are section 02 frames 730, 740,
750, 760, 770, and 780. Frame 730 shows the earlier pose; the later death-pose
frames have no old figure fragment at the left. Frame 770 includes a legitimate
projectile crossing that edge, so only the terminal-clear frame is required to
have a black edge. The QA checks that pixel condition at opening frame 1209 and
checks typed layer state for every remaining frame while the final pose is
active. It confirms the full wilderness-to-tri-vid replacement as well.

The source audio command `0x2E` is requested at the same terminal event. The
Qt audio service has no alias for it, and this image capture has no audio
track. Its identity and playback duration remain open as `OPEN-07`.

## Results and verification limits

- **Code changed:** the source conversion now exposes a semantic left-region
  clear; the Qt controller retires the old layers before drawing the final
  pose.
- **Behavior implemented:** the source's elapsed-`0x168` fill is represented
  as native typed layer state, and that clear persists until scene replacement.
- **Internally checked:** Debug and Release pass
  `shadowrun_ghidra_conversion_qt_title_intro_overlay_qa` and
  `shadowrun_ghidra_conversion_qt_opening_death_layer_qa`. The latter writes
  full-screen `actual.bmp` and event checkpoints. These checks cover callback
  order, the transition pixels, remaining pose lifetime, and tri-vid entry.
- **Independently visually checked:** the death sequence matches the
  appearance in section 02 frames 730-780. Their 30-fps sampling supports the
  visual order but not exact VBlank timing.
- **Reported defect resolved:** `OPEN-03` is resolved for the old left actor
  plate showing beneath the death pose. `OPEN-07` remains open for command
  `0x2E` sound identity, duration, and playback.

This full image capture is not audio evidence and is not an independent
Genesis pixel golden. It does demonstrate that the current Release executable
completes the entire captured route and reaches stable field output.
