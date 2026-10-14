# Scene 19 pool-table map entities

**Status:** partial static-map classification. The viewer records the two table
art groups and their collision footprints. Their metatiles combine object and
background pixels, so the viewer keeps the source artwork in place until an
exact underlay is available.

## ROM and map evidence

- Scene 19's map planes store the table surfaces at `(64,96)` and `(160,96)`;
  visible table pixels start at map Y `120`. Each source placement spans two
  columns and two rows of metatiles. The separate orange line art sits at
  `(96,96)` and `(192,96)`.
- Scene 19 collision map base is `0x1D5ED8`. Records 8 and 9 at `0x1D5F28`
  and `0x1D5F32` cover `(64,128)..(127,159)` and `(160,128)..(223,159)` with
  raw value `0x007E`, classified as blocked by the source terrain probe.
- The tables are map art, not actor records. Their visual entities have no
  linked interaction. Event 30 at record 12 (`0x1D5F50`) is a separate
  top-left map contact region; it is not a table interaction.
- Longplay section 09 frames `006990` and `007035` show both tables while the
  player's position changes. They support the static repeated artwork only;
  they do not establish a table interaction, animation, or sound.

## Visual verification

1. **Does it look correct? — Yes for the static object classification.** The
   extracted map and independent frames show two green-felt tables with the
   orange wall strips above them. The source crops are in
   `video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_09_source_1020-1320/`.
2. **Does the visible look match behavioral state? — Yes for the evidence we
   have.** These are scenery tiles, not spawned people or selectable menu
   items. The two blocked terrain rectangles align with the lower table
   footprints. No event rectangle targets either table.
3. **Are the visuals correct? — Yes for source placement and layer identity.**
   Primary, secondary, and priority art remain separate; each orange strip is
   classified separately from its neighboring table. The entity outline
   follows the table silhouette and does not include surrounding floor.
4. **Should anything animate? — No animation is identified.** The visible
   tables are static map tiles. No actor animation record or table animation
   state is associated with them.
5. **Does it have sound? — No table-specific sound call is identified.** The
   scene's overall music and audio carry-over remain unresolved; this does not
   establish that the map is silent.
6. **Are the other visible parts clean? — Yes for the reviewed map region.**
   Both tables, both orange strips, and their separate collision rectangles
   were checked together. The old-man event-30 region remains elsewhere on the
   map and is not attached to either table.
7. **Do multiple frames support the interpretation? — Yes, with limits.**
   Frames `006990` and `007035` show the same table art at distinct player
   positions. They support static placement, not interaction or timing.
8. **Does the map redraw replace old graphics cleanly? — Yes.** No UI is added
   by this annotation. The editable viewer image matches `mode-19.bmp` at all
   71,680 pixels; the earlier inferred floor replacement was removed. The
   viewer disables movement unless every art part has an exact restorable
   underlay.
9. **Was the whole time window reviewed? — Not applicable to this static-art
   change.** No table event or transition was found, so this audit makes no
   timing claim. The two sampled longplay frames are not a full playback audit.

## Remaining limit

The ROM does not store a hidden floor beneath these table metatiles. Their
artwork and outlines are source-anchored until a non-inferred underlay can be
established. This map-viewer data is not a live gameplay entity implementation.
