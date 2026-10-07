# SEGA shock sound source and playback audit

Date: 2026-10-06  
Scope: `BOOT-01`, missing or mistimed audio during the SEGA lightning/shock
sequence.

## ROM source and sound identity

The US ROM is primary; the embedded Z80 driver bytes were disassembled to
identify what each 68K service request does.

- At `0x5D7F2` and `0x5D80A`, the pre-title loop sends command `0x16` through
  `0x530D2` when the logo-effect counter is below six and when it reaches zero.
  `0x530D2` calls `0x53112`, which writes Z80 command `FF 12 16`. Z80 command
  `0x12` stops active channels whose resource-id field equals 22; it does not
  start the similarly named `effect-016` asset.
- At `0x5D82E`, the loop sends command `0x5C` through `0x530DE` while the
  post-decrement counter remains above four. Reset setup at `0x007A2` sets
  `FFFC08=1` for this boot. The `0x530DE` route therefore sends `FF 12 5C`
  (stop resource 92), then services the queued `FF 10 5C` request. Z80 command
  `0x10` starts the matching table-2 resource.
- Table-2 resource 92 is the extracted `audio.effects.resource.092`,
  `audio/sound-effects/resource-092.flac`. It is 44.1 kHz stereo and measures
  2,750 ms. It is distinct from `audio.field.effect-016`.

Thus `0x16` is stop-only; `0x5C` is the SEGA shock sound's stop-and-start
request. The source callback now calls `stop_event(0x16)` for the former and
`play_event_restarting(0x5C)` for the latter. The restart operation stops all
matching active host channels before starting one channel from position zero.
No `0x16` audio alias is registered.

## Timing and independent reference

The pre-title source emits the `0x5C` request only while the typed loop is at
the SEGA shock condition (`figure_y == 0x160` and post-decrement counter above
four). In the complete Release capture, service-boundary markers for this
path occur at capture frames 94, 96, ... 212 (60 requests). The stop-only
`0x16` path is separately marked from frames 214 through 336 (63 requests,
including the source's two requests at the zero transition). These are
conversion-capture frame indices, not a claimed independent VBlank clock.
The screenshots at frames 94 and 212 show the SEGA logo, figure, and lightning;
frame 336 is after the figure has left. No such command is issued on Decker's
opening entry.

Independent section 01 frame 120 shows the same lightning around the SEGA
logo; frame 134 shows the logo after the lightning has cleared. On the
independent `intro_audio.wav`, resource 92 has the strongest candidate
correlation among the two similarly named assets (normalized score 0.5589 at
4.225 s; `effect-016` scores 0.5011 at that point). The correlation is
supporting evidence only: the ROM driver trace identifies which table resource
the source actually starts. The video is sampled at 30 fps and includes
pre-roll, so it does not prove exact VBlank-to-audio latency.

## Playback verification

`shadowrun_ghidra_audio_playback_test` exercises the real `QMediaPlayer` on the
default output. It verifies that `0x5C` resolves to resource 92, becomes
loaded/playing, reports a 21 ms start latency and 2,750 ms duration, and can be
restarted while leaving exactly one channel playing. It then stops the event
and verifies zero active channels. The test also confirms `0x16` has no alias
and does not reach the asset resolver. The complete Debug and Release Qt
CTest suites pass 62/62.

The no-argument Release application was launched and its window captured at
one, five, and thirteen seconds. It rendered the SEGA animation, stayed alive,
and advanced into `title_ready`. The executable is now linked as Windows GUI
subsystem so the multimedia FFmpeg diagnostics do not open a console over the
game. The CLI `--help` path still returns its usage text.

## Nine-question review

1. **Does it look correct?** The source-timed shock sound is now present at the
   SEGA lightning sequence. Native full-screen frames 94 and 212 show the
   source shock state; independent section 01 frames 120 and 134 show the
   corresponding lightning-on then lightning-off visual states.
2. **Does appearance match behavioral state?** Yes. The typed pre-title
   request's command and service path choose stop-only resource 22 (`0x16`)
   versus stop-and-start resource 92 (`0x5C`). The resource-92 requests are
   bounded by the SEGA shock counter, not the later archetype/opening input.
3. **Are the visuals correct?** Audio wiring does not alter scene pixels. The
   new 6,782-frame Release capture is byte-identical in every frame to the
   previously reviewed post-clear run. Full-screen shock checkpoints and the
   independent reference were inspected; no new screen layer or control is
   drawn for audio.
4. **Is it animated?** Yes, the figure/lightning is animated under the
   source's pre-title cadence. The audio request repeats every two captured
   source updates during the counter-gated shock phase. Each request restarts
   the same 2.750-second effect; the final request is allowed to finish
   naturally.
5. **Is there sound? What and how long?** Yes: table-2 resource 92, measured
   at 2,750 ms. A retrigger stops the previous matching channel before the
   next start. `0x16` is not a sound start; its source operation stops resource
   22 only.
6. **Are the rest of the screen and prior controls correct?** The complete
   run covers the FASA/black/title handoffs, title menus, all Gator Shaman
   scenes, field handoff, and stable field hold. This slice introduces no UI;
   the startup callback does not place buttons or graphics over existing
   panels.
7. **Do multiple frames support this reading?** Yes: native frames 94, 212,
   214, and 336 span the shock requests and stop-only tail. Independent
   frames 120 and 134 bracket lightning presence. Independent audio correlation
   supports resource 92, while ROM/Z80 tracing selects it conclusively.
8. **Are old UI elements cleanly replaced?** No UI is introduced during the
   shock event. The full capture shows the black/FASA/title transitions use
   their existing full-screen redraw and contain no controls left behind.
9. **Was the whole window reviewed?** Yes. The Release process captured all
   6,782 frames through field arrival and the 120-frame stable hold, exiting
   normally. Compared with the post-OPEN-03 full run, every PNG is byte
   identical; the earlier pre-clear baseline differs only in 151 death-pose
   frames within the stale left-plate rectangle, and all 151 old/new pairs were
   reviewed in eight contact sheets.

## Result

- **Code changed:** the pre-title Qt callback now applies stop/restart
  semantics for the two source requests, and the Release executable no longer
  launches with a visible CUI console.
- **Behavior implemented:** `0x5C` plays and restarts resource 92 at the
  source's SEGA shock requests; `0x16` remains a stop-only operation.
- **Independently checked:** the named resource correlates with the recorded
  SEGA sound; the independent frames show the shock sequence. Playback was
  decoded and started on the configured default output.
- **Reported defect resolved:** `BOOT-01` is resolved for the missing/mistimed
  shock effect. The independent clip does not supply exact VBlank phase, and
  stage-2 opening music remains separately unresolved under `OPEN-01`.
