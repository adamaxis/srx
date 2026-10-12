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
panel. The **ROM actor populations** layer draws the template-specific source
sprite and direction at each extracted map anchor. Click an icon to inspect
its name, kind, palette, template/list/position addresses, fixed-point
coordinates, ordered overrides, and activation rule. The extractor profiles
the default actor-list pointer in all 56 scene descriptors;
`default-actor-lists.tsv` records empty as well as populated lists. Default-list
records and faded conditional alternatives have different styling; neither
claims an actor is alive on every visit. Kind 15 (Thon) uses a source state
program rather than the ordinary frame descriptor, so its placement is kept by
name while its map icon remains an unresolved marker. These ROM overlays are
read-only evidence. `default-actor-waypoints.tsv` preserves each default
actor's ordered source coordinate list. **ROM waypoint paths** draws those
sequences as dashed cyan lines; the ROM actor service consumes the lists under
its behavior and state gates, so a path line is route data rather than proof
that an actor is currently moving. `world-activity-actors.tsv` separately
indexes the three ROM-triggered scene-47 lists. **Special actor lists** shows
their named actors with source-derived type and palette art as gated candidates;
the adjacent selector shows one group at a time by default. Choose all groups
only for comparison; that view does not claim the ROM runs them together.

The scene selector uses the ROM-backed contract district names for scenes 0–5.
**Map profile** lists the ROM-derived run-location names for those districts;
these names describe contract destinations and are not treated as proof of a
doorway route. Scene-1 (Puyallup) event markers have guide-marker crosswalks
for all twelve listed sites and all seventeen event records. The right dock's
**What happens**
tab shows each site's confidence, what contact does, where the route leads,
the known dialogue/menu/shop text, and story conditions. Guide labels are explicitly
marked as corroborated, candidate, or unresolved; they are not ROM names.
Unlabeled triggers still show the ROM's numeric scene/activity target and say
whether the route table or a special handler selects it.
The **What happens** pane uses player-facing descriptions and decimal map or
activity numbers. Internal handler names, addresses, and converted runtime
owners stay in **Source** so they are not presented as dialogue or menu text.
Selecting an entity shows its known interactions and imported dialogue text
when that content exists. **Map profile** shows the selected ROM
descriptor, camera setup, default three-member party placement, and the
profile-selected actor list. ROM `0x03084` loads the three profile positions;
`0x02B34` copies them into the three ordinary actor records when the entry
route has no pending coordinate-list override. The **Default party spawns**
layer marks all three defaults. An incoming transition can replace them, so
the profile points are not claimed as every route's arrival positions. Scene 2
slot 1 is the traced New Game lead position before the automatic Stoker walk.
Camera setup remains a separate value and is not treated as a player spawn.

## Current evidence coverage

- All 56 ROM field-map descriptors have an indexed default actor-list count;
  all source positions with supported frame art use that actor type's extracted
  sprite. The 69 conditional pedestrian positions and the three separate
  scene-47 activity lists are shown as gated candidates, not as a claim that
  every candidate is alive at once. Thon's source art program is not yet
  resolved to a viewer sprite, and other timed/event-created populations still
  need ROM profiling.
- Scene 2 currently has 22 authored visual entities and a route crosswalk for
  its 13 ROM event records. The entity list includes the named storefronts,
  facility/perimeter, and southern shelters. Routes keep numeric destinations
  numeric when the ROM does not identify a place or interior.
- Scene 0 has a crosswalk for all 21 ROM event rectangles, including ten
  guide-named sites, both labeled map edges, and four direct event-31 handlers.
  A local event-table result of scene 0 is kept distinct from the guide's
  Penumbra/Renraku connection labels. The ten named sites are now grouped map
  entities with their exact ROM event cells attached; the six visible paved
  approaches are separate terrain entities. Other unlabelled street structures
  and utility props still need source-backed classification.
- Scene 1 has twelve named Puyallup site entities with their ROM doorway/event
  rectangles attached. The facades, priority overhangs, and cracked-concrete
  forecourts are separate named parts/entities; the unnamed west-central
  frontage stays unnamed rather than inheriting a nearby shop's identity. The
  seventeen-event crosswalk still keeps the four event-31 records unresolved.
  Selecting a doorway or building shows its known route, branch, and available
  Lua text keys; missing dialogue, menu, shop inventory, and interior links are
  identified as unresolved instead of being inferred from the guide map.
- Scene 3 has all ten guide-numbered building sites (markers 2-11), three
  separately named but still-unidentified structures visible at the south map
  edge, eight separately outlined street corridors, and a crosswalk for all 20
  ROM event rectangles. Building outlines follow the structure crops and leave
  adjoining lot paving separate. The roadway classification describes the
  visible street surface only; it does not claim a collision mode. The five
  east-edge records stay distinct from the guide's Downtown label: the current
  ROM table result is field scene 10. The four direct event-31 cells remain
  unnamed, and guide-listed rest, contact, entry, and shop details are shown as
  guide evidence until their ROM/Lua branches are connected to those points.
- **Map audio** is present, but map-scoped music remains unresolved until the
  ROM's scene-entry and persistent-audio paths are traced. Menu music is shown
  separately and is not assigned to a map without evidence.

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

Area-level ROM actor origins are not scenery entities. The viewer reads
`field/entities/default-actors.tsv`, the 56-row
`field/entities/default-actor-lists.tsv` source census, and the ROM-imported
`field/entities/conditional-actor-candidates.tsv`. Default profile records
cover scenes 0-5, 20-43, 54, and 55. The conditional table contains the ROM's
69 one-of-three pedestrian candidates for scenes `0x15..0x2B`; it records the
event/world-mode gate, retained selection, template, position, heading, and
ordered overrides. Pedestrians, corp guards, and Yakuza Weenr use their own
extracted ROM kind, palette, and directional frame. Solid sprites are
profile-list records; faded sprites with amber dashed outlines are conditional
candidates. The three candidate icons describe alternatives and do not mean
that three NPCs are simultaneously active. They are separate from the orange
authored spawn marks, which remain unverified until their source behavior is
traced.

The conditional table can be regenerated and checked directly against the US
ROM with `decompilation/tools/extract_field_actor_candidates.py`. The graphics
importer exports all eight directional idle poses for the kinds present in the
56 default lists, plus the already traced guard and pedestrian entry poses.
Actor markers are drawn between the composed map and the independent
priority-pixel layer so foreground pixels can occlude them; map selection and
event/collision evidence remain separate overlays. Timed activity lists and
event-created actors remain a separate source-tracing task and are not
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
Scene 2 has a source-backed Stoker's Coffin Motel definition and 21 additional
Redmond frontage, barrier, and shelter definitions. Stoker's origin is ROM
event 2 at `(960,640)`. Its visual parts include the main facade, roof,
pipe-like attachment, roof stripes, entry threshold, sign, and separate shadow
components. The preview crops are polygon-masked; adjacent concrete is
excluded. ROM collision records stay in the scene's ROM-collision evidence
layer and are not claimed as motel-owned collision. The site's marker 6
corroborates the building name. The ROM route is same-map Stoker dialogue, not
an interior transition.
The Ares frontage has separate facade, masonry/door, and narrow east cable
parts; their source crops and outlines were checked against a 16-pixel map
grid. Hollywood Correctional's main structure and perimeter are separate
entities so the surrounding yard is not grouped into the building footprint.
The same scene also exposes the source-profiled New Game entry event. Its exact
scope is the scene-2 spawn `(0x3CE0,0x2D00)` and its marker appears in map cell
`(30,22)`, two 32-pixel cells below the motel doorway cell. ROM-derived
behavior is a one-time scene-entry rule at that exact spawn point, not a
three-cell walking region. The runtime reads the rule from generated
`world_content_data.lua` and calls the existing Lua heading-zero walk handler.
Clicking scene-2 site triggers opens the ROM route profile, with Lua dialogue,
menu, shop, and state details resolved where the corresponding content module
is available. Route destinations are kept distinct from guide labels and only
marked as traced where the ROM transition caller supports them. Map music is
still unresolved.

Actor overlays cover all extracted default-list records and the 69 conditional
pedestrian candidates in scenes 21-43, with named directional art for the
profiled types. Scene 47 also exposes the three source lists selected through
the `0x2CE0`/`0x43B6` activity paths, with a selector to isolate each list.
Other timed activity lists and
event-created populations still require source profiling. A record outside the
extracted map image is retained in the profile census and omitted from the map
overlay pending interpretation. Kind-15 Thon's sprite state program remains
unresolved. The three default party positions are shown by default;
route-specific overrides remain distinct. Entity drawings and generated part
assets remain authoring data; the live field renderer does not yet consume
these entity definitions.
