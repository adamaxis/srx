# SEGA shock sound source and playback audit

Initial audit: 2026-10-07; correction: 2026-10-10

Scope: `BOOT-01` event placement and `BOOT-06` fragmented playback during the
SEGA lightning/shock sequence.

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
other `FF 10 16` start callers. The ROM issues 60 `FF 10 5C` requests, one
every two VBlanks, while the shock counter is above four. A separate Z80
driver probe keys six FM channels for those requests; that fact does not mean
the extracted host FLAC is an individual channel stem. Resource 92 is already
a complete stereo mix. The boot path does not send an explicit stop for it.

### Historical host playback, superseded 2026-10-10

The first implementation replayed the complete resource-92 FLAC on every
request into six Qt players. Although that matched the count of active YM
channels in a synthetic driver trace, it layered full stereo renders and
produced the fragmented/incorrect cue reported by the user. Its six-player
CTest and old loopback correlations were not fidelity evidence. They are
retained below only as historical artifacts; the current behavior and evidence
follow in the correction sections.

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
5, 6, and 2 before a prior channel is keyed off and reused. This describes the
source driver's voice allocation. It does not establish that a complete
stereo host asset should be replayed six times; the independent mix comparison
below and current Release capture support one complete cue for this sequence.

Independent section 01 frame 120 shows the same lightning around the SEGA
logo; frame 134 shows the logo after the lightning has cleared. On the
independent `intro_audio.wav`, resource 92 has the strongest candidate
correlation among the two similarly named assets (normalized score 0.5589 at
4.225 s; `effect-016` scores 0.5011 at that point). The correlation is
supporting evidence only: the ROM driver trace identifies which table resource
the source actually starts. The video is sampled at 30 fps and includes
pre-roll, so it does not prove exact VBlank-to-audio latency.

## Current playback verification (2026-10-10)

`shadowrun_ghidra_audio_playback_test` exercises the real `QMediaPlayer` on the
default output. The test verifies that `0x5C` resolves to resource 92, becomes
loaded/playing, and reports a 2,750 ms duration. It then feeds all 60 source
requests through the sequence one-shot policy and verifies exactly one player
remains active. Starting a fresh sequence plays one new cue. The resource-22
`FF 12` stop remains distinct from the `FF 10 16` start alias. Debug and
Release targeted audio tests pass; resource 92 reports 2,750 ms and about
20-30 ms initial start latency.

The earlier six-player WASAPI capture is superseded. A fresh process-specific
WASAPI loopback recording from the Release game executable
(`build/visual-review/boot-shock-release-game-loopback-one-shot.wav`, ignored
build output) captured the single cue from the actual Windows output mix. Its
four consecutive 100 ms windows match resource 92 at NCC 1.0000 (capture
positions 2.1681, 2.2681, 2.3681, and 2.4681 s). This validates the current
host output against its decoded source asset; the separate longplay comparison
below is the independent game-audio evidence.

The pre-fix ordinary-game loopback with repeated low correlations is retained
as a historical artifact at `build/visual-review/boot-shock-release-game-loopback.wav`.

The no-argument Release application was launched and its window captured at
one, five, and thirteen seconds. It rendered the SEGA animation, stayed alive,
and advanced into `title_ready`. The executable is now linked as Windows GUI
subsystem so the multimedia FFmpeg diagnostics do not open a console over the
game. The CLI `--help` path still returns its usage text.

## Nine-question review (current one-shot correction)

1. **Does it look correct?** Sound is heard during the ROM-gated SEGA
   lightning sequence. Independent section 01 frames 120 and 134 show
   lightning present then cleared; native full-screen frames 94 and 212
   bracket the source requests.
2. **Does appearance match behavioral state?** The typed pre-title request
   preserves all 60 ROM requests. `0x5C` uses `FF 10` to start resource 92;
   `0x16` uses the distinct `FF 12` path to stop resource 22. The host cue is
   latched once per source sequence because its FLAC already contains the
   complete stereo mix. A new attract sequence clears that latch.
3. **Are the visuals correct?** Audio changes no pixels. The startup visuals
   were reviewed at the shock checkpoints; no UI or graphic layer is
   introduced by this change. The audio-only callback edit does not change
   screen pixels.
4. **Is it animated?** The figure/lightning follows the source pre-title
   cadence. The source requests audio every two VBlanks during its counter
   window. Host audio starts the full cue once for that sequence rather than
   layering repeated copies of the full mix.
5. **Is there sound? What is it and how long does it play?** Resource 92 is
   44.1 kHz stereo and 2,750 ms including its release/silence tail; about 0.8 s
   is audible. Current Release WASAPI loopback matches the decoded cue over
   four consecutive windows at NCC 1.0000. Independent longplay windows match
   a single render as described above. Exact VBlank-to-output latency remains
   bounded by the 30-fps reference.
6. **Are related screen parts also correct?** The callback is confined to
   startup audio. Full-screen checkpoints include the logo, lightning, FASA,
   black/title handoffs, title menus, opening, and field arrival. No unrelated
   control or graphic was added.
7. **Do multiple frames/examples support this reading?** Independent video
   provides lightning-on/off frames and four consecutive audio windows;
   current Release loopback provides four consecutive decoded-cue windows.
   The ROM/Z80 path independently identifies the resource and request cadence.
8. **Are old UI elements cleanly replaced?** No UI is introduced. Full-screen
   handoff captures retain the existing black/FASA/title redraws with no stale
   controls overlaid.
9. **Was the full changed screen/time window reviewed?** The startup whole-run
   capture contains 6,782 frames through field arrival. This audio-only edit
   does not change those pixels; the shock frames and audio window were checked
   specifically. Other startup defects remain tracked separately.

## Result

- **Code changed:** the Qt audio service exposes a source-sequence one-shot
  latch. Pre-title reset clears it; each `0x5C` request remains recorded, but
  only the first starts the complete resource-92 stereo cue. The boot `0x16`
  path remains `FF 12` stop-resource-22.
- **Behavior implemented:** one complete shock cue plays per pre-title
  sequence; a later attract-loop sequence can play it again. The source's 60
  requests and two-VBlank cadence remain represented. No `0x5C` request is
  issued at Decker's opening entry.
- **Independently checked:** four overlapping 200 ms windows, sampled 100 ms
  apart, match independent section 01 audio at NCC 0.9479, 0.9957, 0.9831,
  and 0.9529, at reference positions 4.5246, 4.6246, 4.7246, and 4.8246 s. A
  fresh Release process-loopback capture matches the asset at NCC 1.0000 over
  four consecutive windows. Debug/Release CTest verifies single playback and
  replay after sequence reset.
- **Reported defect resolved:** `BOOT-06` is resolved for the observed
  fragmentation; `BOOT-01` remains resolved for missing/mistimed event
  placement. Longplay timing is 30-fps and cannot establish exact VBlank to
  audio-device latency. Logo explosion/theme and other audio routes remain
  separate/open under `SOUND-03`.
