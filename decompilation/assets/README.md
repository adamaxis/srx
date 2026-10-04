# Direct Shadowrun assets

Generated from the US Genesis ROM by shadowrun_graphics_extract --all-assets.

The native and Ghidra-conversion runtimes consume these individually named BMPs, direct text records, native/state.bin, and resources.tsv directly. This tree deliberately contains semantic folders: menu, intro, field, and item categories. Static intro plates and independently extracted intro sprites remain separate; it does not contain full-screen flipbook samples or an aggregate serializer. The field command overview's ROM-derived command, archetype, runner, status, power, armor, item-detail, and item strings are kept in `menu/field/text.tsv` and exposed through the `menu.field.text` manifest alias. Item art is grouped under items/weapons, items/first-aid, items/miscellaneous, items/magic, and items/armor. Shared field dialogue frames and source A/B/C marker sprites are grouped under dialogue/panel, with speaker portraits under dialogue/portraits; source-backed Stoker prompt text is in `dialogue/stoker-motel.tsv` under the `dialogue.stoker.motel` alias. Separately traced cinematic resources belong under movies/<scene>; they must not be mixed into intro or field assets.

Audio is emitted here only as complete Windows-playable lossless FLAC files under
`audio/`. The offline Z80/VGM extraction sources and intermediate event data
remain outside this runtime asset tree. Verified repeating audio uses
`audio/loop-points.tsv` for exact PCM frame boundaries. Use `resources.tsv` for stable aliases,
such as `assets.audio_path("audio.intro.title.stage-1")`; do not add raw ROM
slices or split event fragments here. A missing loop-point entry is
intentional: the native player must not guess a repeat boundary from an
internal channel restart.

Startup/session records are imported separately under `records/startup/` by `shadowrun_ghidra_startup_record_extract` and listed in `resources.tsv`. Field startup records remain individually named under `records/field/`; the complete 56-row scene profile table is `records/field/scene-profiles.bin`, and the complete field resource set is grouped under `records/field/scenes/scene-00` through `scene-55`. Each scene has its fixed terrain layers, offline-expanded metatile bytes, descriptor, collision records, and optional event table listed by a stable alias in `resources.tsv`. The conversion loader interprets each into typed field state and host collision maps without making the runtime read ROM addresses. The source movement vector table at `0xC002` is likewise one named direct record consumed as typed directional offsets by the conversion layer. The `0xBD34` collision-probe offset banks are kept as one four-bank direct record beside it. The `0x19D2` controller-heading table is kept beside it as one sixteen-word direct record. The `0xBBF6` aligned-route table is kept as four six-byte semantic records after its FFEF10..13 sample pointers are resolved offline.
