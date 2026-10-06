# Opening music start audit

This audit covers `OPEN-01`: the stage-1 opening music was heard only after
the opening had already advanced to its second scene. It does not close the
stage-2 music identity or the source lifetime of any opening track.

## ROM path and host behavior

At `0x59C72`, ROM loads `D0=0x10` and calls `0x530EA`; this happens before the
stage-1 schedule at `0x5AF62`. The audio gate is `FFFC08`. When enabled,
`0x530EA` reaches `0x53112`/`0x52F5A`, which queues the source driver command
bytes `FF 12 10`. Stage 2 similarly sends `FF 12 11` at `0x59C92`, before
`0x59D7C`; stage 3 sends `FF 12 12` at `0x59CB8`, before `0x5AA96`.

These source command arguments are not the standalone table-2 resource IDs
used by the offline extractor's `FF 10 <resource>` probe. That distinction
matters: an event number cannot be assigned to a resource by numeric equality
alone.

Before this correction, the Qt event table sent `0x10` to the 2.7505-second
`audio.field.effect-016` file and sent `0x11` to the 14.8337-second
`audio.intro.title.stage-1` file. The Qt callback forwards opening commands
directly to `AudioPlaybackService::play_event`, so the first request could not
start the stage-1 music.

The Qt table now maps `0x10` to `audio.intro.title.stage-1`. Command `0x11`
stays unmapped until its stage-2 sound is identified. Command `0x12` continues
to map to `audio.intro.title.stage-3`.

## Independent audio evidence

Reference: `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_02_archetype_opening_story_0056-0118/audio.wav`.
The extracted assets were compared with that independent recording using
normalized cross-correlation after mono mixing and resampling to 11.025 kHz:

| Asset | Measured duration | Strong matches in section 02 audio |
|---|---:|---|
| `title-stage-1.flac` | 14.8337 s | 0.4487 at 2.431 s; 0.4487 at 2.737 s |
| `title-stage-3.flac` | 21.0048 s | 0.4977 at 34.844 s |
| `effect-016.flac` | 2.7505 s | best score 0.0682 at 24.292 s |

This supports the stage-1 and stage-3 asset identities and start order. It is
an inference from the independent audio plus the ROM's ordered stage calls;
the Z80 resource-selection table for stage command `0x11` is still unresolved.
The 30-fps capture does not establish exact VBlank alignment.

## Verification

- `shadowrun_ghidra_startup_integration` records opening requests and checks
  that `0x10` is first, followed in order by stage commands `0x11` and `0x12`.
  Debug and Release pass.
- `shadowrun_ghidra_audio_playback` checks that events `0x10` and `0x12`
  decode through Qt Multimedia, expose audio, enter `PlayingState` on the
  default Windows output, report the expected file duration, and stop on
  request. Debug and Release pass. Debug measured 21 ms start latency and
  14,833 ms duration for `0x10`, and 23 ms / 21,004 ms for `0x12`. The test
  does not record Windows output loopback.
- `shadowrun_ghidra_conversion_qt_startup_qa` passes in Debug and Release and
  writes 126 full-screen `320x224` BMPs per configuration. All 126 names,
  dimensions, and SHA-256 hashes match across configurations. I reviewed every
  whole-screen image in both configurations; no visual code changed in this
  audio slice and no new control, stale layer, or redraw overlap appeared.
- Neither intro track has a row in `assets/audio/loop-points.tsv`. Qt plays
  the named files through their measured duration, but the corresponding ROM
  stop/loop condition has not been established. Do not treat host file-end
  playback as proof of the full source music lifetime.

## Eight-question review

1. **Does it look correct?** The change is audio-only. All 126 full-screen
   frames were reviewed at `320x224`. The independent section 02 frames show
   the Seattle text sequence (000120/000150/000210/000300), wilderness
   sequence (000542/000570/000638/000696), tri-vid (000820/000840/000900/
   000940/000960), and airport scene (001014/001065/001080/001320/001410).
   This preserves the audited visuals but does not claim that every unrelated
   intro visual defect is resolved.
2. **Does the visible look match the behavioral state?** Yes for the changed
   audio state: command `0x10` is submitted before the stage-1 entry and now
   resolves to the stage-1 track. Source-order QA checks the later `0x11` and
   `0x12` requests; `0x11` has no assigned asset until identified.
3. **Are the visuals themselves correct?** No image, palette, placement, or
   renderer code changed. The Debug/Release full-frame outputs are byte-equal.
   Independent visual fidelity remains governed by the linked Seattle,
   wilderness, tri-vid, and airport audits.
4. **Is it animated, and should it be?** The scene's existing animations
   continue unchanged. The intro audio files have no declared loop points;
   their exact source repeat, stop, and stage-handoff conditions remain
   unknown.
5. **Does it have sound, and how long does it play?** Qt starts event `0x10`
   on the default output with a 14,833 ms decoded duration and starts event
   `0x12` with a 21,004 ms duration. Event `0x11` remains silent in the host
   until its asset is identified. QMediaPlayer status is playback evidence;
   output loopback and full ROM lifetime are not measured.
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
  leaves unidentified command `0x11` unmapped.
- **Behavior implemented:** stage-1 music is requested at stage-1 entry;
  source stage-command order remains `0x10`, `0x11`, `0x12`.
- **Internally checked:** Debug/Release source-order CTest and default-output
  FLAC playback CTest pass; all 126 full-screen startup QA images were
  reviewed.
- **Independently checked:** section 02 audio correlations support the
  stage-1 and stage-3 track identities and timing.
- **Reported defect resolved:** the stage-1 music's late trigger is corrected.
  Keep `OPEN-01` partial until command `0x11` and source stop/loop lifetimes
  are identified.
