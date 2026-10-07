# Full intro capture review after wilderness pan fix

Date: 2026-10-06  
Build: Release Qt executable at build/conversion/qt/release  
Route: Gator Shaman, 2,700 title-idle frames, complete opening, field handoff,
and stable field hold.

## Capture and review

The CLI full-intro capture completed with exit code 0. It wrote 6,782
consecutive 320x224 PNG frames (113.0 seconds at 60 fps), phase boundaries,
and a per-frame trace under
build/visual-audits/intro-full/release-gator-shaman-20261006-panfix/.
The event trace shows pre-title frames 0-559, title idle 560-3259, Start and
archetype selection through 3433, opening through 6661, then 120 stable field
frames through 6781.

All 18 contact sheets, contact-00.png through contact-17.png, were reviewed
in order. They cover 1,131 full-screen samples at 10 fps, six capture frames
apart. Full-size frames were also checked around the title/menu handoffs, the
Gator Shaman pan, story/cockpit transition, aircraft and cab transition, and
field arrival. Audio is not part of this image capture.

## Whole-run findings

- The capture starts from the pre-title state and includes the reported black
  interval, SEGA reveal, circuit arrival, title, menus, complete story, and
  destination handoff. The title circuit remains absent until after the logo
  reveal; the separate title audit records the exact frame boundary and
  independent section 01 support.
- The title stays stable for the requested idle interval. Start opens the
  shell before its text, New Game opens archetype selection, and Gator Shaman
  proceeds through the opening sequence.
- OPEN-02 is corrected for the wilderness pan. The named body layers now move
  with the typed scroll state, and the head/body relation is stable at start,
  midpoint, and settled states. See
  [the source and nine-question audit](gator-shaman-head-pan.md).
- OPEN-04 is an actual six-state rear-propeller animation in the source, not
  an ambiguous static detail. Its prior source trace, QA, and independent
  section 02 comparison remain recorded in
  [the propeller audit](airport-rotor-and-aircraft-tail.md).
- The tri-vid/cockpit, Michael death-image lifetime, and skyline layering
  remain OPEN-03 and OPEN-06. The full-screen capture includes the cockpit
  entry, monitor content, later screen changes, and exit; the scoped layer
  order and death-image source lifetime have not yet been traced, so they are
  not claimed fixed.
- The aircraft sequence continues into the cab-map scene and the field.
  The last 120 frames remain stable. The process neither stalled nor crashed
  during this route.

## Verification limits and next work

The 10 fps contact-sheet sweep is for whole-run ordering and stale-layer
review. The exact per-frame PNGs remain available; brief cadence claims use
source counters plus the dedicated frame QA rather than the contact sheets.
Native captures validate the conversion's own behavior, not Genesis fidelity.
Independent section 02 frames 000480, 000510, 000570, and 000630 support the
Gator Shaman pan composition. Other opening scenes still need their own
source-traced, whole-screen comparisons before those goals can be closed.

The capture has no audio track. The missing SEGA shock effect and opening
music event questions remain open under BOOT-01 and OPEN-01; a successful
image capture is not audio evidence.
