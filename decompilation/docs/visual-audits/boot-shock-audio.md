# SEGA shock sound source and playback audit

Date: 2026-10-07

Scope: `BOOT-01`, missing or mistimed audio during the SEGA lightning/shock
sequence.

## ROM source and sound identity

The US ROM is primary; the embedded Z80 driver bytes were disassembled to
identify what each 68K service request does.

- At `0x5D7F2` and `0x5D80A`, the pre-title loop sends command `0x16` through
  `0x530D2` when the logo-effect counter is below six and when it reaches zero.
  `0x530D2` calls `0x53112`, which writes Z80 command `FF 12 16`. Z80 command
  `0x12` stops active channels whose resource-id field equals 22. The same
  command value can start resource 22 when another caller uses the `FF 10`
  queue path; the operation depends on the service path, not the ID alone.
- At `0x5D82E`, the loop sends command `0x5C` through `0x530DE` while the
  post-decrement counter remains above four. `0x530DE` tests `FFFC0A` and,
  when enabled, reaches `0x530FE`/`0x52F3C`, which queues `FF 10 5C`. Z80
  command `0x10` starts the table-2 resource. `FFFC08` belongs to separate
  routine `0x530EA`; it does not select the path from `0x530DE`.
- Table-2 resource 92 is the extracted `audio.effects.resource.092`,
  `audio/sound-effects/resource-092.flac`. It is 44.1 kHz stereo and measures
  2,750 ms. It is distinct from `audio.field.effect-016`.

Thus the boot caller uses `0x16` only to stop resource 22, while `0x5C` starts
resource 92. Resource 22 is also registered as `audio.field.effect-016` for
other `FF 10 16` start callers. A 60-request ROM-driver trace, with one
`FF 10 5C` every two VBlanks, keys six distinct YM2612 FM voices before the
first voice is reused. Earlier voices continue across later start requests;
`FF 10` does not stop all active copies of the same resource. The boot path
does not send an explicit stop for resource 92.

The prior Qt callback incorrectly stopped every host channel for resource 92
before each request, leaving only a short fragment between 33 ms retriggers.
It now submits each request as a new one-shot into a six-voice host pool;
reusing a slot stops only that individual player. The separate `FF 12 16`
resource-22 stop remains unchanged.

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

In the offline driver probe, the repeated starts occupy YM channels 0, 1, 4,
5, 6, and 2 before a prior channel is keyed off and reused. This is why the
host reproduces overlapping voices instead of restarting the full resource on
every source request.

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
loaded/playing, and reports a 2,750 ms duration. It then submits 60 requests
at 33 ms intervals and verifies six overlapping players remain in
`PlayingState`; the stop operation clears all six. It also verifies that an
`FF 10 16` start resolves to resource 22, plays, and is stopped by the
resource-22 stop operation. The targeted audio CTest passes in Debug and
Release; each reports 2,750 ms for resource 92 and approximately 20-28 ms
initial start latency.

A Windows WASAPI loopback recording captured the Release test's default
`Speakers (Realtek(R) Audio)` output at 44.1 kHz stereo. The repeated-shock
window produces normalized correlation 0.370 against the first 80 ms of
resource 92 at 12.271 s in the capture, with further peaks at 12.411, 12.541,
12.741, 13.331, and 13.631 s. The WAV is retained under the ignored build
artifact `build/visual-review/boot-shock-release-loopback-aligned.wav`. This
confirms audio reached the actual Windows output mix; it is separate from the
independent Genesis audio comparison below.

I also captured the ordinary Release game executable, launched in its normal
game mode with the extracted asset directory. Its 10-second WASAPI recording
has repeated normalized matches to resource 92 through the startup burst: a
0.764 match for the initial 80 ms at 1.641 s, followed by 0.40-level matches
at 1.971, 2.371, 2.471, 2.841, 3.171, 3.301, and 3.571 s. The output RMS
remains near 0.036 during the burst and falls below 0.005 after 4.3 s. The
recording is `build/visual-review/boot-shock-release-game-loopback.wav` under
the ignored build directory.

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
   request's command and service path distinguish direct stop resource 22
   (`0x16`, `FF 12`) from queued start resource 92 (`0x5C`, `FF 10`). The
   repeated resource-92 start requests are bounded by the SEGA shock counter,
   not the later archetype/opening input. Event ID `0x16` maps to resource 22
   for `FF 10` callers, while this boot callback follows its traced `FF 12`
   stop operation.
3. **Are the visuals correct?** Audio wiring does not alter scene pixels. The
   new 6,782-frame Release capture is byte-identical in every frame to the
   previously reviewed post-clear run. Full-screen shock checkpoints and the
   independent reference were inspected; no new screen layer or control is
   drawn for audio.
4. **Is it animated?** Yes, the figure/lightning is animated under the
   source's pre-title cadence. The audio request repeats every two captured
   source updates during the counter-gated shock phase. The six-voice native
   pool overlaps those starts and recycles each channel individually.
5. **Is there sound? What and how long?** Yes: table-2 resource 92, measured
   at 2,750 ms including the extracted release tail. WASAPI loopback confirms
   the rapid burst reached the current Windows default output. The boot
   caller's `0x16` is a separate stop operation for resource 22; other callers
   can start resource 22 with `FF 10 16`.
6. **Are the rest of the screen and prior controls correct?** The complete
   run covers the FASA/black/title handoffs, title menus, all Gator Shaman
   scenes, field handoff, and stable field hold. This slice introduces no UI;
   the startup callback does not place buttons or graphics over existing
   panels.
7. **Do multiple frames support this reading?** Yes: native frames 94, 212,
   214, and 336 span the shock requests and stop-only tail. Independent
   frames 120 and 134 bracket lightning presence. Independent audio correlation
   supports resource 92; the ROM/Z80 trace selects it and shows the six-voice
   overlap behavior.
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

- **Code changed:** each `0x5C` request now starts another host voice in a
  six-player pool; only the selected player is stopped when that slot is
  reused. The boot `0x16` path still applies the `FF 12` stop to resource 22.
- **Behavior implemented:** 60 source-timed requests during the SEGA shock
  produce overlapping playback. No `0x5C` request is issued on Decker's
  opening entry.
- **Independently checked:** independent Genesis frames/audio support the
  shock event and resource identity. Debug/Release QA verifies six active Qt
  voices, and WASAPI loopback captured repeated resource-92 matches on the
  current Windows default output.
- **Reported defect resolved:** `BOOT-01` is resolved for the missing sound
  and mistimed Decker-start playback. Exact alignment to a Genesis VBlank
  remains bounded by the 30-fps reference; unrelated opening-music questions
  remain under `OPEN-01`.
