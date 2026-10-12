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
triggers over it. Click an otherwise unmarked map cell to inspect its row-major
primary/secondary ROM tilemap words, coordinates, dimensions, and extracted
record paths. These are source metatile indices, not semantic object labels.
Click a yellow trigger or a visible red collision rectangle to inspect its ROM
record, decoded route, addresses, and recorded runtime/QA owners in the side
panel. The **ROM actor populations** layer draws the source actor sprite and
direction at each extracted map anchor. Click an icon to inspect its type,
template/list/position addresses, fixed-point coordinates, ordered overrides,
and activation rule. Default-list records and faded conditional alternatives
have different styling; neither claims an actor is alive on every visit.
These ROM overlays are read-only evidence.

An authored entity is a grouped annotation: its visual-part regions and
interaction regions share one area origin. The outline and hit area follow
each visual part independently, while collision and interaction rectangles
remain separate layers. No bounding rectangle bridges empty space between
parts. Click a part outline to inspect the entity and drag it to move the whole
group. This edits placement metadata only; it does not erase, extract, or move
pixels in the composed BMP.
A separate image-composition/import step is still required before authored
entity parts can replace pixels in a runtime map.

Select an entity and use **Add collision** to draw an entity-local rectangle.
It appears as a green dash-dot outline and moves with the grouped entity
origin. ROM collision rectangles remain red, independent read-only evidence.
The Lua `world_content.collision_at(scene_id, x, y)` lookup resolves reviewed
entity collision against area placements; it returns nothing for unreviewed
regions. The live movement/collision path does not yet call this lookup.

The viewer also separates ROM evidence from authored map logic. Purple **T**
regions are Lua trigger areas, orange **S** marks are authored actor spawn
points, and blue **R** regions are authored encounter areas. These records have
condition/module/function fields and evidence/review status. They are saved as
separate scene files and exported as Lua data; blank handlers stay unassigned.
Click a mark to inspect it, edit it in the side panel, or remove it. They do
not overwrite or reinterpret the yellow ROM event rectangles.

## Entity definition

Each entity has its own folder under `entities/<entity-id>/`:

- `entity.tsv`: stable ID, display name, reviewed category, and evidence.
- `parts.tsv`: one row per visual part, with its source scene and source crop,
  entity-local position, draw depth, role, evidence, and generated asset path.
  `outline_points` stores one or more entity-local polygons (`x,y;x,y|...`);
  the viewer outlines and hit-tests each part separately, clips its named crop
  to those polygons, and does not draw a union rectangle across empty space.
  Saving materializes each crop as a named PNG under
  `decompilation/assets/field/entities/<entity-id>/`, preserving alpha from a
  priority-overlay source. The source kind and crop coordinates remain in the
  TSV as provenance. Selecting a part previews that exact named image in the
  properties panel. A building can own separate body, overhang, shadow, and
  decal images while keeping one world origin.
- `interactables.tsv`: local interaction rectangles and their Lua handler,
  ROM event, and evidence fields. Unknown handlers and events remain blank.
- `collisions.tsv`: local collision rectangles, an authored mode
  (`blocking`, `walkable`, or `nonblocking`), review status, and evidence.
  New regions remain `unreviewed` until supported by ROM evidence or an
  explicit new-content decision.

Area-level ROM actor origins are not scenery entities. The viewer reads both
`field/entities/default-actors.tsv` and the ROM-imported
`field/entities/conditional-actor-candidates.tsv`. The first has the two
profile-selected lists currently traced end to end (three scene-2 pedestrians
and eight scene-20 records). The second contains the ROM's 69 one-of-three
pedestrian candidates for scenes `0x15..0x2B`; it records the event/world-mode
gate, retained selection, template, position, heading, and ordered overrides.
Each marker uses the extracted pedestrian or corp-guard sprite in its ROM
direction with the source frame origin. Solid sprites are default-list
records; faded sprites with amber dashed outlines are conditional candidates.
The three candidate icons describe alternatives and do not mean that three
NPCs are simultaneously active. They are separate from the orange authored
spawn marks, which remain unverified until their source behavior is traced.

The conditional table can be regenerated and checked directly against the US
ROM with `decompilation/tools/extract_field_actor_candidates.py`. The graphics
importer exports all eight directional idle and entry poses for the actor
kinds used by the source lists. Actor markers are drawn between the composed
map and the independent priority-pixel layer so foreground pixels can occlude
them; map selection and event/collision evidence remain separate overlays.
Other timed activity lists and event-created actors are still open and are not
invented as ambient NPCs.

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

Area behavior is kept in typed files alongside placements:

- `areas/scene-NN-triggers.tsv`: map rectangles, Lua event/condition functions,
  repeat policy, evidence, and review status.
- `areas/scene-NN-spawns.tsv`: actor profile, map point, initial heading,
  condition, source record, evidence, and review status.
- `areas/scene-NN-encounters.tsv`: encounter region, Lua event pool, condition,
  source table, evidence, and review status.

These authored tables are extension content. The viewer does not seed them
from gameplay-video bodies, and an unreviewed entry does not become a ROM fact.

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

Use **Add object** to mark a map region for review. Use **Add visual part**
to add another source crop to the selected entity, **Add interactable** to
add a local hit region, and **Add collision** to add an entity-local collision
rectangle. **Add trigger**, **Add spawn**, and **Add encounter**
create separate map-scoped Lua content records. A trigger with a nonzero
runtime event ID is compiled into the typed field event registry; its trigger
kind, exact scope, state flags, handler, and repeat policy are read from the
same TSV record by the game. Display coordinates are map pixels, while
`field_fixed_4` preserves exact actor coordinates from the source. The viewer
draws the containing 32-pixel cell for an exact point scope. Select a record on
the map to inspect/edit it. Set an entity category only after checking the
source layers and ROM evidence. Available categories include buildings,
terrain features, foliage, decals, props, and structures. `Ctrl+S` saves the
current scene and exports the records to Lua.

On save, the viewer refreshes
`decompilation/gameplay/scripts/world_content_data.lua` from these TSV files.
The Lua package loader loads the module, and `game.modules.world_content`
provides entity/placement lookups plus `triggers_in_scene`,
`actor_spawns_in_scene`, `encounter_regions_in_scene`, and `collision_at`.
Scene 2 currently has one source-backed Stoker's Coffin Motel definition. Its
origin is ROM event 2 at `(960,640)`. The entity has separate outlines for the
main facade, priority roof, left pipe-like attachment, three roof stripes,
entry threshold, sign, and two shadow components. The preview crops are
polygon-masked; the adjacent concrete is excluded. ROM collision records stay
in the scene's ROM-collision evidence layer and are not claimed as motel-owned
collision. The site's marker 6 corroborates the building name. The ROM route
is same-map Stoker dialogue, not an interior transition.
The same scene also exposes the source-profiled New Game entry event. Its exact
scope is the scene-2 spawn `(0x3CE0,0x2D00)` and its marker appears in map cell
`(30,22)`, two 32-pixel cells below the motel doorway cell. ROM-derived
behavior is a one-time scene-entry rule at that exact spawn point, not a
three-cell walking region. The runtime reads the rule from generated
`world_content_data.lua` and calls the existing Lua heading-zero walk handler.
The other Redmond structures still need a reviewed entity pass. Actor markers
also remain incomplete: scene 2 and scene 20 have profile-selected default
lists, scenes 21-43 have gated pedestrian candidates, and timed activity or
event-created populations still require source profiling. Entity drawings and
generated part assets are authoring data; the live field renderer does not yet
consume these entity definitions.
