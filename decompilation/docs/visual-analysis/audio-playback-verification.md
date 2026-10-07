# Qt audio effect playback verification

## Scope

This review covers the shared one-shot audio service for source events `0x16`,
`0x1E`, `0x24`, `0x25`, `0x4A`, `0x4B`, and `0x5C`. It does not cover background music,
persistent event `0x30`, or every sound call in the ROM.

## ROM and asset evidence

- The converted menu and field-service paths submit event IDs to the audio
  command service at ROM `0x530DE`.
- Event `0x25` selects table-2 resource 37, named
  `audio.effects.resource.037`; its extracted FLAC is 44.1 kHz, stereo, and
  2.134 seconds including the release tail.
- Event `0x24` maps to `audio.menu.command-024`; its extracted FLAC is 2.484
  seconds including the release tail.
- A general `FF 10 16` start selects table-2 resource 22, extracted as
  `audio.field.effect-016` (2.7505 seconds). The pre-title caller sends the
  same ID through `FF 12 16`, which stops resource 22; the Qt callback keeps
  those operations separate by service path.
- The pre-title shock caller sends `FF 10 5C`, selecting table-2 resource 92
  (`audio.effects.resource.092`, 2.750 seconds). Its source repeats the start
  during the lightning window; the ROM-driver trace shows six distinct FM
  voices active before any is reused. Qt now overlaps the starts in a six-voice
  pool. See the [boot shock audio audit](../visual-audits/boot-shock-audio.md)
  for the source trace and independent audio/output checks.
- The electromagnetic-lock unlock path submits event `0x4B`, mapped to
  resource 75; its extracted FLAC contains 97,770 samples at 44.1 kHz, or
  2.2167 seconds.
- The field spawn cycle at ROM `0x1514E` selects an event from tables
  `0x15318`/`0x15336` and submits it at `0x15238`. For action codes 8..10,
  ordinary state sends event `0x1E` / resource 30 (2.284 seconds) and
  alternate state sends event `0x4A` / resource 74 (2.267 seconds). These
  durations come from their decoded FLAC sample counts; no independent
  longplay shot has yet confirmed either sound in context.
- The empty-ammo player action submits `0x4B` after the selected-action
  behavior gate and held-A input gate: ROM `0x14226` dispatches
  it before the delay/clip/meter update, then the successful `0xAAD2` return
  reaches the `0xACAE` tail, which dispatches `0x4B` again and resets the
  normal delay. Qt QA checks both event IDs and order. It does not capture
  Windows output loopback, so the audible result of the two rapid submissions
  is not independently established.
- Silent resource 44 remains omitted. Event `0x30` remains unmapped because
  resource 48 is a persistent observation capture with no traced source stop.

## Defect found and code changed

The Qt service passed these `.flac` paths to `QSoundEffect`. On the installed
Qt 6.9.1 Windows runtime, `QSoundEffect::supportedMimeTypes()` listed WAV MIME
types, and loading `menu-024.flac` changed its status from `Loading` to
`Error`. The existing event-to-alias test passed while this playback path was
silent. Qt's [QSoundEffect documentation](https://doc.qt.io/qt-6/qsoundeffect.html)
describes it as a player for uncompressed files, typically WAV.

`AudioPlaybackService` now uses six `QMediaPlayer`/`QAudioOutput` channels.
The existing FLAC files remain individually named runtime assets. Event
selection and volume (`0.6`) are retained. The six-voice pool follows the
distinct YM2612 voices seen during the repeated shock requests. The
service exposes a small playback status for QA, including decoded audio
availability, play state, duration, and errors. Unknown events remain
unmapped.

## Internal playback check

`shadowrun_ghidra_audio_playback` resolves the real files, plays events
`0x10`, `0x12`, `0x16`, `0x1E`, `0x24`, `0x25`, `0x4A`, `0x4B`, and `0x5C`
through the default Windows output device, waits for
decoded audio and `PlayingState`, checks the decoded duration against each
source file, then stops the active channels. The test skips with CTest's
`SKIP_RETURN_CODE` only if Windows reports no default audio output. Debug and
Release pass on the current `Speakers (Realtek(R) Audio)` output. For `0x5C`,
the test adds 59 starts at 33 ms intervals, checks that six channels remain
playing, and verifies resource-stop behavior. Each test run reports resource
92 at 2,750 ms with roughly 20-30 ms initial start latency.

A separate Windows WASAPI loopback recording captured the Release test output
at 44.1 kHz stereo. During the repeated `0x5C` burst, the capture has a 0.370
normalized correlation with the first 80 ms of resource 92 at 12.271 s, with
additional matches through 13.631 s. The recording is
`build/visual-review/boot-shock-release-loopback-aligned.wav` (ignored build
output). This verifies that the effect reached the Windows output mix; it does
not set Genesis VBlank timing.

The ordinary Release game executable was also launched with the extracted
asset directory and captured through the same default endpoint. Its loopback
contains repeated 0.40-level resource-92 matches across the startup burst
(1.971-3.571 s), then returns to the pre-effect output level after 4.3 s. The
10-second WAV is `build/visual-review/boot-shock-release-game-loopback.wav`.

The shared scene paths also passed Debug and Release Qt QA:

- `shadowrun_ghidra_conversion_qt_field_interaction_qa` exercises the
  interaction route carrying event `0x4B`.
- `shadowrun_ghidra_conversion_qt_field_player_action_qa` checks the pair of
  source event `0x4B` submissions and the before/waiting/reload field frames.
- `shadowrun_ghidra_conversion_qt_field_shop_qa` exercises the Ares shop and
  emits full-frame images under `qt_runtime/field-shop-ares-qa/`.
- `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` exercises a menu path
  carrying event `0x24`.
- The audio playback test separately resolves and decodes the combat spawn
  assets for events `0x1E` and `0x4A`; it does not establish that live Qt
  combat reaches the spawn service.

The shop frame inspected at
`field-shop-ares-qa/shop-item/actual.bmp` remains a clean 320x224 page. The
audio service does not paint controls or change screen composition.

## Independent audio comparison

I compared extracted assets with audio from the independent
`video analysis/long_gameplay_video.mp4` Ares sequence at 5,980–6,000 seconds.
Repeated short waveform matches identify resource 37: the strongest normalized
correlations were 0.5195 at 5,987.022 seconds, 0.5149 at 5,993.456 seconds, and
0.5121 at 5,985.289 seconds. The command sound also has repeated matches to
`menu-024.flac`, with correlations 0.8052 at 5,992.711 seconds and 0.7966 at
5,986.477 seconds. These matches support the sound identity in the recording;
they do not establish exact emulator frame timing.

## Eight-question review

1. **Does it look correct?** This is a shared sound mechanism and adds no
   visible controls. The Ares shop QA frame was inspected for the active
   layout.
2. **Does the visible look match behavioral state?** Yes for the playback
   indicator used by QA: each tested event resolves to its source-supported
   alias, decodes audio, and reaches `PlayingState`.
3. **Are the visuals correct?** The existing full-frame shop capture remains
   intact; no renderer or asset-placement code changed.
4. **Should anything animate?** The audio effects are one-shots. Each player
   plays through the extracted release tail and reaches the end of its file;
   no loop is requested. The intro stage tracks are one-shots in the
   90-second ROM trace; loops for other background music remain open.
5. **Does it have sound, and how long does it play?** The files, output route,
   start state, duration, and stop behavior are checked for the events listed
   above. Spawn events `0x1E`/`0x4A` decode to 2.284/2.267-second one-shots;
   their independent gameplay audio match is still missing. The boot path's
   `0x16` is specifically a stop operation, while other
   callers can start resource 22 with `FF 10 16`. The independent longplay
   repeatedly matches resource 37 and also supports resource 92 at 4.225 s.
   Endpoint loopback captured the boot-shock resource on the current
   default device. Process-specific loopback on the normal Release opening
   route captures stage 1 at opening entry, silence through stage 2, and stage
   3 at its entry; other event-specific routes remain open.
6. **Are related screen parts also correct?** The Ares page, text, item art,
   cursor, and right-side HUD remain visible in the 320x224 QA frame.
7. **Do multiple frames support the interpretation?** The independent audio
   comparison found repeated matches at distinct longplay times. The shop QA
   also writes multiple UI states, though those are internal conversion
   captures, not independent Genesis frames.
8. **Was old UI cleared before new controls were drawn?** This change adds no
   controls. Existing shop page captures retain the page's full-screen
   composition without an audio-driven overlay.

## Remaining work

The current service verifies the mapped one-shot events above and two
identified intro music assets. In the current Debug/Release runs, resource 22
(`FF 10 16`) reported 17/31 ms startup and 2,750 ms duration; resource 92
(`FF 10 5C`) reported approximately 20-30 ms and 2,750 ms. The boot callback
uses the separate `FF 12` resource-stop operation for `0x16`; its repeated
`0x5C` path now has source-aligned overlapping voices and recorded Windows
loopback. The exact paired stage-2 request
`FF 12 11`, `FF 10 11` produces setup/quieting writes but no YM key-on; the
host intentionally leaves it unmapped. Source `0x53126` sends `FF 16` at
stage boundaries, and Qt now stops all playback channels there. The intro
tracks show no source restart in 90-second ROM traces. More source-event
mappings, event `0x30` stop lifetime, other music loop points, and
process/endpoint loopback for the other mapped routes remain open. The opening
route is verified as recorded in [the OPEN-01 audit](../visual-audits/opening-music-start.md).
Therefore the general audio hill remains partial.
