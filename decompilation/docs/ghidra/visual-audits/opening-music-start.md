# Opening music audit

The opening music now maps all three ROM stage requests to their extracted
tracks. The earlier runtime omitted page 2 (`0x11`), so it went silent between
the Seattle and tri-vid scenes. The earlier audit mistook “no ordinary YM
key-on” for “no audio” and did not count the Timer-A-driven YM DAC stream.
That conclusion is superseded by the paired ROM trace below.

## ROM requests and transitions

At `0x59C72`, the opening requests stage ID `0x10` before the first scene.
Stage 2 requests `0x11` at `0x59C92`, and stage 3 requests `0x12` at
`0x59CB8`. Each goes through `0x530EA`, which queues `FF 12 <id>` followed by
`FF 10 <id>`. The stage boundaries call `0x53126`; its fade ends with `FF 16`
(channel reset) before the next page request. The host keeps the three tracks
in the music channel and follows those same boundaries.

The stage-2 sequence uses the YM DAC. The Z80 driver polls Timer A in its
routine at `0x02B9` and emits sample data through YM register `0x2A`, with
`0x2B` enabling the DAC. A paired replay of the US ROM using
`extract_genesis_audio.py --ids 17 --sequence 0x12,0x10 --seconds 40` recorded
75,548 writes to `0x2A` and 45 writes to `0x2B`, with no dropped VBlank
interrupts. It also recorded 244 channel key-ons; the earlier check that only
counted key-ons and PSG tones missed the much larger DAC stream.

The existing asset `audio/intro/title-stage-2.flac` is 44.1 kHz stereo,
16.480 seconds long, with nonzero PCM through the audible body of the track.
The manifest generator now maps `MusicId.INTRO_PAGE_2` to this file as music.
Stage 1 remains `0x10` -> `title-stage-1`; stage 3 remains `0x12` ->
`title-stage-3`.

## Playback checks

`audio_playback_test` checks that event `0x11` resolves to the stage-2 asset,
reports decoded audio, and starts a music voice. `audio_mixer_test` checks the
mapped row and nonzero PCM. In the Debug middle Start-skip route, the `0x11`
start occurs at mixer frame 1771 and the skip stops it at frame 2022; the first
44,100 stereo sample frames after start match `title-stage-2.flac` exactly.
The Debug/Release Start-skip QA requires the route to issue the `0x11` start by
the stage-2 and later checkpoints, and writes the event timeline and headless
WAV for review.

The earlier visible Release process-loopback capture verified stage 1 and
stage 3, but it was made while `0x11` was unmapped. Its silence during the
stage-2 interval was a port defect, not evidence that the ROM is silent. The
provided Genesis longplay has mixed/compressed audio; its stage-2 track match
is not strong enough to serve as independent waveform proof. Current source
behavior is established by the US-ROM Timer-A trace, and host routing is
checked headlessly through the production frame loop. Physical speaker-path
fidelity has not been tested.

## Result and remaining scope

The missing stage-2 music is fixed by mapping the existing extracted asset to
ROM command `0x11`. The source `FF 16` resets still cut tracks at page
boundaries. Exact independent audio alignment for stage 2 remains unverified;
intro sound effects beyond the mapped stage music remain open under
`AUDIO-RECHECK-02`.