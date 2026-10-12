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
The **Map artwork** selector includes the full map, each source artwork plane,
and **Editable objects**. The two planes are inspection views: ROM planes are
not semantic object layers, and a building can use pixels from both planes and
the priority overlay. When all placed parts use source-plane crops, Editable
objects keeps Plane A, Plane B, and priority art separate. It restores the
original Plane-A floor tiles, then draws named Plane-A parts, independent
Plane-B art, and priority art in source order. This preserves a separate
Plane-B object over a moved Plane-A shadow. Restoring a sparse Plane-B tile
replaces transparent pixels too, so a moved sprite cannot remain as a ghost at
its source position. Scenes with composite-source parts
use the full image and can only detach parts whose source pixels match. Dragging
an entity moves matched artwork with its annotations; incomplete outlines leave
unclassified pixels in place. A drag from the full-map view automatically
switches to this editable view. This is an authoring preview, not a claim that
every map object has been classified.
The area record keeps both the current placement and the original map position,
so a saved move can still clear its old pixels after the viewer is reopened.
Part images detach only when their visible source pixels match; mismatched parts
are listed as needing review and stay in the map until their source is corrected.
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

The scene selector uses the ROM-backed contract district names for scenes 0–5
and labels scenes 6–9 with explicitly marked map-guide cross-references.
**Profile** lists the ROM-derived run-location names for those districts;
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
runner- and transaction-specific adjustments remain separate. **Profile**
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

The **Profile** also lists known incoming routes from authored ROM route
profiles, including the source map, site cross-reference, numeric destination,
record addresses, and recorded conditions. This is a reverse index of profiled
route rows; an empty list means incoming routes have not been profiled here,
not that the ROM has no way into the map.

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
  numeric when the ROM does not identify a place or interior. Selecting the
  Rat's Nest record shows the existing three-page Vigore-search narrative,
  its pending/not-consumed condition, Landmark Map and Karma result, and the
  ordinary resource-activity fallback. The dialogue choice that sets the
  prerequisite remains unconnected, and this event does not prove a separate
  Rat's Nest interior.
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
  **Profile** lists character types, map positions, recorded
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
- Scene 6 matches Mike's RPG Center's Wilderness Area 1 map. Eight visible
  village structures are separate map entities: the two guide-marked lodges
  and six unnamed structures. Their facade/platform and priority-roof parts
  stay separate from the surrounding trails. The river and timber crossing
  are separate visual features. Five ROM table events and the direct event-31
  continuation are clickable: events 0, 1, 8, 9, and 10 select field map 13;
  event 31's continuation remains unresolved. Scene 13 is shared with the
  separate Jump House route, so the guide's David Owlfeather, Red Buffalo
  Woman, and cave labels remain cross-references rather than ROM interior
  names. Related Notebook text appears as related quest text, not as confirmed
  entry dialogue. The source scene-6 default actor list is empty, so the
  viewer does not place those named people as static map residents. Scene 13's
  additional world-script gate, the named locations' actual dialogue/speaker
  branches, return routes, and field audio remain unresolved.
  Its selector label is a guide cross-reference, not a ROM scene name.
- Scene 7 is Wilderness Area 3 by full-map visual cross-check. Its four ROM
  event rectangles are kept separate: marker 1 aligns with the Cave 2 guide
  route, marker 2 with Cave 3, and marker 3 with the visible longhouse the
  guide labels Stark. The fourth east-edge event has no matching named guide
  marker. Each event-table row selects shared field map 13; the guide labels
  do not establish that map's interior, speaker, dialogue, or return point.
  The longhouse remains a separate entity with an exact entrance event,
  front facade, priority roof, and foreground window detail. The Scene 7 actor
  list is empty, so the viewer does not turn its static map art into NPCs.
- Scene 8 is Wilderness Area 2 by full-map visual cross-check. Its four edge
  event rectangles align with guide markers 1–4, including the guide's two
  Cave 1 routes, a cave listed as having no point of interest, and a Cave 3
  route. All four ROM event-table rows select shared field map 13; the guide
  descriptions do not prove the contents of that destination. Scene 8's actor
  list is empty. Its fixed carved posts are annotated from isolated priority
  plane sprites, and its timber bridge reuses the Scene 6 entity definition
  after a pixel comparison. These are scenery entities, not spawned actors;
  the route and collision layers remain independent. **Profile** presents
  route records in marker order and keeps guide descriptions distinct from
  numeric ROM destinations.
- Scene 9 is Wilderness Area 4 by full-map visual cross-check. Its guide
  marker 3 aligns with the Sinsearach village gate; markers 1, 2, and 4 align
  with three cave-route rectangles. All four event-table rows select shared
  field scene 13. The next scene-selection step checks FFFC1E bit 19 and, when
  set, compares runner marker 7 before choosing either the field scene or a
  separate world script. The flag's story meaning and that script's dialogue
  remain unresolved. The gate's feathered monument and two fence panels are
  separate visual parts; its open passage stays clear. Two timber bridges are
  grouped scenery entities. The default actor list is empty, so map artwork is
  not promoted to residents.
- Scene 10 is labeled as the Friendship Restaurant route map because Council
  Island's guide marker 2 and Scene 4 event 1 identify that route. Scene 3 also
  yields numeric Scene 10 from an east-edge event, but that route's context is
  unresolved, so the label is not treated as a universal map identity. The
  editor separates the northwest doorway surround, back-bar/counter, seven
  potted palms, and five table fixtures. Each palm and table uses separate
  background and ROM-priority foreground parts. Its default actor list is
  empty; Walking Bear is guide-associated, but no static ROM actor placement
  or first-contact dialogue is added. The northwest event-30 continuation
  remains unresolved, as do the destinations of the two south-edge exits.
- Scene 11 is identified as The Big Rhino from its Penumbra route and the ROM
  contract-location table. Its map annotations separate the service counter,
  booth alcove, potted plants, 17 lit dining booths, six lower-room table
  fixtures, and two green wall panels. The adjacent event-30 cells at the dark
  booth open a source-backed Mortimer Reed dialogue profile with a 56x72 ROM
  portrait and information/work/leave choices. The ROM's conditional
  descriptor adds an alternate opening about Mortimer owing Joshua credits;
  the flag meaning and later choice effects remain unresolved. Nearby
  restaurant and package strings are labeled as related text until their exact
  trigger branches are traced. Scene 11 has no default map actors; the booth
  portrait is not shown as a spawned NPC. Its two event-8/event-9 markers
  remain unresolved.
- ROM event-29 floor selectors show the child-scene list extracted from the
  scene profiles, the Up/Down highlight behavior, A-button confirmation, and
  the separate Start exit. The panel keeps the lift landing distinct from a
  map's generic spawn and ordinary building entrance. These are numeric scene
  groups; the ROM evidence does not name the building.
- The source actor layer is data-driven across the extracted map catalog: the
  viewer uses each ROM record's actor type and sprite art, shows default scene
  records separately from conditional choices, and keeps special event lists
  separate from ordinary map populations. Event-character controls only appear
  on scenes that contain those lists. It does not synthesize scenery NPCs.
  Selecting a character shows its source list, position, facing, and route data.
- **Characters** gives a per-scene census with the matching extracted sprite,
  map position, and a plain-language category for ordinary actors, conditional
  alternatives, and event-only actors. The separate **Random events** tab lists
  the source event outcomes that can select temporary actors. A scene with no
  ordinary actor records stays explicitly empty; map artwork is never used to
  invent a person.
- **Audio** reports the field-entry trace. The traced New Game and ordinary
  scene-entry branches expose no scene-ID-to-track selection, and the scene
  descriptor has no music resource ID. This does not prove field silence:
  audio already active or queued by the previous mode may carry over, and
  field events can request sounds. Menu-mode music and effects are shown
  separately rather than assigned to maps without evidence. See
  `decompilation/docs/field-map-audio-profile.md` for the trace and limits.
- **Random events** lists the ROM's conditional random field-event outcomes for
  every extracted map context. The source maps 56 scenes through the 16-group
  table at `0x3312`, then uses two bound-five random draws to select one of
  nine sums. The 246 enabled scene/sum cells select 26 unique ROM handlers;
  their opening text and visible choices are decoded from the prompt records.
  Disabled cells enter separate dynamic actor sequences. The asset importer
  follows their tagged actor templates and marker-2 overrides for every map
  context and roll sum. The panel shows the actor's source name and extracted
  sprite beside the outcome. Those temporary actors appear at open edge points
  chosen from the current map state, so the source does not provide permanent
  map coordinates and they are not drawn as standing residents. Later choices
  may branch on live game state.

An authored entity is a grouped annotation: its visual-part regions and
interaction regions share one area origin. The outline and hit area follow
each visual part independently, while collision and interaction rectangles
remain separate layers. No bounding rectangle bridges empty space between
parts. Click a part outline to inspect the entity and drag it to move the whole
group. Switch to **Editable objects** to preview the move: the viewer restores
the source Plane-A tiles, draws named parts at their new origin, and keeps
Plane-B and priority art on their own layers. If a source part cannot be
matched to its recorded pixels or tile row, it stays at the source position
and the details panel reports it. The preview does not rewrite the extracted
map BMP.
Entity names remain in the inventory and appear on the map only for the
selected object, so dense rooms do not get covered by overlapping labels.
Unattached event callouts avoid other event rectangles and callouts; when a
clear position is unavailable, the event remains clickable and its name stays
in the right-hand details panel.
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
separate scene files and read by the runtime; blank handlers stay unassigned.
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

Selecting an authored building or object opens **What happens** automatically.
The panel presents the linked location name, interaction behavior, destination,
known speaker/dialogue, menu choices, shop catalog, and conditions. Clicking a
ROM doorway/event rectangle also opens this panel for that exact record.
Unlinked material stays marked unresolved; ROM addresses and internal handler
names remain in **Source**.

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
create separate map-scoped content records. A trigger with a nonzero
runtime event ID is loaded into the typed field event registry; its trigger
kind, exact scope, state flags, handler, and repeat policy are read from the
same TSV record by the game. Display coordinates are map pixels, while
`field_fixed_4` preserves exact actor coordinates from the source. The viewer
draws the containing 32-pixel cell for an exact point scope. Select a record on
the map to inspect/edit it. Set an entity category only after checking the
source layers and ROM evidence. Available categories include buildings,
terrain features, foliage, decals, props, and structures. `Ctrl+S` saves the
current scene and its separate entity and area TSV files.

The Lua package loader reads `gameplay/world/entities/<entity-id>/entity.tsv`,
`parts.tsv`, `interactables.tsv`, `collisions.tsv`, and the area,
trigger, spawn, and encounter TSV files under `gameplay/world/areas/` directly
when it starts. It places the
loaded records in an in-memory `game.modules.world_content_data` table;
there is no generated `world_content_data.lua` snapshot to edit or maintain.
The `game.modules.world_content` behavior module provides entity/placement
lookups plus `triggers_in_scene`,
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
three-cell walking region. The runtime reads the rule from
`scene-02-triggers.tsv` and calls the existing Lua heading-zero walk handler.
Clicking scene-2 site triggers opens the ROM route profile, with Lua dialogue,
menu, shop, and state details resolved where the corresponding content module
is available. Route destinations are kept distinct from guide labels and only
marked as traced where the ROM transition caller supports them. Map music is
still unresolved.

Scene 12 now has its own area/entity records for Icarus Descending. The dark
booth event links to the ROM's bodyguard gate and Caleb Brightmore profile,
including the first-meeting, repeat, and conditional post-run text. The viewer
imports Caleb's 56x72 portrait from profile `0x121E98`; it is dialogue art, not
a standing field character. The north-west wall panel, booth frame, north-east
display counter, five repeated dining booths, and five planters use separate
named entities and source crops. Event 8 and event 9 at the south map edge
remain unresolved. The guide's reputation-6 note stays separate from the ROM
`FFFC1E` bit-10 booth gate.

Scene 13 is cataloged as a shared interior map with a Jump House route
cross-reference. Its source event-30 cells open Gunderson's dialogue and are
attached to the north-west booth; the Lua profile exposes the information,
conditional question, contact-purchase, and leave pages. The 1,000-nuyen
contact purchase is sourced, while the work-contract branch remains partial.
The scene has no default actor records. Its two event-0 cells lie below the
visible map bounds and remain unresolved rather than being labeled as a return
door. Other districts also route numerically to scene 13, so their incoming
routes stay individually listed in **Profile**.

Scene 14 is cross-referenced to The Wanderer from Puyallup's scene-1 event
record 73 and ROM contract location `[1][0]`. Its event-30 cell selects the
Vigore/Jarl dialogue slot, with a first-visit introduction and a separate
state-gated booth-vacated branch. The shared Lua Johnson script owns the
first-visit pages and root menu; the conditional credit/quest outcomes and
event-0 exit remain partial. The source actor list is empty, so neither the
Vigore portrait nor random world-event candidates are presented as a standing
resident. The Profile tab lists the entry and unresolved lower-edge events
separately. The map entities distinguish the bar assembly, booth, pool tables,
lamp tables, square lamps, and east-wall fixtures across the composite and
priority layers.

Scenes 26, 27, and 28 each have a source-backed `Console table` prop
definition: seven placements in scene 26, thirteen in scene 27, and seven in
scene 28. The candidates are anchored by repeated Plane-A words
`0x0057/0x0058/0x0059`; all 27 table-body and priority-equipment crops match
the same source parts pixel for pixel. The 96x28 Plane-B monitor/lamp crop also
matches the priority-plane equipment crop pixel for pixel at all 27 instances;
the viewer keeps those as independent behind-actor and foreground parts. Each
ground shadow is a separate, instance-scoped part. The Scene 28 editable
preview restores Plane A and replaces sparse Plane-B cells, including their
transparent pixels, while keeping separate Plane-B furniture under two
overlapping shadows. Nearby 0x7E movement records remain independent. The
scene-2 routes that select scene 26 remain numeric resource-activity routes;
the repeated console art does not identify a named building or establish an
office interior. Scene 27's furniture, partitions, elevator geometry, and
entry context, and Scene 28's other room geometry, still need classification.

When inspecting a route, use **What happens** for the location name, player
flow, destination, dialogue pages, menu labels, shop contents, and known state
conditions. **Source** retains ROM records, Lua module paths, and low-level
cross-references. **Profile** lists ordinary scene actors and generic fallback
arrival positions; **Random events** lists conditional actor/event alternatives
without converting them into permanent map scenery. **Audio**
is present for every scene, but currently reports that no map-specific track
has been confirmed rather than assigning a guessed song.

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

The **Map artwork** selector includes the extracted full image, source-plane
inspection, and **Editable objects**. In editable mode, matched named art parts
are cut out of the map image and move with their area instance. The original
map position is retained when a placement is changed and saved. Parts whose
source art does not match their recorded position stay in place and are marked
for review. See the [movable-art visual audit](../../docs/visual-audits/map-viewer-movable-art.md)
for the 56-scene image comparison and current coverage.
