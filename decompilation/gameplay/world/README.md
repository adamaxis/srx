# Field map entity authoring data

`tools/map_entity_viewer.py` opens the extracted `field/maps/mode-N` images and
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
triggers over it. An annotation does not erase or move pixels in the BMP. A
map asset must be decomposed and rebuilt before moving a visual part can change
the image shown by the game.

## Entity definition

Each entity has its own folder under `entities/<entity-id>/`:

- `entity.tsv`: stable ID, display name, reviewed category, and evidence.
- `parts.tsv`: one row per visual part, with its source scene and source crop,
  entity-local position, draw depth, role, and evidence. A building can own a
  body, overhang, shadow, and decals while keeping one world origin.
- `interactables.tsv`: local interaction rectangles and their Lua handler,
  ROM event, and evidence fields. Unknown handlers and events remain blank.

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
python -m pip install -r decompilation/tools/requirements-map-viewer.txt
```

Then open a map:

```powershell
python decompilation/tools/map_entity_viewer.py --scene 2
```

The Qt menu bar has a **Scene** menu containing every extracted map. Choosing a
scene there or in the side-panel selector keeps both controls synchronized and
saves pending edits before switching.

To make a standalone Windows executable, double-click
`decompilation/tools/build_map_entity_viewer.cmd` or run it from PowerShell.
The executable is written to
`build/map-entity-viewer/shadowrun_map_entity_viewer.exe`. It includes Python,
PyQt6, and the viewer code; it reads the individually named extracted assets
and authoring files from the adjacent repository checkout. Keep the executable
inside that checkout (or launch it with `--assets` and `--world` paths).

Use **Add object region** to mark a map region for review. Use **Add visual
part** to add another source crop to the selected entity, and **Add
interactable** to add a local hit region. Select a part or interaction in the
right panel to edit or remove its metadata. Set the entity category only after
checking the source layers and ROM evidence. `Ctrl+S` saves the current scene
and its entity definitions.

On save, the viewer refreshes `gameplay/scripts/world_content_data.lua` from
these TSV files. The normal Lua package loader loads that data with the game.
`game.modules.world_content` provides `entity(id)`,
`instances_in_scene(scene_id)`, and `interaction_at(scene_id, x, y, event)` for
map scripts. Existing ROM collision and event routes remain authoritative
until a specific authored interaction is traced and connected to its Lua
handler. The viewer does not erase pixels from the precomposed map or change
the running map renderer.
