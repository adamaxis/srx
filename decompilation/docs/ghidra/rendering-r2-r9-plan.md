# Rendering R2/R9 plan: 3 name tables + VdpModel (design note, not implemented)

Source: /workspace/shadowrun/docs/21_rendering_entity_model.md §2 (R2, R9) and §3.
Status: R1, R3, R4, R10 done; R5/R6/R7 follow-ons tracked separately; R8/R11 are [H] and out of scope.
Hook points below are given by function name, because FSR line numbers move with the thinning work.

## Problem
- R2 [C]: the field map is one pre-flattened image (`field_map_`) plus one high-priority overlay image
  (`field_map_priority_overlay_`). Sprites are painted between the two layers in `paint_field_map_composition`
  (qt_runtime/src/field/field_map_composition.cpp). This is only correct while tiles never change.
  The ROM writes tiles at runtime: 0x406A..0x41A2 metatile writes, with priority inverted through FFDF58.
- R9 [C writes / H scenes]: planes A and B are flattened together. Planes A and B scroll identically, except for
  the plane B offset writes. [C] 0x1AB0 copies FFE00A -> FFDD0E, and 0x26E2 does `addq #1, FFDD0E` then sets FFEF14 bit 3.
  [H] Which scenes use the plane B offset visibly is not yet known.

## Target model
- `VdpModel` (new, qt_runtime/src/rendering/vdp_model.{hpp,cpp}, pure C++ with no Qt in its state):
  - VRAM tile store: 0x800 patterns of 8x8 4bpp.
  - CRAM: 4x16 words.
  - Name tables A, B and window: 64x32 words each. Each word holds priority, palette, vflip, hflip and the 11-bit tile index.
  - Scroll: per-plane H/V scroll words.
  - `compose(QImage&)` paints the planes in priority order: B low, A low, sprites low, B high, A high, window, sprites high.
- `PlaneStreamer`: a port of 0x2C92 (normal viewport, FFE100, FF2700) and 0x2CB2 (alternate, FFE102, FF4700, FFDF58=0x8000)
  through 0x3D18/0x3D4A. It writes name-table words into VdpModel, including the FFDF58 priority rule.
  - The conversion logic already exists in src/field/field_viewport_conversion.cpp (`render_normal_field_viewport`,
    `render_alternate_field_viewport`) and include/.../field_entity_render_batch_conversion.hpp (0x3D18 notes).
    The streamer should call these functions rather than re-derive them.

## Concrete hook points (current code)
1. **Asset load**
   - Where: `FieldSceneRuntime::bootstrap` (loads `field_map_` / `field_map_priority_overlay_` by alias) and
     `transition_to_field_destination` (field_scene_runtime_transitions.cpp, `.priority.overlay` alias).
   - Change: add loading of the scene's metatile/name-table source and tile patterns into VdpModel. Keep both images as a fallback behind a flag until parity.
2. **Runtime tile writes**
   - Where: the door, terrain-mark and metatile write paths that today only toggle state. Find them by ROM 0x406A..0x41A2.
   - Change: call `PlaneStreamer::write_metatile`.
3. **Scroll**
   - Where: the camera_x/camera_y source used by `paint_field_scene`.
   - Change: feed plane A scroll from it, and plane B scroll from camera plus the FFDD0E offset (R9).
4. **Compose**
   - Where: `paint_field_scene` -> `paint_field_map_composition(painter, field_map_, field_map_priority_overlay_, ...)`.
   - Change: replace the call with `vdp_model_.compose(...)`, passing the low/high sprite callbacks.
   - **paint_field_scene is owned by the combat branch.** Land this one-line swap only after that branch merges, or as a separately reviewed patch.
5. **Fades (R5) and latch (R6)**
   - Where: VdpModel's CRAM.
   - Change: once CRAM lives in VdpModel, the 0x1088/0x10D6 saturating fades become CRAM writes, and the VBlank latch (R6) becomes "apply pending SAT/scroll/CRAM at the start of compose".

## Order of work
1. VdpModel with unit tests: name-table word decode and the priority compose order.
2. A static-scene parity test against the current flattened images. Every scene must match pixel-exactly before the swap.
3. PlaneStreamer, wired to runtime tile writes, with a test against a door-open scene dump.
4. The plane B offset (R9), plus a capture comparison for a scene that uses FFDD0E.
5. The compose swap in paint_field_scene (after the combat merge).
