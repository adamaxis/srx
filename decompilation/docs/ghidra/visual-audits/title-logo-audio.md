# Title logo reveal and theme audio audit

Date: 2026-10-10
Scope: `BOOT-07`, the sound during the Shadowrun logo reveal and the title
theme that begins with the copyright row.

## ROM trace

The US ROM is primary; Ghidra output was used only as a cross-check.

- Title loop `0x1A9E` waits 90 updates before calling `0x1E4E`, then advances
  one logo column every two title updates.
- ROM bytes at `0x1E4E` read the column counter at `FF1E6`, test bit zero, and
  load `D0=0x5D` before calling `0x530DE` when the counter is even. The sound
  request is serviced before that column is uploaded. Across the 38 columns,
  this produces 19 requests for indices `0, 2, ..., 36`.
- Completing the source count sets the title-text state and a `0x5A` countdown.
  At `0x1C1E`, the countdown is decremented once per title update. On its
  negative transition, the ROM loads `D0=0`, calls `0x530EA`, then calls
  `0x1DF6` to draw the copyright row. The theme request therefore follows the
  logo reveal and precedes the copyright draw.
- The extracted audio catalog identifies command `0x5D` as table-2 resource
  93 (`audio.effects.resource.093`) and command `0x00` as music resource 0
  (`audio.music.resource.000`). Resource 93 is a 44.1 kHz stereo render of
  2.4503 seconds. Resource 0 is a 44.1 kHz stereo render of 120.0007 seconds;
  `loop-points.tsv` records its source loop segment at 40.48–80.78 seconds.

## Native playback

The title-loop conversion publishes each audio request at the source transition.
The original Qt callback mapped command `0x5D` directly to one standalone
resource-93 render and latched it once. That lost the other 18 ROM starts. The
corrected offline asset, `audio/intro/shadowrun-logo-reveal-sequence.flac`, is
rendered by replaying all 19 `FF 10 5D` commands through the ROM Z80 driver at
four-frame intervals, then rendering the resulting YM2612/SN76489 writes. Its
2.700-second endpoint is the ROM transition to the title theme: command `0x00`
stops/restarts the title resource at that point. Qt starts this source-cadenced
sequence once at the first `0x5D` callback; it does not layer complete stereo
copies or discard the source-timed retriggers. A new source sequence clears
the playback latch.

`shadowrun_ghidra_conversion_test` verifies the 19 even-column requests and the
theme request after the 91-update countdown transition, before copyright
presentation. `shadowrun_ghidra_audio_playback` verifies both catalog mappings,
FLAC decoding, default-output playback, reported durations, one active
composite channel across 19 source callbacks, replay on a new sequence, and
stop. Debug and Release CTests pass on the corrected sequence asset.
The Qt startup QA logs 19
`title.logo_reveal.audio_command_5d` boundaries followed by one
`title.theme.audio_command_00` boundary.

A Release path trace confirmed that the Qt runtime opened
`audio.intro.shadowrun-logo-reveal-sequence` and then the title music alias. A
fresh 16-second uninstrumented process-loopback capture matches the reveal
asset at full-window PCM NCC 0.9885 and 50 ms RMS envelope Pearson 1.000 across
the 2.5-second active interval. The reveal starts near 11.17 seconds; the title
music transition follows near 13.9 seconds. The earlier partial capture gave a
lower raw-sample score, so the full clean recapture is the current route
verification.

The independent Genesis longplay in section 01 supports the same order: frame
000342 shows the partial logo, frame 000384 shows the completed logo, and frame
000420 shows the copyright row with the circuit visible. Comparing 50 ms RMS
envelopes across the full 2.5-second active reveal interval gives Pearson 0.980
at about 11.25 seconds with a 1.03 time scale. Short 250 ms raw-audio windows
also support resource 93 and resource 0 identity at NCC 0.831 and 0.852. The
compressed video supports the repeated reveal envelope and ordering; it does
not establish exact VBlank timing, chip waveform phase, or speaker latency.

## Full-screen review

The audio change adds no UI or drawing operation. I reviewed the 134 complete
320x224 Release startup QA frames as contact sheets, then inspected the logo
and copyright boundary frames at native size. Debug and Release produce the
same 134 filenames and SHA-256 hashes. The complete logo is clean before the
circuit palette step; the copyright row appears after the source text delay;
Push Start, the Start Options shell, and its returned menu do not retain stale
controls from the preceding screen. The white target outline in the Seattle
opening capture is also present in independent section 02 frame 000417, so it
is not an extra control drawn over the skyline.

## Nine-question review

1. **Does it look correct?** The complete logo finishes before the copyright
   transition; the independent frames 000342, 000384, and 000420 support that
   order. The sound cue occurs during the rolling logo and the theme begins
   after it.
2. **Does the look match behavioral state?** Yes. The title loop records 19
   `0x5D` requests on even column indices, then one `0x00` request when the
   completed-logo text countdown becomes negative.
3. **Are the visuals correct?** No rendering code changed. Whole-screen QA
   confirms the black-circuit/logo-complete boundary, subsequent circuit
   appearance, copyright row, and prompt phases. Existing circuit color and
   texture concerns remain under `TITLE-03`.
4. **Is it animated?** Yes. The logo advances one column every two source
   updates. The reveal cue is requested on every other column, 19 starts total
   at four-frame intervals. The title theme starts after the completed-logo
   countdown, about 2.7 seconds after the first reveal request.
5. **What sound plays and how long?** The standalone resource-93 render is
   2.4503 seconds, but it represents only one start. The source-cadenced asset
   contains all 19 starts and ends at the 2.700-second ROM theme handoff. Its
   independent 50 ms amplitude envelope matches the longplay across the active
   reveal interval; the normal Release route starts the expected asset and its
   envelope matches at 1.000. This clean capture supersedes the earlier partial
   capture; the independent chip waveform phase remains unverified.
6. **Are other screen parts also correct?** The full QA frames show no new
   menu or intro controls placed over an old panel. The circuit, title text,
   prompt, options menu, and Seattle outline were checked against their state
   or reference frames; unrelated open defects remain tracked separately.
7. **Do multiple frames support the interpretation?** The independent frames
   000342/000384/000420 support logo completion, theme/copyright ordering, and
   the section-01 audio envelope matches the full repeated reveal at Pearson
   0.980. The Qt process capture separately confirms the expected sequence
   asset and matching amplitude envelope.
8. **Were old buttons or graphics left under new UI?** The audio transition
   draws no controls. The complete title and Start Options captures show clean
   transitions; the return-from-empty-load route clears the old menu text before
   redrawing the Start Options choices.
9. **Was the full affected screenshot set reviewed?** Yes. All 134 startup QA
   screenshots were reviewed in chronological contact sheets. Debug and
   Release captures are byte-identical; individual title transition frames
   were also inspected at 320x224.

## Disposition

- **Code changed:** the offline source driver emits repeated timed requests;
  Qt maps `0x5D` to a 19-start chip-rendered sequence and `0x00` to the theme.
- **Behavior implemented:** the host preserves every source retrigger and
  plays the reveal through its source boundary at the theme handoff.
- **Independently checked:** section-01 video and audio support logo order and
  the full 2.5-second reveal envelope (Pearson 0.980); the ordinary Release
  capture opened the expected asset and matches at PCM NCC 0.9885/envelope
  Pearson 1.000.
- **Reported defect:** `BOOT-07` is resolved for repeated-cue count and audible
  duration. Exact waveform phase and source-to-device timing remain bounded by
  chip differences and the 30-fps reference.
