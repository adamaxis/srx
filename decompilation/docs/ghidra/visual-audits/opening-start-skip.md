# Opening Start-skip handoff

## ROM path and runtime state

The opening skip is a ROM Start-button path, not a cinematic-specific
controller animation. ROM `0x0BF2` reads the sampled controller words, checks
the Start bit (`0x8000`), and writes the opening exit flag at `0xFFDDC2`.
The check points are specific. `0x5AF62`, `0x59D7C`, `0x5AA96`, and `0x5AD1A`
sample and latch during their own timed loops. After the schedule, stage 2,
and tri-vid orchestrator return, `0x59C5E` takes the Start exit directly to
`0x59CFE`. Stage 2's first loop can stop, but its second do-while still renders
one pass before returning. The airport loop is different: `0x59C5E` clears its
stop marker after `0x5AD1A` and always enters the post-intro cab route at
`0x59D12`; a later Start can exit from that route's own `0x134D2` loop.
Palette-transition work does not latch Start, so an input sampled immediately
before a child loop remains pending until the loop's latch pass.

On the early-stage exit path, `0x59CFE` samples the controller, steps the
palette toward black, calls mode-entry preparation, and returns to the caller.
The New Game caller then continues at `0x07F0` through ordinary field setup.
A, B, and C do not set this Start flag.

The Qt conversion now gives the opening its own sampled/latching Start word.
Only Space/Enter (host Start) is forwarded while the opening is active; A/B/C
are ignored as skip commands. The converted loops sample and check Start at the
ROM loop boundaries above. An early-stage exit runs the `0x59CFE` tail and
enters `field_ready`; the airport loop instead clears its marker and completes
the cab route unless a new Start edge is latched there. The early exit path
bootstraps scene 2,
sets the New Game opening marker, and dispatches the scene-entry Lua rule
`field_events.new_game_opening_walk`. The script yields a heading-zero player
walk. The Release trace records the runner at `(0x3CE0, 0x2D00)` after field
bootstrap and `(0x3CE0, 0x2A58)` at arrival. Without further controller input,
the normal field controller reaches the Stoker arrival dialogue after 34
field frames in the current QA route.

## Evidence and visual review

The CTest `shadowrun_ghidra_conversion_qt_opening_start_skip_qa` advances the
opening, sends A and verifies that it continues, sends Start, and asserts the
source sample/latch/`0x59CFE` callback order in the same source pass. It then
advances the field with no further input. Integration tests also cover the
stage-2 second-loop pass, deferred input during a palette transition, and the
airport marker clear followed by normal cab-route completion. The Qt test
writes two full 320x224 frames:

- `qt_runtime/opening-start-skip-qa/actual.bmp`: field immediately after the
  Start exit and New Game bootstrap.
- `qt_runtime/opening-start-skip-qa/stoker-arrival.bmp`: the scripted
  zero-input walk has reached the Stoker arrival page.

The complete field frame has the scene-2 background, one controlled runner,
and both field HUD regions. It contains no title/menu shell or stale dialogue
controls. The arrival page fully replaces the field frame; it has Stoker's
portrait and sign text, one A continuation, and the outward-facing `< MORE >`
marker. This pass caught the field-font HFlip extraction defect; the named
font asset was regenerated from the US ROM and the fix is covered separately
in the [dialogue font flip audit](dialogue-font-flip-orientation.md).

Independent longplay section 03 frames `000038`-`000055` show the Redmond
street and its moving runner, and frame `000090` shows the Stoker arrival
panel. These references support the screen family, HUD, portrait, text, and
marker appearance. They do not show a Start-pressed route or expose the exact
input/field source-frame counter. The runner position in the immediate Qt
field dump is not presented as an exact longplay match.

## Nine-question visual verification

1. **Does it look correct? Yes for the field and arrival screen families.**
   Full-screen Qt outputs were compared with independent section 03 frames
   `000038`-`000055` and `000090`. Exact input timing has no external capture.
2. **Does the visible look match the behavioral state? Yes for this tested
   early-stage route.** The Start bit alone exits the schedule; A leaves the
   opening active. The resulting field state has the New Game marker and the
   Lua scene-entry walk active, then the Stoker prompt appears and that walk
   ends. The stage-2, airport, and post-intro gates follow their separate ROM
   paths described above.
3. **Are the visuals correct? Yes for the checked output.** The immediate
   frame draws the field map and HUD; the later frame has the ROM-backed
   portrait, text, panel, continuation marker, and corrected glyph direction.
4. **Should anything animate? Yes.** In the tested early route, the schedule
   stops at its next sample/latch boundary and the field runner walks. The
   field auto-walk and animation use the regular field source update loop and
   stop when Stoker's doorway event opens the arrival dialogue. Debug and
   Release integration tests check the other skip gates, but do not store a
   continuous per-frame capture for them. Independent video supports movement
   and arrival appearance, not exact cadence after a Start edge.
5. **Does it have sound? A stop request is traced; host cutoff duration is
   unmeasured.** ROM mode-entry preparation (`0x53126`) submits the source
   all-channel stop command (`FF 16`); the Qt callback stops active channels.
   No new cue is requested by the skip branch. This QA did not capture output
   loopback after pressing Start, so the audible cutoff is not declared
   independently resolved.
6. **Are the other visible parts correct? Yes for the two reviewed whole
   frames.** Scene art, HUD, runner, portrait, text, and panel were checked;
   no adjacent stale controls were found.
7. **Do multiple frames support the interpretation? Yes, with a limit.** The
   internal route writes both field and Stoker frames, while independent
   section 03 provides multiple street frames and the arrival page. There is
   no independent skipped-intro sample.
8. **Does the new UI replace the old UI cleanly? Yes.** The field handoff
   paints the field scene after startup, and the dialogue presenter clears the
   whole frame before drawing the arrival panel. No title buttons or stale
   field HUD remain behind the dialogue.
9. **Was the whole time window reviewed? Partial.** The test executes the
   whole opening skip, field bootstrap, Lua walk, and dialogue handoff, while
   this CLI QA stores the two full-screen state checkpoints. The ordinary
   opening capture is separately reviewed; the skipped route has no continuous
   per-source-frame capture or independent input record yet.

## Outcome

- **Code changed:** Start is sampled and latched at the traced ROM loop gates.
  Early-stage exits call the source tail; airport exit clears its marker and
  continues through the cab sequence; post-intro Start uses its own loop.
- **Behavior implemented:** A does not skip; Start during the schedule reaches
  scene 2; the zero-input Lua walk opens Stoker's arrival dialogue. Stage-2
  and airport edge cases follow their distinct ROM transitions.
- **Internally checked:** startup integration and Qt Start-skip CTests pass in
  Debug and Release; the Qt QA writes `actual.bmp` and `stoker-arrival.bmp`.
- **Independently visually checked:** section 03 street frames
  `000038`-`000055` and arrival frame `000090`. They do not prove a Start edge
  or exact skip timing.
- **Reported defect resolved:** the conversion implements the ROM Start gates
  and shared New Game handoff. Independent confirmation of the exact skipped
  route and host audio cutoff remains open.
