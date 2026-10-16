# Full intro capture review (pre-pan-fix baseline)

This is the historical baseline that reproduced OPEN-02. The corrected
whole-run recapture is documented in
[the post-fix intro review](full-intro-capture-2026-10-06-pan-fix.md).

Date: 2026-10-06
Build: Release Qt executable at `decompilation/ghidra_conversion/build/conversion/qt/release/`
Route: Gator Shaman selection, 2,700 title-idle frames, complete opening, field
handoff, and stable field hold.

## Capture record

Command:

```text
shadowrun_ghidra_conversion_qt_qa.exe --full-intro-capture 2 2700 <output-dir>
```

Output: `archive/qa-artifacts/visual-audits/intro-full/release-gator-shaman-20261006/`.
The executable wrote 6,782 consecutive PNGs at 320x224 (113.0 seconds at 60
frames per second), a phase-boundary log, and a per-frame trace. The process
returned exit code 0 after the field hold. The capture contains no video audio
track; it is not used as audio-playback evidence.

For the end-to-end visual sweep, all 18 contact sheets in
`contact-sheets-10fps/contact-00.png` through `contact-17.png` were reviewed.
They contain 1,131 evenly spaced samples, six frames apart, covering the full
capture at 10 samples per second. Every thumbnail is a complete game screen.
The original PNGs remain available for native-size review. Whole-size frames
at title/menu handoffs, story changes, cockpit transition, cab sequence, and
field arrival were inspected separately. This reviews the complete run while
keeping its exact per-frame records for short transitions.

## Whole-window findings

- The pre-title run includes the SEGA shock, lightning clear, discrete figure
  fade, FASA mark, black interval, title reveal, and circuit scroll. The title
  circuit is black through the logo completion state and begins appearing on
  the following source palette step. Whole-screen native frames 188 and 189
  record that boundary; independent section 01 frames 000384 and 000390
  support the logo-only then circuit-visible order. Circuit brightness and
  texture remain open as `TITLE-03`.
- The title remains stable for the requested idle interval. Start opens the
  shell and Start Options, the selected New Game route opens archetype choice,
  and Gator Shaman continues into the opening sequence.
- The Gator Shaman story does reproduce `OPEN-02`: native full-screen frames
  004190, 004340, and 004500 show the head separated from the body in the
  scene. This establishes current-build reproduction; ROM actor/layer
  ownership and the corrective change remain pending. The Decker label in an
  earlier defect note was wrong and has been replaced with Gator Shaman.
- The tri-vid/cockpit scene remains under review as `OPEN-03` and `OPEN-06`.
  This capture records its entry, monitor image, cleared monitor, and exit
  across full frames 004800–005140 and the following aircraft transition.
  Those frames alone do not prove the intended death-image lifetime or skyline
  depth; no layer-order fix is claimed here.
- The airplane/airport sequence continues into the cab-map transition, then
  reaches the field scene. The last 120 frames provide a stable field hold.
  No process exit, stall, or crash occurred before the capture completed.

## Evidence limits and follow-up

This capture exercises the current conversion and proves its process can
complete the tested route. Its own renderer is not an independent Genesis
golden, and the sampled contact sheets cannot establish exact source VBlank
timing. The independent section 01 frames support pre-title/title order. The
opening-story and cockpit defects still require ROM entry/state/draw/exit
traces, adjacent full-screen comparison with the numbered independent
section 02 frames, and a second complete native capture after any corrective
change.

Historical note: this capture note was written before the boot-audio wiring
and its path-sensitive correction. The current pre-title callback starts
resource 92 for the `0x5C` queue requests and stops resource 22 for the `0x16`
variant-service requests. The full frame capture itself contains no audio;
see [the boot shock audio audit](boot-shock-audio.md) for ROM/Z80 and playback
evidence.
