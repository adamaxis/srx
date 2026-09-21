# Extracted asset taxonomy

These folders contain host-ready resources extracted from the Genesis image.
The native runtime consumes these artifacts and never opens the ROM.

| Folder | Scope |
| --- | --- |
| `title/` | Precolored title background, logo, text, circuit, and sprite assets |
| `boot/` | Precolored boot snow, SEGA, lightning, strobe, guy, and FASA assets |
| `startup/` | Intro, save/load, no-save, and archetype screens |
| `field/map/` | Field map backgrounds for every decoded area, plus the ROM descriptor index |
| `field/entities/` | Field entity idle, walking, and grayscale sprites |
| `field/hud/` | Field HUD background, portraits, items, and counters |
| `field/menu/` | Field command menu, cursor, boxes, and status UI |
| `intro/` | Opening-stage backgrounds, vehicles, actors, projectiles, and patches |
| `matrix/menu/` | Matrix-specific screens and menu resources |
| `audio/` | ROM-extracted custom Z80 music and sound resources |
| `native/` | Legacy aggregate pack for screens not yet migrated to directory assets |

`docs/resource-catalog.tsv` is the provenance index. It records the original
ROM ranges and every known consumer so extraction does not create duplicate,
untraceable assets.

The title and boot assets are individual precolored BMP components. The title
circuit field is one background plus small per-phase light sprites; each
sprite's placement origin is recorded in `manifest.tsv`. The renderer supplies
the ROM-derived phase cadence. Generate them explicitly, once, with:

```powershell
& '.\build-debug\shadowrun_graphics_extract.exe' `
  '.\input\shadowrun_genesis_us.bin' '.\assets' --title-boot-assets
```

These files are not screenshots and do not require a runtime Genesis palette.

The sound import is separate because the ROM contains a custom Z80 sound
driver, not standard MIDI files. Extract its exact source slices with:

```powershell
& '.\build-debug\shadowrun_audio_extract.exe' `
  '.\input\shadowrun_genesis_us.bin' '.\assets'
```

This writes `audio/catalog.tsv`, the driver, the three pointer-delimited
initializer tables, and the fourth initializer's descriptor table. The raw
event data is the source for the later MIDI conversion hill; no guessed MIDI
files are generated.

`field/map/index.tsv` is the corresponding source index for graphics: it
contains one row for every field descriptor at ROM `0x3232`, including the
record, setup, palette, pattern, metatile, tilemap, event-table, and rendered
image dimensions.

The complete graphics import is a separate explicit step. It writes every
currently decoded background, sprite, animation frame, map, HUD layer, and menu
resource to individual precolored files and records its kind, dimensions, ROM
provenance, and extraction status in `catalog.tsv`:

```powershell
cmake --build .\build-debug --target shadowrun_all_assets
```

The opening export uses one static background per stage, individual SAT sprite
lists, and individual ambush map patches. It does not export the native
runtime's sampled full-screen ambush compositions. Fully transparent
state-specific resources remain as transparent BMPs and are marked
`extracted-empty`; `missing` is reserved for a decoder coverage failure.
