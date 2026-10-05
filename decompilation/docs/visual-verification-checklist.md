# Visual Verification Checklist

Run these questions for every screen, menu, selector, dialog, and animation
change. Record the evidence and an explicit `yes`, `no`, or `unknown` for each
question; do not treat a plausible screenshot as proof by itself.

1. **Does it look correct?** Compare the frame with the strongest available
   real-game reference. Record the source, frame numbers, crop, and any limits
   on the comparison.
2. **Does the visible look match the behavioral state?** Verify selection,
   focus, enabled/disabled state, active character, counters, and result text
   against the input transition and source state. Gather ROM or runtime
   evidence for each visible state marker.
3. **Are the visuals themselves correct?** Check the selected asset, palette,
   dimensions, placement, text, transparency, outline thickness, and clipping.
   Compare every item with the source draw path and reference pixels.
4. **Should anything animate?** Identify whether it animates, its source
   counter/cadence, and the conditions that start, pause, or stop it. Inspect
   multiple frames across both active and idle input paths.
5. **Does it have sound?** Trace the triggering event to a source sound or
   resource, then check when it starts, its measured duration, and its stop or
   release behavior. Verify that the host player supports the asset's stored
   format, decodes it, reports audio available, and enters its playing state on
   the configured output device. A resolver or event-alias assertion alone does
   not prove playback. Capture output loopback when practical; distinguish that
   from player-state evidence. If no source call is found, record `no call
   traced`; do not invent a sound or duration.
6. **Are the other visible parts of the screen also correct?** Review the full
   frame, nearby panels, HUD, text, counters, and transition state. Follow
   related controls when one anomaly could affect another.
7. **Do multiple frames support the interpretation?** Find examples from
   separate input moments for the same interface: open it, move, confirm,
   cancel, and return as applicable. If the supplied recording has no such
   path, record that independent evidence is unavailable.
8. **Does the new UI replace the old UI cleanly?** Confirm that old buttons,
   graphics, text, and selectors are hidden or cleared before drawing the new
   page. Check entry, nested-page replacement, and return against a freshly
   rendered parent or field frame; look for controls accidentally drawn over
   controls that should no longer exist.

For each feature report these outcomes separately:

- **Code changed:** identify the implementation.
- **Behavior implemented:** identify the transitions and state covered.
- **Internally checked:** name the test and frame dump; state what it proves.
- **Independently visually checked:** identify the real-game capture used, or
  say that no matching capture was found.
- **Reported defect resolved:** claim this only when the user's reported
  behavior or appearance has independent supporting evidence.
