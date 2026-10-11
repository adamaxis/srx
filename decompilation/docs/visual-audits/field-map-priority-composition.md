# Field map foreground priority and actor occlusion

Date: 2026-10-11

**Status:** partial. The Qt conversion renderer now preserves the ROM's sparse
high-priority field pixels over ordinary actors for every loaded map. The
flattened map remains the visual base; extraction also emits one named
transparent PNG per scene for the foreground pixels. This fixes the reported
scene-26 terminal/table occlusion. It does not rescale map art or actor sprites,
and independent Genesis captures for the affected map are not identified.

## Source trace and implementation

Scene setup at ROM `0x03074` reaches the two-plane field map path. The ROM
`0x3D80` tile-word writer toggles the secondary-plane priority bit using
`FFDF58`; it does not make every pixel of the flattened composite one depth
level. ROM `0x481E` separately builds ordinary field actor sprite records.
Flattening the map before drawing those actors discarded which map pixels were
supposed to cover them.

The offline importer now retains the selected high-priority pixels in
`GenesisFieldMapLayers::priority_overlay` while continuing to emit the
flattened BMP used for inspection and the background pass. The extractor writes
an individually named RGBA PNG for each of the 56 scene maps and lists it in
`resources.tsv` and `field/maps/index.tsv`. The active Qt `FieldSceneRuntime`
loads and size-checks the map/overlay pair on initial scene setup and on every
scene transition. Its composition order is: map crop, low-priority actors,
transparent priority overlay, then the field HUD. Native shared asset loading
also decodes these direct PNG assets.

All 56 overlay images are checked against their map dimensions. Every opaque
overlay pixel must exactly match the flattened map's RGB at that coordinate;
empty pixels remain transparent. Scene 26 additionally checks that a sourced
foreground pixel covers a synthetic actor while a transparent pixel leaves the
actor visible. The map and overlay are thus two depth passes from the same ROM
map pixels; this change does not invent table colors or geometry.

I also reviewed map-wide overview sheets covering all 56 extracted maps, with
the ROM-selected foreground pixels tinted for inspection. They show the range
of field, building, resource, and scripted-activity layouts and no missing
map/overlay images. This is an extraction sanity pass; the sheets are not
independent Genesis captures or a substitute for scene-by-scene route QA.

## Eight-question full-screen review

1. **Does it look correct?** The user's supplied scene-26 captures identify
   the teal terminal/table as the object and its black silhouette as the
   shadow. The corrected composition keeps the shadow in the base map and
   restores the table's high-priority pixels over an actor. All 614 captured
   route frames were reviewed in chronological contact sheets; the field,
   actor, HUD, terminal, menu, transition, and return remain present. This
   verifies the layer change in the host route, not against a Genesis golden.
2. **Does the look reflect behavioral state?** The actor is rendered between
   the base map and priority pixels, so ordinary movement changes actor state
   without changing the static table or shadow. The active transition loader
   swaps map and overlay together and rejects mismatched dimensions.
3. **Are the visuals correct?** The selected RGB values come directly from
   the source map composition; the sparse overlay adds no color or scaling.
   The route image and the 56-map asset checks are available under the build
   output. No independent Genesis frame shows this exact scene-26 overlap.
4. **Is it animated?** The table and shadow are static map art. The runner's
   movement animation remains driven by the existing actor update path. The
   priority pass adds no animation or timer.
5. **Is there a sound?** The map pixels add no sound request. Actor movement
   and field interaction sound behavior is unchanged and was not used as proof
   of the depth fix.
6. **Are the other screen areas correct too?** The map crop still uses the
   current camera rectangle; actor rendering remains in its callback, and the
   HUD draws after both map passes. The full route contact sheets were checked
   for changes to the HUD, terminal panel, menu, transitions, and return. The
   known scene-2 actor/HUD overlap remains a separate open defect.
7. **Do multiple frames support the interpretation?** The route QA contains
   614 consecutive full-screen frames through entry, movement, both locks,
   terminal result, menu navigation, and return. Eight chronological sheets
   were reviewed. These are multiple host frames, not independent game
   captures; the user-provided images are the visual report for the table and
   shadow.
8. **Are new controls drawn over stale controls?** The priority image has
   alpha zero outside its selected map pixels and contains no buttons or
   panels. It is drawn before the HUD. Existing modal screens replace the
   field composition and the tested return redraw clears them; this layer does
   not retain controls from a prior screen.

## Verification and limits

The focused CTests are
`shadowrun_ghidra_conversion_qt_field_map_composition_qa`,
`shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`, and
`shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa`. The latter two
write full-screen `actual.bmp`/route frames through the production Qt runtime.
The composition test validates every map/overlay asset and uses a real
scene-26 overlay pixel to enforce foreground ordering.

This establishes the map/actor draw-order correction and asset compatibility.
It does not establish exact map-object bounds, every collision rectangle's
meaning, every building's named destination, or Genesis-faithful pixel timing.
The Scene-2 Redmond event destinations are documented separately in the
[ROM and guide crosswalk](mikes-rpg-center-shadowrun-reference-notes.md#redmond-rom-coordinate-cross-check-jump-house-and-rats-nest).
