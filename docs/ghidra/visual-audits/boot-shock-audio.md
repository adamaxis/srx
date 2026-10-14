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
every two VBlanks, while the shock counter is above four. The catalog
`resource-092.flac` is a render of one start; it is not the complete boot
sequence. Replaying the ROM driver with all 60 starts at the source cadence
produces 1,605 YM2612/SN76489 writes, 30 YM key-on writes, and 25 stream
restarts. The chip output decays through about 2.84 seconds after the first
request. There is no explicit `FF 12 5C` stop in the pre-title path.

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
source driver's voice allocation. The single-start catalog FLAC is neither a
channel stem nor the complete 60-request render; the new sequence asset
captures the driver and chip writes for the complete request window.

Independent section 01 frame 120 shows the same lightning around the SEGA
logo; frame 134 shows the logo after the lightning has cleared. Comparing raw
PCM samples from a software YM/PSG render to the compressed longplay gives weak
full-waveform correlation (0.041); sample phase is not a useful duration test
across different chip implementations and video encoding. A 50 ms RMS envelope
comparison across the 2.8-second active render instead gives Pearson 0.983 at
about 2.35 s in section 01, with a 0.955 time scale. This aligns the shock's
attack and decay with the independent capture; the matching frames show the
lightning present during the sound and absent afterward. The 30-fps visual
recording still bounds exact frame-to-sound latency.

## Source-cadenced host correction (2026-10-10)

The prior Qt route played the single-start `resource-092.flac` once and ignored
the other 59 source starts. That implementation was too short; its playback
and short-window checks did not defend the full sound sequence. The offline
extractor now supports repeated source-frame command schedules and can rebase
the VGM clock to the first request. Its output is rendered through libgme's
YM2612/SN76489 emulation into
`audio/intro/sega-logo-shock-sequence.flac`. The Qt alias for `0x5C` selects
that complete sequence render. Runtime starts one file once because it already
contains all 60 chip-level starts and their natural envelopes; it does not
layer 60 stereo copies.

The sequence FLAC is 4,842 ms including a two-second release tail. Its
rendered chip waveform remains above the extraction threshold through about
2,842 ms after the first request. The 60 source requests span 1,967 ms; the
remaining audible interval is the driver's continued voice/envelope output.
The resource-22 `FF 12` stop remains distinct from the `FF 10 16` start alias.
The Debug/Release audio test checks this sequence asset, its 4,842 ms duration,
one host player while the 60 ROM callbacks arrive, and replay after a new
source sequence. A Release path trace confirms the runtime opens this sequence
asset. A fresh 16-second uninstrumented process-loopback capture matches the
FLAC at full-window PCM NCC 0.9998 and 50 ms envelope Pearson 1.0000; both
remain active through about 2.75 seconds at a 2% peak-RMS threshold. The earlier
one-shot route's loopback is historical and is not used as current evidence.

The pre-fix ordinary-game loopback with repeated low correlations is retained
as a historical artifact at `archive/qa-artifacts/visual-review/boot-shock-release-game-loopback.wav`.

The no-argument Release application was launched and its window captured at
one, five, and thirteen seconds. It rendered the SEGA animation, stayed alive,
and advanced into `title_ready`. The executable is now linked as Windows GUI
subsystem so the multimedia FFmpeg diagnostics do not open a console over the
game. The CLI `--help` path still returns its usage text.

## Nine-question review (source-cadenced correction)

1. **Does it look correct?** This is audio-only; no visual layer changed.
   Independent section 01 frames 120 and 134 show lightning present then
   cleared, while native full-screen frames 94 and 212 bracket the source
   requests.
2. **Does the sound reflect behavioral state?** Yes at the request and
   driver level: all 60 `FF 10 5C` starts are represented at two-frame
   spacing in the source-cadenced asset. The adjacent `0x16` operation remains
   the distinct `FF 12` stop for resource 22. The host starts one composite
   render at the first request; the asset itself contains the repeated starts.
3. **Are the visuals correct?** Audio changes no pixels. The startup visuals
   were reviewed at the shock checkpoints; no UI or graphic layer is
   introduced by this change. The audio-only callback edit does not change
   screen pixels.
4. **Is it animated?** The figure/lightning follows the source pre-title
   cadence. Audio starts are requested every two VBlanks; Z80 stream updates,
   YM key-ons, and chip envelopes continue after the final request.
5. **Is there sound? What is it and how long does it play?** The single-start
   resource is 2,750 ms overall but has only about 0.8 s of sound. The
   60-request composite is 4,842 ms including its release tail, with chip
   output through about 2,842 ms after the first start. The independent
   longplay's 50 ms amplitude envelope supports the full attack/decay interval;
   ordinary Release loopback matches the generated asset envelope separately.
6. **Are related screen parts also correct?** The callback is confined to
   startup audio. Full-screen checkpoints include the logo, lightning, FASA,
   black/title handoffs, title menus, opening, and field arrival. No unrelated
   control or graphic was added.
7. **Do multiple frames/examples support this reading?** Independent video
   provides lightning-on/off frames, and its full 2.8-second audio envelope
   matches the source-cadenced render at Pearson 0.983. The ROM/Z80 trace
   establishes all 60 starts; the compressed reference confirms the overall
   envelope, not exact chip waveform phase or VBlank latency.
8. **Are old UI elements cleanly replaced?** No UI is introduced. Full-screen
   handoff captures retain the existing black/FASA/title redraws with no stale
   controls overlaid.
9. **Was the full changed screen/time window reviewed?** The startup whole-run
   capture contains 6,782 frames through field arrival. This audio-only edit
   does not change those pixels; the shock frames and audio window were checked
   specifically. Other startup defects remain tracked separately.

## Result (source behavior and audible envelope verified)

- **Code changed:** the offline extractor now queues repeated command starts
  at source-frame intervals and rebases the VGM timeline. Qt maps `0x5C` to
  the 60-start source-cadenced asset. Pre-title reset clears its playback
  latch for attract-loop replay. The boot `0x16` path remains `FF 12`
  stop-resource-22.
- **Behavior implemented:** the host no longer drops 59 of the 60 source
  starts by playing only the single-start resource. It plays one
  driver-generated YM2612/SN76489 sequence. No `0x5C` request is issued at
  Decker's opening entry.
- **Checked:** the ROM-driven VGM contains 1,605 chip writes and 30 YM
  key-on writes; the FLAC is 4,842 ms with chip output through about 2,842 ms.
  Debug/Release Qt tests pass; fresh Release output matches the expected asset
  at PCM NCC 0.9998 and envelope Pearson 1.000. The independent longplay
  envelope matches at Pearson 0.983.
- **Reported defect status:** `BOOT-06` is resolved for the missing repeated
  starts and short audible duration. Exact waveform phase and source-to-device
  latency remain limited by chip differences and the 30-fps reference.
  `BOOT-01` remains resolved for sound placement; logo reveal/theme are
  tracked under `BOOT-07`.
