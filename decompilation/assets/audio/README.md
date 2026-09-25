# Extracted Windows audio

These are rendered PCM WAV files for the native Windows port. They are not
raw Z80 fragments, split event records, or screen-capture audio. Every file
is 44.1 kHz, 16-bit, stereo PCM and can be played directly by Windows and Qt.
Finite resources with a verified end are exported through their complete
source event lifetime; persistent or ambiguous streams have an explicit
observation-capture policy below.

The offline extractor boots the ROM's uploaded Z80 sound driver, implements
the Genesis ROM bank window, records YM2612 and SN76489 writes as timed VGM
commands, and renders those commands through a YM2612-capable VGM renderer.
The native game does not run the Z80 driver or consume VGM files.

The table-2 resource table contains 94 indexed slots. The extraction pass
tested all 94 with the source's standalone reset/play command sequence. 65
slots produced non-silent chip output and are present here; 29 slots were
verified no-op/silent entries and are intentionally absent rather than being
added as empty runtime assets. `resources.tsv` is the authoritative alias
list, for example `assets.audio_path("audio.intro.title.stage-1")`.

The source command IDs that have been identified in the ROM are kept in the
aliases and directory names where the evidence is specific: title intro
stages, menu commands, field effects, and world effects. Other entries remain
under `sound-effects/resource-NNN.wav` until a calling screen gives them a
more specific semantic name.

Offline sources:

- `../input/shadowrun_genesis_us.bin`
- `../tools/audio/extract_genesis_audio.py`
- `../tools/audio/trim_wav.py`

The intermediate VGM files belong in a temporary extraction directory and
must not be copied into this runtime asset tree.

`loop-points.tsv` records the exact exclusive PCM loop endpoints for resources
whose source stream has two successive restarts. The first part of those WAVs
is an intro; playback starts at frame zero and repeats only the recorded
`loop_start_frame..loop_end_frame` interval. Do not loop the entire file or
reconstruct these points from rounded seconds. These are source-driver timing
boundaries, not a claim that a newly reset software synthesizer has identical
oscillator/envelope state at both ends. A host player must preserve the source
continuity or use a seam-safe loop/crossfade policy; it must not reset a
different synthesized state at the loop start and call that seamless.

## Duration policy

The extractor's trace window is only an observation window; it is never used
as the WAV length. Resources with verified repeatable source boundaries are
traced through two successive source-driver stream restarts; their WAV retains
the intro and `loop-points.tsv` identifies the exact repeat interval. One-shot
resources are exported through their final traced chip event plus a two-second
release tail.
The renderer is given extra time and the resulting PCM is then cropped to the
measured source boundary, avoiding the renderer's coarse duration quantizing
from cutting a loop short.

Resources 012, 013, and 048 continue producing source chip events without a
standalone stop or repeat boundary during a ten-minute extraction probe. They
are kept as ten-minute persistent streams rather than being replaced by short
previews; their eventual native playback should use the source command's
explicit lifecycle.

Resources 005, 014, 015, and 040 also have internal or per-channel restarts,
but the present trace does not prove a synchronized whole-track boundary.
They are therefore retained as 120-second observation captures with no
`loop-points.tsv` entry. Native playback must stop them by gameplay lifetime
until their complete source lifecycle is traced; it must not infer a loop from
the first restart seen in one channel.
