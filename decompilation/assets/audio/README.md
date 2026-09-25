# Extracted Windows audio

These are rendered PCM WAV files for the native Windows port. They are not
raw Z80 fragments, split event records, or screen-capture audio. Every file
is 44.1 kHz, 16-bit, stereo PCM and can be played directly by Windows and Qt;
finite resources are exported through their complete source event lifetime.

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

## Duration policy

The extractor's trace window is only an observation window; it is never used
as the WAV length. Looped resources are traced through a source-driver stream
restart and exported through one complete source loop. One-shot resources are
exported through their final traced chip event plus a two-second release tail.
The renderer is given extra time and the resulting PCM is then cropped to the
measured source boundary, avoiding the renderer's coarse duration quantizing
from cutting a loop short.

Resources 012, 013, and 048 continue producing source chip events without a
standalone stop or repeat boundary during a ten-minute extraction probe. They
are kept as ten-minute persistent streams rather than being replaced by short
previews; their eventual native playback should use the source command's
explicit lifecycle.
