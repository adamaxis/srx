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
triggers over it; `--show-collision` includes the terrain bounds in a saved
viewer snapshot. Click an otherwise unmarked map cell to inspect its row-major
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
program rather than the ordinary frame descriptor. The importer resolves its
seven-piece sprite list and the scene-55 pattern upload into the named
`thon-idle.bmp` asset; the ROM points all eight directional selectors at that
same art. These ROM overlays are read-only evidence.
`default-actor-waypoints.tsv` preserves each default
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
Selecting an entity shows its known interaction areas, destination, story
conditions, linked dialogue and menu choices, and shop stock when those have
been source-profiled. Shop prices are identified as base template prices;
runner- and transaction-specific adjustments remain separate. **Map profile**
shows the selected ROM descriptor, camera setup, default three-member party
placement, and the profile-selected actor list. ROM `0x03084` loads the three
profile positions;
`0x02B34` copies them into the three ordinary actor records when the entry
route has no pending coordinate-list override. The **Default spawn** layer
marks the profile's fallback party positions. The lead position is the generic
map spawn; an incoming transition can replace it, so it is not claimed as
every route's arrival position. Scene 2 slot 1 is the traced New Game lead
position before the automatic Stoker walk. Camera setup remains a separate
value and is not treated as a player spawn.

## Current evidence coverage

- All 56 ROM field-map descriptors have an indexed default actor-list count;
  the viewer shows the 155 in-bounds default, conditional, and special-list
  actor marks across 33 populated scenes, each with its source actor art. The
  other 23 descriptors have no in-bounds actor rows in these profiles and
  remain empty rather than receiving invented NPCs. The 69 conditional
  pedestrian positions and the three separate scene-47 activity lists are
  shown as gated candidates, not as a claim that every candidate is alive at
  once. Thon's seven-piece source sprite is extracted from the scene-55 upload.
  The separate random event selector is now indexed for all 56 contexts; its
  dynamic actor/effect creation still has no fixed map coordinate in that
  selector data.
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
- Scene 4 has all six named buildings, the guide's Spirit Eyes site, and a
  crosswalk for all nine ROM event rectangles. Contract-location names and
  numeric event destinations remain separate; the additional Passport Lodge
  and Medicine Lodge vicinity events keep their downstream behavior unresolved.
  Related notebook, runner, service, and hotel text is shown as reference text
  only when it is not verified on that map route. The static figure art at the
  Spirit Eyes event cell is not added to the ROM actor population; Scene 4's
  three default pedestrians and their waypoint lists remain sourced from the
  extracted actor records. Route rows keep `player_summary`,
  `player_destination`, and `player_conditions` separate from the technical ROM
  trace shown under **Source**; `reference_text_keys` and `guide_reference` are
  explicitly labeled as related evidence rather than trigger dialogue. The
  route panel displays readable text without internal string keys; technical
  identifiers stay in the source trace. A separate annotation marks the
  restaurant connector, west lake dock, and narrow central bridge. These labels
  describe visible art only and do not invent collision behavior. The
  **Map profile** lists character types, map positions, recorded
  waypoints, conditional alternatives, and the descriptor's default fallback
  spawn positions.
- Scene 5's six ROM entrance events are cross-referenced to Renraku Arcology
  guide markers and the game's contract-location names. The **What happens**
  pane describes each route in plain language; the **Source** pane keeps the
  ROM record and table result. It lists guide-only store stock, fees, and named
  encounters separately from dialogue or menus linked to the trigger. The six
  storefronts have separate body/overhang/sign parts and their ROM entrance
  cells are clickable. The guide's Downtown connector remains distinct from its
  ROM table result, which currently selects no valid scene. The storefront
  bounds describe the visible structures; surrounding floor and entry cells
  remain separate annotations.
- The source actor layer is data-driven across the extracted map catalog: the
  viewer uses each ROM record's actor type and sprite art, shows default scene
  records separately from conditional choices, and keeps special event lists
  separate from ordinary map populations. Event-character controls only appear
  on scenes that contain those lists. It does not synthesize scenery NPCs.
  Selecting a character shows its source list, position, facing, and route data.
- **Map audio** reports the field-entry trace. The traced New Game and ordinary
  scene-entry branches expose no scene-ID-to-track selection, and the scene
  descriptor has no music resource ID. This does not prove field silence:
  audio already active or queued by the previous mode may carry over, and
  field events can request sounds. Menu-mode music and effects are shown
  separately rather than assigned to maps without evidence. See
  `decompilation/docs/field-map-audio-profile.md` for the trace and limits.
- **Field events** lists the ROM's conditional random field-event outcomes for
  every extracted map context. The source maps 56 scenes through the 16-group
  table at `0x3312`, then uses two bound-five random draws to select one of
  nine sums. The 246 enabled scene/sum cells select 26 unique ROM handlers;
  their opening text and visible choices are decoded from the prompt records.
  Disabled cells enter the separate dynamic world-effect path. These handlers
  do not carry a fixed map coordinate, so they appear in the text panel and are
  not drawn as standing NPCs. Later choices may branch on live game state.

An authored entity is a grouped annotation: its visual-part regions and
interaction regions share one area origin. The outline and hit area follow
each visual part independently, while collision and interaction rectangles
remain separate layers. No bounding rectangle bridges empty space between
parts. Click a part outline to inspect the entity and drag it to move the whole
group. This edits placement metadata only; it does not erase, extract, or move
pixels in the composed BMP.
A separate image-composition/import step is still required before authored
entity parts can replace pixels in a runtime map.

The **Terrain / event bounds** layer describes movement and route behavior,
not the visible identity of a wall, street, or building. It labels ROM
interaction records with the profiled site name and destination where those
are known, and distinguishes directional edges, object-bounds checks, the
always-blocked `0x7E` marker, and the `0x7F` boundary/event handoff. Raw values
and source record offsets remain available in the details. Visible map
features still require their own reviewed entity part; a collision rectangle
does not tell the viewer whether the pixels are pavement, a wall, or a shadow.
See `decompilation/docs/field-map-geometry-classification.md` for the ROM trace.

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

The conditional pedestrian table can be regenerated and checked directly
against the US ROM with
`decompilation/tools/extract_field_actor_candidates.py`. The random field-event
outcome table is generated by
`decompilation/tools/extract_world_activity_outcomes.py`; it profiles every
scene/sum selection cell and decodes each selected handler's initial text and
choices. The graphics
importer exports all eight directional idle poses for the kinds present in the
56 default lists, plus the already traced guard and pedestrian entry poses.
Actor markers are drawn between the composed map and the independent
priority-pixel layer so foreground pixels can occlude them; map selection and
event/collision evidence remain separate overlays. The random field-event
handler table is separate from actor placement; follow-up actor creation and
movement remain unresolved where the ROM uses dynamic coordinates. The viewer
does not turn those conditional events into ambient NPCs.

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
overlay pending interpretation. Kind-15 Thon's sprite uses seven records at
`0xA77A`; marker-2 entry to scene 55 loads their pattern tiles from `0x1EBDEE`
to VRAM `0x5920`. The importer resolves the source palette and offsets to
`field/entities/actors/thon-idle.bmp`. The three default party positions are shown by default;
route-specific overrides remain distinct. Entity drawings and generated part
assets remain authoring data; the live field renderer does not yet consume
these entity definitions.
