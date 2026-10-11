# Field map entity authoring data

`decompilation/ghidra_conversion/tools/map_entity_viewer/map_entity_viewer.py`
opens the extracted `field/maps/mode-N` images and
the independent priority, terrain-collision, and event-trigger evidence. It
does not run an image classifier. New annotations start as `unreviewed` so a
background decal is not silently treated as scenery or as an entity.

## Coordinate system

Map and entity-part coordinates are integer pixels from the top-left corner of
the extracted `mode-N.bmp`. The editor grid is 32 pixels, matching one
source-map metatile. These coordinates are for authored map data; the game
actor's ROM fixed-point coordinates are a separate representation.

The BMP is already a composed map image. The priority overlay is a separate
sparse image, and the viewer can draw the ROM collision rectangles and event
triggers over it. Click a yellow trigger or a visible red collision rectangle
to inspect its ROM record, decoded route, addresses, and recorded runtime/QA
owners in the side panel. The **ROM default actors** layer marks coordinates
from the extracted default actor lists; click a marker to inspect its template,
fixed-point position, ordered overrides, and source record. A listed position
does not claim the actor is active on every visit. These ROM overlays are
read-only evidence.

An authored entity is a grouped annotation: its bounding region, visual-part
regions, and interaction regions share one area origin. Click the outline to
inspect the entity and drag it to move the whole group. This edits placement
metadata only; it does not erase, extract, or move pixels in the composed BMP.
A separate image-composition/import step is still required before authored
entity parts can replace pixels in a runtime map.

## Entity definition

Each entity has its own folder under `entities/<entity-id>/`:

- `entity.tsv`: stable ID, display name, reviewed category, and evidence.
- `parts.tsv`: one row per visual part, with its source scene and source crop,
  entity-local position, draw depth, role, and evidence. A building can own a
  body, overhang, shadow, and decals while keeping one world origin.
- `interactables.tsv`: local interaction rectangles and their Lua handler,
  ROM event, and evidence fields. Unknown handlers and events remain blank.

Area-level ROM actor origins are not scenery entities. The viewer reads the
source-derived `decompilation/assets/field/entities/default-actors.tsv` and
shows these as a separate evidence layer. Random/conditional encounter tables
must be profiled separately; a default-list position is not an encounter rule.

The first row in each file is a header. Keep one record per line; tabs and
newlines are reserved as separators.

## Area placement

`areas/scene-NN.tsv` contains one row per entity instance:

```text
instance_id    entity_id    origin_x    origin_y
```

The area file only places an entity definition. Visual composition and
interaction behavior stay with the entity definition and its Lua handler.
The same entity definition can be placed in more than one scene.

## Use

Requires Python 3.10 or newer and PyQt6 6.x. Install the dependency from the
repository root:

```powershell
python -m pip install -r decompilation/ghidra_conversion/tools/map_entity_viewer/requirements.txt
```

Then open a map:

```powershell
python decompilation/ghidra_conversion/tools/map_entity_viewer/map_entity_viewer.py --scene 2
```

The Qt menu bar has a **Scene** menu containing every extracted map. Choosing a
scene there or in the side-panel selector keeps both controls synchronized and
saves pending edits before switching.

To make a standalone Windows executable, double-click
`decompilation/ghidra_conversion/tools/map_entity_viewer/build.cmd` or run
`build.ps1` from PowerShell. The executable is written beside the viewer at
`decompilation/ghidra_conversion/tools/map_entity_viewer/dist/shadowrun_map_entity_viewer.exe`.
It includes Python,
PyQt6, and the viewer code; it reads the individually named extracted assets
and authoring files from the adjacent repository checkout. Keep the executable
inside that checkout (or launch it with `--assets` and `--world` paths).

Use **Add object region** to mark a map region for review. Use **Add visual
part** to add another source crop to the selected entity, and **Add
interactable** to add a local hit region. Select a part or interaction in the
right panel to edit or remove its metadata. Set the entity category only after
checking the source layers and ROM evidence. Available categories include
buildings, terrain features, foliage, decals, props, and structures. `Ctrl+S`
saves the current scene and its entity definitions.

On save, the viewer refreshes
`decompilation/gameplay/scripts/world_content_data.lua` from these TSV files.
The Lua package loader loads the module, and `game.modules.world_content`
provides `entity(id)`, `instances_in_scene(scene_id)`, and
`interaction_at(scene_id, x, y, event)`. The live field renderer and interaction
path do not yet consume these authored entities. The scene 2 map therefore
still renders from its extracted composite and source collision/event records;
this editor does not imply that an authored entity is already a runtime game
object. Authored actor spawn points and random-encounter regions are not yet
part of the editable content schema; the visible default actor layer is the
ROM's source-list evidence only.
