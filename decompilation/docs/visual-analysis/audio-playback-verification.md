# Qt audio effect playback verification

## Scope

This review covers the shared audio service for source events `0x16`, `0x1E`,
`0x24`, `0x25`, `0x32`, `0x34`, `0x35`, `0x37`, `0x3A`, `0x4A`, `0x4B`, `0x5C`,
and `0x5D`. Events `0x5C` and `0x5D`
use ROM-cadenced composite sequence assets rather than the standalone
one-start resource files. This does not cover every ROM sound call or the full
lifecycle of background music and persistent event `0x30`.

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
- The pre-title shock caller sends `FF 10 5C`, selecting table-2 resource 92.
  The standalone resource-92 FLAC is one start, not the full 60-start shock
  sequence. The corrected `audio.intro.sega-logo-shock-sequence` asset replays
  all starts at two-frame intervals through the ROM Z80 driver and Genesis chip
  render. Qt starts that composite once per pre-title sequence. See the
  [boot shock audio
  audit](../visual-audits/boot-shock-audio.md) for source, independent audio,
  and loopback checks.
- The title reveal submits `FF 10 5D` 19 times at four-frame intervals. The
  corrected `audio.intro.shadowrun-logo-reveal-sequence` asset contains those
  repeated starts and stops at the ROM theme command boundary (2.700 seconds
  from the first reveal request).
- The electromagnetic-lock unlock path submits event `0x4B`, mapped to
  resource 75; its extracted FLAC contains 97,770 samples at 44.1 kHz, or
  2.2167 seconds.
- The field spawn cycle at ROM `0x1514E` selects an event from tables
  `0x15318`/`0x15336` and submits it at `0x15238`. For action codes 8..10,
  ordinary state sends event `0x1E` / resource 30 (2.284 seconds) and
  alternate state sends event `0x4A` / resource 74 (2.267 seconds). These
  durations come from their decoded FLAC sample counts; no independent
  longplay shot has yet confirmed either sound in context.
- Spell effects reach the `0x4946` event table once after `0x15830` reserves
  an effect slot. Its encounter action IDs are hexadecimal, while the named
  table-2 resource filenames are decimal: `0x32` -> resource 50 (2.467 s),
  `0x34` -> 52 (2.517 s), `0x37` -> 55 (2.820 s), `0x35` -> 53
  (2.434 s), and `0x3A` -> 58 (2.703 s). The final route is Heal Wounds.
  These lengths are the extracted FLAC sample counts at 44.1 kHz.
  Debug/Release audio QA checks the event-to-resource routes and live Qt
  decoding/playback. The supplied longplay has not yet provided a confirmed
  same-action spell sound, so these are host resource lengths, not a Genesis
  hardware lifetime measurement.
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

`AudioPlaybackService` uses six `QMediaPlayer`/`QAudioOutput` channels for
unrelated concurrent effects. The existing FLAC files remain individually
named runtime assets. Event selection and volume (`0.6`) are retained. The
`0x5C` and `0x5D` source sequences use one host player because their mapped
files contain the complete source-timed repeated chip writes; their individual
request counts remain available in the source trace. The service exposes
playback status for QA, including decoded audio availability, play state,
duration, and errors. Unknown events remain unmapped.

## Internal playback check

`shadowrun_ghidra_audio_playback` resolves the real files, plays events
`0x10`, `0x12`, `0x16`, `0x1E`, `0x24`, `0x25`, `0x4A`, `0x4B`, `0x5C`, and `0x5D`
through the default Windows output device, waits for
decoded audio and `PlayingState`, checks the decoded duration against each
source file, then stops the active channels. The test skips with CTest's
`SKIP_RETURN_CODE` only if Windows reports no default audio output. The test
also feeds 60 shock and 19 reveal callbacks and verifies they do not restart
the composite files for every request. Debug/Release results for the corrected
assets are recorded in their focused audits. These checks establish decoding
and the connected Qt route, not a Genesis-hardware waveform match.

The former six-player WASAPI captures are retained in ignored build output but
do not verify the current boot-shock mix. The one-shot correction is checked in
the section below.

The shared scene paths also passed Debug and Release Qt QA:

- `shadowrun_ghidra_conversion_qt_field_interaction_qa` exercises the
  interaction route carrying event `0x4B`.
- `shadowrun_ghidra_conversion_qt_field_player_action_qa` checks the pair of
  source event `0x4B` submissions and the before/waiting/reload field frames.
- `shadowrun_ghidra_conversion_qt_field_shop_qa` exercises the Ares shop and
  emits full-frame images under `qt_runtime/field-shop-ares-qa/`.
- `shadowrun_ghidra_conversion_qt_field_magic_spell_qa` exercises a menu path
  carrying event `0x24`.
- `shadowrun_ghidra_conversion_qt_field_spell_effect_qa` exercises all five
  source spell profiles in its field renderer capture; the sound route is
  separately decoded by the audio playback test.
- The audio playback test separately resolves and decodes the combat spawn
  assets for events `0x1E` and `0x4A` and spell assets for events `0x32`,
  `0x34`, `0x35`, `0x37`, and `0x3A`; it does not establish that an independent
  Genesis cast reaches the spell service.

The shop frame inspected at
`field-shop-ares-qa/shop-item/actual.bmp` remains a clean 320x224 page. The
audio service does not paint controls or change screen composition.

## Intro sequence audio corrections

The earlier one-shot mappings played standalone first-start renders and
discarded later source starts, so their internal checks and short longplay
correlations could not establish the complete cues. The offline extractor now
queues all ROM-timed starts through the Z80 driver, renders the resulting
YM2612/SN76489 writes, and creates one composite runtime asset for each intro
sequence. Debug/Release route checks and fresh process loopback results are
recorded in the [shock audit](../visual-audits/boot-shock-audio.md) and [title
reveal audit](../visual-audits/title-logo-audio.md). Independent 50 ms RMS
envelopes match the full shock and reveal intervals at Pearson 0.983 and 0.980;
raw chip sample phase is not claimed from the compressed longplay.

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
   above. Spell events `0x32`/`0x34`/`0x35`/`0x37`/`0x3A` decode to
   2.467/2.517/2.434/2.820/2.703-second one-shots; no matching independent
   gameplay audio was identified. Spawn events `0x1E`/`0x4A` decode to
   2.284/2.267-second one-
   shots; their independent gameplay audio match is still missing. The boot path's
   `0x16` is a stop operation, while other callers can start resource 22 with
   `FF 10 16`. Release loopback matches the 60-start shock and 19-start reveal
   assets; independent 50 ms RMS envelopes match at Pearson 0.983 and 0.980
   across the full active intervals. The opening-music route is captured in the
   OPEN-01 audit.
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

The current service verifies the mapped one-shot events above and the two
intro sequence assets. The boot callback uses the separate `FF 12` stop
operation for `0x16`; its `0x5C` path represents all 60 starts in a single
source-cadenced render. The title reveal path represents all 19 `0x5D` starts
until the theme handoff. Release process-loopback matches the assets at PCM NCC
0.9998/0.9885 and envelope Pearson 1.000/1.000; independent longplay envelopes
match at Pearson 0.983/0.980 over the active intervals. This verifies the
repeated cue shapes and durations without claiming exact chip sample phase.

The exact paired stage-2 request `FF 12 11`, `FF 10 11` produces
setup/quieting writes but no YM key-on; the host intentionally leaves it
unmapped. Source `0x53126` sends `FF 16` at stage boundaries, and Qt stops all
playback channels there. The intro tracks show no source restart in 90-second
ROM traces. Other source-event mappings, event `0x30` stop lifetime, other
music loop points, and process/endpoint loopback for other routes remain open.
The opening route is verified in [the OPEN-01 audit](../visual-audits/opening-music-start.md).
Therefore the general audio hill remains partial.
