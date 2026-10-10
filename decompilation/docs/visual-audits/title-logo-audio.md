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
The Qt callback maps command `0x5D` to resource 93 and command `0x00` to resource
0. The 19 logo requests remain observable, while the complete resource-93
render plays once per title sequence; the host does not layer 19 copies of a
complete stereo file. A new source sequence clears that one-shot latch. The
theme stops/restarts its own resource before playback, matching the title audio
command handoff.

`shadowrun_ghidra_conversion_test` verifies the 19 even-column requests and the
theme request after the 91-update countdown transition, before copyright
presentation. `shadowrun_ghidra_audio_playback` verifies both catalog mappings,
FLAC decoding, default-output playback, reported durations, one active resource
93 channel across 19 source requests, replay on a new sequence, and stop.
Debug and Release pass. The Qt startup QA logs 19
`title.logo_reveal.audio_command_5d` boundaries followed by one
`title.theme.audio_command_00` boundary.

A normal Release Qt process was captured through WASAPI application loopback.
The 44.1 kHz capture is `build/visual-review/title-logo-process-loopback.wav`.
After anti-aliased 4:1 resampling for rate alignment, overlapping 400 ms
windows match resource 93 at NCC `0.964`, `0.918`, `0.906`, `0.987`, and `0.994`
for source offsets 0, 0.2, 0.5, 1.0, and 1.5 seconds. The same capture matches
resource 0 at NCC `0.995`, `0.997`, `0.998`, `0.991`, `0.996`, and `0.960` for
offsets 0, 0.5, 1, 2, 4, and 8 seconds. The observed starts are 11.148 seconds
for the logo cue and 13.828 seconds for the theme. The 19 command requests
therefore do not produce 19 overlapping host players; the complete sound
decays before the title music begins.

The independent Genesis longplay in section 01 supports the same order: frame
000342 shows the partial logo, frame 000384 shows the completed logo, and frame
000420 shows the copyright row with the circuit visible. In its 44.1 kHz audio,
250 ms asset windows from resource 93 and resource 0 match at NCC `0.831` and
`0.852` respectively near their reveal and title transitions. These short
compressed-recording matches support cue identity/order but do not establish
exact VBlank or speaker latency.

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
   updates. The reveal cue is requested on every other column. The title theme
   is a continuous music track, not a per-column effect.
5. **What sound plays and how long?** The reveal requests resource 93, whose
   extracted render is 2.4503 seconds. The title text transition requests
   resource 0, whose extracted render is 120.0007 seconds. Qt process-loopback
   confirms both play in the ordinary title route. The source loop metadata is
   retained; applying music loop points outside the title route remains a
   separate audio-lifecycle question.
6. **Are other screen parts also correct?** The full QA frames show no new
   menu or intro controls placed over an old panel. The circuit, title text,
   prompt, options menu, and Seattle outline were checked against their state
   or reference frames; unrelated open defects remain tracked separately.
7. **Do multiple frames support the interpretation?** Yes. Native full-screen
   captures cover the reveal, palette, copyright, prompt, menu, and opening
   transitions. Independent section 01 frames 000342/000384/000420 support the
   logo-to-title visual sequence, and its audio supplies short matching windows
   for both resources.
8. **Were old buttons or graphics left under new UI?** The audio transition
   draws no controls. The complete title and Start Options captures show clean
   transitions; the return-from-empty-load route clears the old menu text before
   redrawing the Start Options choices.
9. **Was the full affected screenshot set reviewed?** Yes. All 134 startup QA
   screenshots were reviewed in chronological contact sheets. Debug and
   Release captures are byte-identical; individual title transition frames
   were also inspected at 320x224.

## Disposition

- **Code changed:** title audio request publication, event-to-resource mapping,
  Qt playback connection, and regression checks.
- **Behavior implemented:** command `0x5D` during the logo reveal and command
  `0x00` before the copyright row are connected to named extracted assets.
- **Independently checked:** the supplied longplay supports the visual/audio
  order; process-specific loopback confirms the normal Qt route plays each
  mapped asset.
- **Reported defect:** resolved for the missing/mistimed logo cue and title
  theme. The source loop metadata and audio lifetimes on other title/menu
  routes remain open.
