# Map viewer movable artwork

Defect: dragging a map entity moved its outline and labels while leaving the
map pixels behind.

## Source and implementation

Field art is extracted from the ROM's map descriptors into a flattened image,
a sparse priority image, and two separate source-plane images. The planes are
kept as inspection assets; they are not treated as whole objects. Each authored
entity part has a named alpha-masked crop and a placement relative to its
entity. Area instances now retain both their current position and their
original map position. **Editable objects** checks the crop against the pixels
at that original position, removes only matching pixels, and draws the crop as
part of the movable entity group. A mismatched part stays in the source image
and is listed for review rather than clearing unrelated pixels.

The ordinary full-map view remains the extracted image. Dragging an entity from
that view switches the editor to **Editable objects**, so a successful move
shows the actual annotated pixels moving. Saving retains the original position
needed to clear the old pixels when the map is reopened.

## Eight-question visual review

1. **Does it look correct?** The complete original-position render matches the
   extracted map plus its priority image pixel-for-pixel in each of the 56 map
   scenes. The selected Stoker close-up also shows the facade, roof, three
   hazard marks, side pipe, sign, and sign shadow moving as one entity.
2. **Does the display reflect the stored state?** The current origin controls
   where the art is drawn. A separate saved original origin controls which
   source pixels are removed. Scene 2 has 44 of 44 placed art parts that match
   their recorded source pixels; all six Stoker parts detach when moved.
3. **Are the visuals correct?** The viewer uses the extracted source pixels
   and part outlines directly. Across scenes 0–55 the editable render at the
   original positions had zero pixel differences from the extracted map plus
   its priority image. This checks the cutout/redraw operation, not the ROM
   extraction's independent visual fidelity or the completeness of the labels.
4. **Is it animated?** Map objects in this editor are static images. This
   feature adds no animation or frame cadence.
5. **Is there sound?** The editor has no sound behavior for moving annotations.
6. **Were other screen areas reviewed?** Full-window snapshots were inspected
   for the map, layer controls, object list, and route text. The close-up
   before/after move also showed the surrounding source map. No game HUD or
   gameplay route is part of this editor operation.
7. **Do multiple frames support the reading?** This is a static authoring
   view, so there are no animation frames. Evidence covers all 56 maps plus a
   Stoker original-position and moved-position snapshot. The moved snapshot is
   an editor preview, not a gameplay capture.
8. **Are new controls covering old controls?** The artwork selector reuses the
   existing map panel. The art itself is drawn in the map canvas; it does not
   add buttons or retain controls from another game screen.

## Coverage and limits

All 56 scenes preserved their full original image in the source-position
comparison. Of 420 placed entity-part records, 327 currently match the pixels
at their saved source locations and can detach in the preview. The other 93
stay in place and are identified in the selected object's part list; most are
in scenes 6 and 8–14, where the current part crop/placement data needs review.
Scenes 15–55 have no authored entity-part records yet. This feature makes
authored objects move as images; it does not finish object classification,
building labels, door routes, or gameplay rendering.

Inspection images are in the ignored `archive/qa-artifacts/visual-audits/` directory:

- `scene02-editable-art-only.png` — editable scene art at the original placement
- `scene02-composite-priority.png` — extracted map and priority reference
- `scene02-stoker-editable-original.png` — selected motel parts and route text
- `scene02-stoker-auto-editable-moved.png` — the group after a 64-by-48 preview move
