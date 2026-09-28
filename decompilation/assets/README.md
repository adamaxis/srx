# Direct Shadowrun assets

Generated from the US Genesis ROM by shadowrun_graphics_extract --all-assets.

The native runtime consumes these individually named BMPs and native/state.bin and resources.tsv directly. This tree deliberately contains semantic folders: menu, intro, field, and item categories. Static intro plates and independently extracted intro sprites remain separate; it does not contain full-screen flipbook samples or an aggregate serializer. Item art is grouped under items/weapons, items/first-aid, items/miscellaneous, items/magic, and items/armor. Separately traced cinematic resources belong under movies/<scene>; they must not be mixed into intro or field assets.

Audio is emitted here only as complete Windows-playable lossless FLAC files under
`audio/`. The offline Z80/VGM extraction sources and intermediate event data
remain outside this runtime asset tree. Verified repeating audio uses
`audio/loop-points.tsv` for exact PCM frame boundaries. Use `resources.tsv` for stable aliases,
such as `assets.audio_path("audio.intro.title.stage-1")`; do not add raw ROM
slices or split event fragments here. A missing loop-point entry is
intentional: the native player must not guess a repeat boundary from an
internal channel restart.

Field startup records remain individually named under `records/field/`. The
mode-2 descriptor, collision-record stream, and event table are direct source
records; the conversion loader interprets them into typed field state and a
host collision map without making the runtime read ROM addresses.
