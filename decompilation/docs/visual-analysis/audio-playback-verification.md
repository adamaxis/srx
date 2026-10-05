# Qt audio effect playback verification

## Scope

This review covers the shared one-shot audio service for source events `0x24`,
`0x25`, and `0x4B`. It does not cover background music, persistent event
`0x30`, or every sound call in the ROM.

## ROM and asset evidence

- The converted menu and field-service paths submit event IDs to the audio
  command service at ROM `0x530DE`.
- Event `0x25` selects table-2 resource 37, named
  `audio.effects.resource.037`; its extracted FLAC is 44.1 kHz, stereo, and
  2.134 seconds including the release tail.
- Event `0x24` maps to `audio.menu.command-024`; its extracted FLAC is 2.484
  seconds including the release tail.
- The electromagnetic-lock unlock path submits event `0x4B`, mapped to
  resource 75; its extracted FLAC contains 97,770 samples at 44.1 kHz, or
  2.2167 seconds.
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

`AudioPlaybackService` now uses four `QMediaPlayer`/`QAudioOutput` channels.
The existing FLAC files remain individually named runtime assets. Event
selection, volume (`0.6`), and four-voice overlap capacity are retained. The
service exposes a small playback status for QA, including decoded audio
availability, play state, duration, and errors. Unknown events remain
unmapped.

## Internal playback check

`shadowrun_ghidra_audio_playback` resolves the real files, plays events
`0x24`, `0x25`, and `0x4B` through the default Windows output device, waits for
decoded audio and `PlayingState`, checks the decoded duration against each
source file, then stops the active channels. The test skips with CTest's
`SKIP_RETURN_CODE` only if Windows reports no default audio output. Debug and
Release both passed on the attached Jabra Evolve2 30 SE device. Measured player
start latencies were 19 ms, 31 ms, and 23 ms, respectively; reported durations
were 2,483 ms, 2,133 ms, and 2,217 ms. These measurements confirm that the Qt
decoder and output route ran. They are not a Windows loopback recording.

The shared scene paths also passed Debug and Release Qt QA:

- `shadowrun_ghidra_conversion_qt_field_interaction_qa` exercises the
  interaction route carrying event `0x4B`.
- `shadowrun_ghidra_conversion_qt_field_player_action_qa` checks the pair of
  source event `0x4B` submissions and the before/waiting/reload field frames.
- `shadowrun_ghidra_conversion_qt_field_shop_qa` exercises the Ares shop and
  emits full-frame images under `qt_runtime/field-shop-ares-qa/`.
- `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` exercises a menu path
  carrying event `0x24`.

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
   no loop is requested. Music-loop behavior remains open.
5. **Does it have sound, and how long does it play?** Yes for events `0x24`,
   `0x25`, and `0x4B`; the files, output route, start state, duration, and stop
   behavior are checked above. The independent capture matches resource 37
   repeatedly. Windows output loopback remains unrecorded.
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

The current service verifies three mapped one-shot events. Music command
playback and source loop points, more source-event mappings, event `0x30` stop
lifetime, and a Windows output-loopback capture remain open. Therefore the
audio hill remains partial.
