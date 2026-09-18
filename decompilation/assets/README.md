# Extracted asset taxonomy

These folders contain host-ready resources extracted from the Genesis image.
The native runtime consumes these artifacts and never opens the ROM.

| Folder | Scope |
| --- | --- |
| `title/` | Title logo, title animation, and title text layers |
| `startup/` | Intro, save/load, no-save, and archetype screens |
| `game/field/` | Field/map layers, entities, HUD, and movement visuals |
| `game/menu/` | Field command menu, cursor, boxes, and status UI |
| `game/items/` | Item art, inventory cells, and item-detail values |
| `matrix/menu/` | Matrix-specific screens and menu resources |
| `audio/` | Host-ready music and sound resources |
| `native/` | Runtime asset packs assembled from the categorized resources |

`docs/resource-catalog.tsv` is the provenance index. It records the original
ROM ranges and every known consumer so extraction does not create duplicate,
untraceable assets.
