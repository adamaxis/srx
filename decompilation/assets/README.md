# Extracted asset taxonomy

These folders contain host-ready resources extracted from the Genesis image.
The native runtime consumes these artifacts and never opens the ROM.

| Folder | Scope |
| --- | --- |
| `title/` | Precolored title background, logo, text, circuit, and sprite assets |
| `boot/` | Precolored boot snow, SEGA, lightning, strobe, guy, and FASA assets |
| `startup/` | Intro, save/load, no-save, and archetype screens |
| `game/field/` | Field/map layers, entities, HUD, and movement visuals |
| `game/menu/` | Field command menu, cursor, boxes, and status UI |
| `game/items/` | Item art, inventory cells, and item-detail values |
| `matrix/menu/` | Matrix-specific screens and menu resources |
| `audio/` | Host-ready music and sound resources |
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
