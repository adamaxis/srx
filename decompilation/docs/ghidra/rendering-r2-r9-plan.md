# Rendering R2/R9 plan: 3 name tables + VdpModel

Source: /workspace/shadowrun/docs/21_rendering_entity_model.md §2 (R2, R9) and §3.
Status: R1, R2 runtime mirror, R3, R4, R6 latch, R7 H40 SAT limits, R9 base scroll input, and R10 are implemented.
R6/R7 are wired into the opt-in field mirror; the SAT feed from paint is pending combat integration. R8/R11 are [H] and out of scope.

## Status
- **Done — R2 live mirror, default off:** `FieldSceneRuntime` recognizes
  `SRX_VDP_MODEL=1` or a QA override. With the flag enabled, field bootstrap
  and scene-buffer rebuilds load extracted Plane-A/Plane-B images into a
  `VdpModel` and stream the camera window through `PlaneStreamer`. The mirror
  derives indexed patterns and CRAM colors from those RGB images; priority is
  inferred from the extracted overlay [H]. The normal path retains the
  flattened images and allocates no mirror.
- **Done — runtime cell writes:** terrain marks and world-effect map writes
  refresh the mirror using `PlaneStreamer::write_metatile` alongside the
  existing flattened cell repaint. A headless CTest boots scenes 2 and 26,
  exercises a live scene-2 cell write, and reports pixel mismatch counts for
  `VdpModel::compose` against the current flattened-map composition. It is an
  internal synchronization check using the same extracted scene assets; it is
  not an independent visual-correctness claim.
- **Done — R9 base scroll:** the mirror uses the camera for Plane A and the
  camera plus the tracked Plane-B-minus-Plane-A vertical bias for Plane B.
  `0x1AB0` copies FFE00A to FFDD0E (0x1AA8 clears FFE00A first) [C]; `0x3CF2..0x3CFE` advance FFDD0A and FFDD0E by the same
  field-camera step [C]. `FieldCameraFollowState` tracks the A/B step biases,
  whose difference is zero in the 23 captured field save states [C, emulator].
  The independent increment at `0x26E2` is outside the field
  path [C]. A scene-specific offset capture is still needed before claiming
  which field scenes visibly use a nonzero offset.
- **Done — R6 frame latch in the opt-in mirror:** each completed `advance_frame`
  tick stages its field camera scroll and pending SAT pieces. The next tick
  latches them before field logic and streams the mirror from that displayed
  scroll. ROM `0x0BD2` is the main-loop VBlank wait; `0x1872` services the
  display-frame transfers after the wait, including SAT then H/V scroll [C].
  Mapping the host `advance_frame` boundary to that source wait is the runtime
  scheduling hypothesis [H]. Disabled mirror runs allocate no latch.
- **Done — R7 H40 limits in the opt-in mirror:** each latched SAT list runs
  through `apply_vdp_sprite_limits`, retaining per-piece visible scanlines,
  pieces rejected by the 80-entry cap, and scanline counts that reached the
  20-sprite and 320-visible-pixel limits [C: H40 VDP]. The
  `stage_field_sat_pieces` entry point is wired; after combat integration,
  `paint_field_scene_entities` should call it with the source-ordered SAT list
  before its reverse draw loop. The current field sprite list is local to that
  paint function, so no real SAT pieces are collected yet.
- **Left:** do not swap `paint_field_scene` to `VdpModel::compose` until the
  combat merge; that function remains combat-owned. Add an R9 offset capture
  after an independent field-path FFDD0E write is traced. R5 fade writes and
  paint integration of real SAT pieces remain pending.
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
1. **Asset load — done**
   - Where: `FieldSceneRuntime::bootstrap` (loads `field_map_` / `field_map_priority_overlay_` by alias) and
     `transition_to_field_destination` (field_scene_runtime_transitions.cpp, `.priority.overlay` alias).
   - Change: the opt-in mirror loads the scene's extracted primary/secondary plane images and streams them through `PlaneStreamer`; the flattened images remain the field renderer's input.
2. **Runtime tile writes — done**
   - Where: the door, terrain-mark and metatile write paths that today only toggle state. Find them by ROM 0x406A..0x41A2.
   - Change: `repaint_field_map_cell` writes both mirrored plane cells with `PlaneStreamer::write_metatile`; map reset rebuilds the mirror from the current scene assets.
3. **Scroll — base path done; nonzero R9 capture pending**
   - Where: the camera_x/camera_y source used by `paint_field_scene`.
   - Change: feed plane A from the camera and plane B from the camera plus the tracked B-minus-A vertical bias. The current field path keeps the biases equal.
4. **Compose**
   - Where: `paint_field_scene` -> `paint_field_map_composition(painter, field_map_, field_map_priority_overlay_, ...)`.
   - Change: replace the call with `vdp_model_.compose(...)`, passing the low/high sprite callbacks.
   - **paint_field_scene is owned by the combat branch.** Land this one-line swap only after that branch merges, or as a separately reviewed patch.
5. **Fades (R5); latch (R6) wired to the opt-in mirror**
   - Where: VdpModel's CRAM and `FieldVdpMirror::latch_vblank`.
   - Change: 0x1088/0x10D6 saturating fades still need CRAM writes. The R6
     latch applies pending SAT/scroll before field logic on the next host tick;
     it does not change composition or painting.

## Order of work
1. VdpModel with unit tests: name-table word decode and the priority compose order. **Done.**
2. Static-scene parity against the current flattened images for field modes 0, 2, and 26. **Done as a mirror/parity check only.**
3. PlaneStreamer runtime metatile writes and headless field-scene QA. **Done.**
4. Capture a field scene where the B-minus-A vertical offset is nonzero. **Pending; no independent field FFDD0E write is currently modeled.**
5. Compose swap in `paint_field_scene`. **Pending combat merge.**


## FFDD0E (plane B vertical scroll) capture, 2026-10-25
Evidence comes from a headless Genesis Plus GX trace from power-on with no input (`/tmp/boot_dd0e.py`). Captures and the trace are in `shadowrun/out/ffdd0e_capture/`: `trace.txt`, `f*_B*_A*.png` and `sheet.png`.
- **Title screen [C].** 0x1A9E is the title loop (0xE10 = 3600 frames, entered from boot at 0x7BA). 0x1AA8/0x1AB0 set FFDD0E = FFE00A = 0. In the loop, 0x26D0 runs `addq #1, FFDD0E` once every 5 frames (FFDD2E reloads with 4) and sets FFEF14 bit 3. The trace shows this: DD0E goes 1 at frame 789, 2 at 794 and so on, reaching 721 at 8864, then resets to 0 at 8965, while DD0A, DD08 and DD0C stay 0. Plane B (the circuit-board backdrop) scrolls up under a fixed plane A logo. This is the nonzero plane B offset scene.
- **Intro [C].** 0x5B06E steps FFDD0E up to 0x78. 0x5BCE0 subtracts 2 from both FFDD0A and FFDD0E (down to -0x90). During the pre-title sequence (frames 219..) the trace shows DD0A moving alone while DD0E stays 0.
- **Field [C, emulator].** 23 field save states (`shadowrun/emu/doors/*.state`) were each run 120 frames. FFDD0E - FFDD0A = 0 and FFDD0C - FFDD08 = 0 in every frame. This matches 0x3CF2..0x3CFE adding the same step to both, and the 0x1406 descriptors (+8..+15) all seed equal A/B words: 0x1FD8, 0x59D3E, 0x5B4A8, 0x5B528, 0x5B574 and 0x5C966 are all 0. The field VDP mirror's B offset of 0 is therefore correct for the field and no longer [H].
- **Port status.** The title screen already applies FFDD0E (`draw_wrapped_vertical_image` in the app window, not core-owned). [H] It wraps at the 224-px resource height, while a 64x32-cell plane wraps at 256 px. Above 224 the ROM shows name-table rows 28..31 instead of the top of the image. Owner: video/title. Verify with a frame at DD0E in 224..255 (about frame 1909+).
