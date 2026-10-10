# Title Idle Attract-Loop Audit

Date: 2026-10-10

Scope: `TITLE-05`, natural no-input completion of the title and restart of
the boot/title sequence.

## ROM path and behavior

The primary source is the US ROM at `0x1A9E` and its caller `0x075E`;
`0x131C`, `0x1342`, and `0x53126` supply the transition and caller-service
details. Ghidra and exported disassembly are cross-checks.

- `0x1A9E` initializes the title countdown to `0x0E10`. It waits and updates
  the title once per pass, then takes the natural completion path after 3,601
  title updates. This differs from the Start-latch path: Start sets the
  activity state and enters Start Options, while a natural timeout leaves the
  activity state clear.
- The caller at `0x075E` runs the same six command-`0x14` audio service
  transactions on the no-input path, each followed by three VBlank waits: 18
  VBlanks total. With no activity, it does not enter Start Options. It runs a
  second `0x131C` transition (17 palette passes, two VBlanks per pass), then
  re-enters `run_game_loop()` and begins the normal pre-title sequence again.
- The restarted `0x5D746` pre-title sequence emits its normal shock request
  (`0x5C`). The source path therefore repeats the boot/title attract loop;
  it is not a terminal black screen.

The shipped independent longplay contains title frames and the Start Options
transition, but no matching 3,601-update no-input interval followed by a
replayed boot. That visual comparison remains unavailable.

## Visual verification checklist

1. **Does it look correct? — Internally consistent; independent loop example
   unavailable.** Debug and Release QA capture the timeout edge, both black
   intervals, a fresh boot, and restarted title. The restarted boot and title
   are pixel-identical to their first-pass images. No supplied Genesis frame
   sequence shows this full no-input route.
2. **Does the visible look match behavioral state? — Yes in the runtime QA.**
   The no-input countdown completes with activity clear; the first fade ends
   in black; the 18-VBlank caller service completes; the second fade finishes
   before state returns to `pre_title`; title countdown, reveal, and startup
   state are reset for the replay.
3. **Are the visuals correct? — Internally clean.** Whole 320x224 images are
   checked for black during the service hold, then compared pixel-for-pixel
   against the fresh boot and title. This establishes a clean redraw and no
   stale title/menu layers in this implementation, not Genesis pixel fidelity
   for the absent reference route.
4. **Should anything animate? — Yes.** The title keeps running through its
   3,601-update countdown. Its natural exit uses 17 palette passes across 34
   VBlanks. The caller holds black for 18 VBlanks, then performs a second
   17-pass/34-VBlank fade before restarting the pre-title choreography. The
   pre-title shock/strobe and logo timing then begin from reset. QA checks the
   countdown edge, both fade endpoints, and the first title reveal at update
   91; the other title animation cadences have their own prompt/scroll/circuit
   coverage.
5. **Does it have sound? — The source requests are traced; audible duration
   remains unresolved.** The caller issues six command-`0x14` transactions.
   The replayed pre-title route emits command `0x5C`, and QA verifies that
   request is published after reset. This test does not establish the
   command-`0x14` resource alias, output-device playback, or measured duration
   of either request. The independent longplay has no matching idle-replay
   audio segment.
6. **Are the other visible parts of the screen also correct? — Reviewed.** All
   full-screen images produced by the new QA route were inspected, including
   the initial boot/title, timeout edge, both black holds, replayed shock,
   restarted boot, and title-delay/reveal frames. The title and intro layers
   are cleared before restart; no Start Options controls leak into the replay.
7. **Do multiple frames support the interpretation? — Yes for the native
   state sequence; independent repeated examples are unavailable.** The QA
   captures multiple transition boundaries and compares both first/replay
   boot and title frames. The supplied longplay does not show a repeated idle
   route or controller/VBlank logs.
8. **Does the new UI replace the old UI cleanly? — Yes in the native replay.**
   Startup presentation layers/state are reset before entering `pre_title`,
   while named assets remain loaded. The new boot and title match their clean
   first-pass frames pixel-for-pixel; no stale menu, sprite, or opening layer
   remains.
9. **Was the whole affected time window reviewed? — Reviewed at the captured
   source boundaries, with a limit.** The QA advances from initial boot
   through timeout, both fades, caller service, replayed shock, title delay,
   and first logo column; it retains full-screen checkpoints and a phase trace.
   It does not save every one of the 3,601 repeated title frames because the
   title's recurring animation clocks are covered by separate cadence QA. The
   new path has no matching independent full-window capture.

## Implementation and verification

- **Code changed:** typed `TitleIdleRestartFadeState` models the caller's
  second fade. `restart_startup_attract_loop()` resets mutable startup/menu/
  opening state but retains the loaded named asset catalog. The Qt host starts
  this transition only after the idle path's caller service completes.
- **Behavior implemented:** no-input timeout now fades to black, completes
  the 18-VBlank service and second fade, then starts the normal boot sequence
  and reaches a clean title again. The replay issues the source shock request.
- **Internally checked:**
  `shadowrun_ghidra_conversion_qt_title_idle_restart_qa`, the adjacent
  startup/boot/audio tests, and the conversion tests passed in Debug and
  Release. CLI `actual.bmp` plus named full-screen checkpoints are written to
  `qt_runtime/title-idle-restart-qa/`; the trace records phase boundaries.
  The test checks the 3,601-update countdown, 34-VBlank fade, 18-VBlank
  service, second 34-VBlank fade, recurring shock request, and pixel equality
  for first/replay boot and title.
- **Independently visually checked:** no matching Genesis no-input replay was
  found in the supplied longplay. The independent reference confirms the
  ordinary title and title-to-options path only.
- **Reported defect status:** the Qt black-screen stall is fixed and the
  source-derived route is defended internally. Independent visual confirmation
  of the repeated idle route and audible duration of the caller service remain
  open.
