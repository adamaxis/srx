# Opening music start audit

This audit covers `OPEN-01`: the stage-1 opening music was heard only after
the opening had already advanced to its second scene. It resolves the stage-2
sound identity as silent for the source command pair and models the source
track reset at stage transitions. Independent confirmation of stage-2 silence
and any within-stage loop behavior remain open.

## ROM path and host behavior

At `0x59C72`, ROM loads `D0=0x10` and calls `0x530EA` before the stage-1
schedule at `0x5AF62`. The gate is `FFFC08`. When enabled, `0x530EA` calls
`0x53112`/`0x52F5A` to queue `FF 12 <D0>`, then continues through
`0x530FE`/`0x52F3C` to queue `FF 10 <D0>`. Thus the actual stage-1 request is
the pair `FF 12 10`, `FF 10 10`. Stage 2 sends the same pair with ID `0x11`
at `0x59C92`, before `0x59D7C`; stage 3 sends it with ID `0x12` at
`0x59CB8`, before `0x5AA96`. The ROM bytes at `0x530EA` confirm the wrapper
call followed by the queue-service path; the exported disassembly is only a
cross-check.

Before stage 2 at `0x59C8C` and stage 3 at `0x59CB2`, the caller invokes
`0x53126`. The ROM performs six `FF 20 14` transactions, each separated by
three VBlank waits, then `0x52FA0` submits `FF 16` and the tick wrapper submits
`FF 20 00`. In the uploaded Z80 driver, command `0x16` clears the active state
of all sixteen `0x20`-byte channel records and runs the channel/voice reset
path at driver offsets `0x0CF9`/`0x0D42`. The final opening-to-field
mode-entry also calls the same reset service. Qt now stops all host playback
channels at these `prepare_mode_entry` boundaries before issuing the next
stage command.

The command IDs select the corresponding table-2 entries only in the complete
source sequence. A standalone `FF 10 <id>` probe is not equivalent: for
example, standalone ID `0x10` produces the short `effect-016` sound, while the
source pair produces the stage-1 track. The runtime aliases therefore come
from full paired-sequence extraction and independent capture evidence, not
from numeric ID equality alone.

Before this correction, the Qt event table sent `0x10` to the 2.7505-second
`audio.field.effect-016` file and sent `0x11` to the 14.8337-second
`audio.intro.title.stage-1` file. The Qt callback forwards opening commands
directly to `AudioPlaybackService::play_event`, so the first request could not
start the stage-1 music.

The Qt table maps `0x10` to `audio.intro.title.stage-1` and `0x12` to
`audio.intro.title.stage-3`. Command `0x11` intentionally has no asset: the
ROM-backed trace of the exact `FF 12 11`, `FF 10 11` pair contains setup and
quieting writes but no YM key-on or PSG tone sequence.

### ROM-backed stage probe

The offline extractor was run against the US ROM for all three stage IDs with
the exact source pair:

```text
python decompilation/tools/audio/extract_genesis_audio.py \
  decompilation/input/shadowrun_genesis_us.bin \
  build/audio-open-stage-probe/paired-reset-play \
  --ids 16,17,18 --sequence 0x12,0x10 --seconds 40
```

The resulting VGM traces contain:

| Stage ID | YM writes | YM key-ons | YM frequency writes (A0-A2/A4-A6) | PSG writes |
|---:|---:|---:|---:|---:|
| `0x10` | 898 | 6 | 652 | 8 |
| `0x11` | 35 | 0 | 0 | 8 |
| `0x12` | 6,396 | 146 | 1,550 | 8 |

The eight PSG writes for each trace are the same maximum-attenuation setup
values. Stage ID `0x11` adds no audible note sequence, unlike `0x10` and
`0x12`; leaving it unmapped is the source-supported result. This does not
replace the global `FF 16` stage-boundary reset. The stop boundary is now
implemented in Qt; whether either track loops before that boundary remains
open.

## Independent audio evidence

Reference: `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_02_archetype_opening_story_0056-0118/audio.wav`.
The extracted assets were compared with that independent recording using
normalized cross-correlation after mono mixing and resampling to 11.025 kHz:

| Asset | Measured duration | Strong matches in section 02 audio |
|---|---:|---|
| `title-stage-1.flac` | 14.8337 s | 0.4487 at 2.431 s; 0.4487 at 2.737 s |
| `title-stage-3.flac` | 21.0048 s | 0.4977 at 34.844 s |
| `effect-016.flac` | 2.7505 s | best score 0.0682 at 24.292 s |

This supports the stage-1 and stage-3 asset identities and start order. The
stage-2 silence conclusion comes from the ROM-driven exact-pair trace; the
capture's sparse audio correlation is not used as proof of driver behavior.
The 30-fps capture does not establish exact VBlank alignment or independently
confirm the stage-boundary stop.

## Verification

- `shadowrun_ghidra_startup_integration` records opening requests and checks
  that `0x10` is first, followed in order by stage commands `0x11` and `0x12`.
  Debug and Release pass.
- `shadowrun_ghidra_audio_playback` checks that events `0x10` and `0x12`
  decode through Qt Multimedia, expose audio, enter `PlayingState` on the
  default Windows output, report the expected file duration, and stop on
  request. It also starts `0x10`, applies the source `FF 16` reset, and checks
  that `0x11` stays unmapped and the prior channel is no longer playing.
  Debug and Release pass. Debug measured 21 ms start latency and 14,833 ms
  duration for `0x10`, and 23 ms / 21,004 ms for `0x12`. The test does not
  record Windows output loopback.
- `shadowrun_ghidra_conversion_qt_startup_qa` passes in Debug and Release and
  writes 118 full-screen `320x224` BMPs per configuration. All 118 names,
  dimensions, and SHA-256 hashes match across configurations. I reviewed every
  whole-screen image in both configurations; no visual code changed in this
  audio slice and no new control, stale layer, or redraw overlap appeared.
- Neither intro track has a row in `assets/audio/loop-points.tsv`. The source
  clears playback channels at stage boundaries; whether either track loops
  within its stage remains unestablished. Do not treat host file-end playback
  as proof of the complete source lifetime.

## Eight-question review

1. **Does it look correct?** The change is audio-only. All 118 full-screen
   frames were reviewed at `320x224`. The independent section 02 frames show
   the Seattle text sequence (000120/000150/000210/000300), wilderness
   sequence (000542/000570/000638/000696), tri-vid (000820/000840/000900/
   000940/000960), and airport scene (001014/001065/001080/001320/001410).
   This preserves the audited visuals but does not claim that every unrelated
   intro visual defect is resolved.
2. **Does the visible look match the behavioral state?** Yes for the changed
   audio state: command `0x10` is submitted before the stage-1 entry and now
   resolves to the stage-1 track. Source-order QA checks the later `0x11` and
   `0x12` requests; the exact source-pair probe shows that `0x11` has no
   audible asset to assign.
3. **Are the visuals themselves correct?** No image, palette, placement, or
   renderer code changed. The Debug/Release full-frame outputs are byte-equal.
   Independent visual fidelity remains governed by the linked Seattle,
   wilderness, tri-vid, and airport audits.
4. **Is it animated, and should it be?** The scene's existing animations
   continue unchanged. The source resets audio channels at stage entry/handoff.
   The intro audio files have no declared loop points, so within-stage repeat
   behavior remains unknown.
5. **Does it have sound, and how long does it play?** Qt starts event `0x10`
   on the default output with a 14,833 ms decoded duration and starts event
   `0x12` with a 21,004 ms duration. The ROM-driven `0x11` pair has no YM
   key-on or PSG tone sequence, so the host correctly leaves it silent. The
   source `FF 16` reset stops the preceding host track at the stage handoff.
   QMediaPlayer status is playback evidence; output loopback and within-stage
   loop behavior are not measured.
6. **Are related parts of the screen also correct?** The full dump includes
   boot/title, Start Options, archetypes, Seattle, wilderness, tri-vid,
   airport, post-intro cab map, and field/menu states. The audio-only diff
   introduces no overlays or changes to these visible parts.
7. **Do multiple frames support the interpretation?** Yes. The continuous
   independent recording has separate stage-1, wilderness, stage-3, and
   airport checkpoints. Its audio has repeated stage-1 matches at 2.431/2.737
   s and a separate stage-3 match at 34.844 s. Timing is not VBlank-exact.
8. **Does new UI replace old UI cleanly?** No UI was added. The reviewed
   full-frame transition captures show no new old-page controls or graphics
   left under a newly drawn page.

## Outcome

- **Code changed:** Qt maps opening command `0x10` to the stage-1 track and
  deliberately leaves source-silent command `0x11` unmapped.
- **Behavior implemented:** stage-1 music is requested at stage-1 entry;
  source stage-command order remains `0x10`, `0x11`, `0x12`; the host stops
  active playback at source `FF 16` mode-entry resets.
- **Internally checked:** Debug/Release source-order CTest and default-output
  FLAC playback CTest pass; all 118 full-screen startup QA images were
  reviewed.
- **Independently checked:** section 02 audio correlations support the
  stage-1 and stage-3 track identities and timing. The silent stage-2 result
  is supported by a ROM-driven Z80 trace of the exact source command pair.
- **Reported defect resolved:** the stage-1 music's late trigger is corrected,
  the exact stage-2 source pair has no audible note sequence, and stage
  transitions stop active host playback at the source reset. Keep `OPEN-01`
  partial until stage-2 silence is independently confirmed, within-stage loop
  behavior is established, and the relevant output route is captured.
