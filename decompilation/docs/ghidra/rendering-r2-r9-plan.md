# Field rendering research: host compositor and QA mirror

Historical source: `/workspace/shadowrun/docs/21_rendering_entity_model.md` §2
(R2, R9) and §3; that linked file is unavailable in this checkout.
Status: R1, R3, R4, R9 base scroll input, and R10 are implemented. The R2/R6/R7/R8 VDP mirror is now QA-only; it is not part of the game executable. R11 remains [H] and out of scope.

## Status
- **QA diagnostic — R2 live mirror:** only the headless QA executable builds
  `FieldVdpMirror`. With the test override enabled, field bootstrap
  and scene-buffer rebuilds load extracted Plane-A/Plane-B images into a
  `VdpModel` and stream the camera window through `PlaneStreamer`. The mirror
  derives indexed patterns and CRAM colors from those RGB images; priority is
  inferred from the extracted overlay [H]. The normal path retains the
  flattened images. The interactive game target does not compile the mirror,
  model, frame latch, sprite-limit helper, or plane streamer.
- **QA diagnostic — cell-write synchronization:** terrain marks and
  world-effect map writes refresh the optional mirror through
  `PlaneStreamer::write_metatile` alongside the host asset repaint. A headless
  CTest boots scenes 2 and 26, exercises a live scene-2 cell write, and
  compares `VdpModel::compose` against the current flattened-map composition.
  This checks the diagnostic mirror only; gameplay consumes the host map and
  priority-overlay images directly. It is an
  internal synchronization check using the same extracted scene assets; it is
  not an independent visual-correctness claim.
- **Done — R9 base scroll:** the game renderer uses the shared field camera
  for its named map and priority overlay. The QA mirror replays the camera for
  Plane A and the camera plus the tracked Plane-B-minus-Plane-A vertical bias
  for Plane B.
  `0x1AB0` copies FFE00A to FFDD0E (0x1AA8 clears FFE00A first) [C]; `0x3CF2..0x3CFE` advance FFDD0A and FFDD0E by the same
  field-camera step [C]. `FieldCameraFollowState` tracks the A/B step biases,
  whose difference is zero in the 23 captured field save states [C, emulator].
  The independent increment at `0x26E2` is outside the field
  path [C]. Field plane B offset is zero in the captured field states.
- **QA diagnostic — R6 frame latch:** each completed QA `advance_frame`
  tick stages its field camera scroll and pending SAT pieces. The next tick
  latches them before field logic and streams the mirror from that displayed
  scroll. ROM `0x0BD2` is the main-loop VBlank wait; `0x1872` services the
  display-frame transfers after the wait, including SAT then H/V scroll [C].
  Mapping the host `advance_frame` boundary to that source wait is the runtime
  scheduling hypothesis [H]. Disabled mirror runs allocate no latch.
- **QA diagnostic — R7 H40 limits:** each latched QA SAT list runs
  through `apply_vdp_sprite_limits`, retaining per-piece visible scanlines,
  pieces rejected by the 80-entry cap, and scanline counts that reached the
  20-sprite and 320-visible-pixel limits [C: H40 VDP].
  `paint_field_scene_entities` stages its source-ordered image bounds before
  the reverse draw loop when the mirror is enabled. ROM 0x5B1E stably orders
  the source SAT and lower SAT indices win overlap [C]; representing each
  extracted host sprite image as one SAT piece is [H].
- **QA diagnostic — R8 field compose:** the QA override can compose Plane B/A
  and the existing field-sprite callback through `VdpModel::compose`. Scene 2
  and 26 QA compares complete field
  renders and requires zero pixel differences. VDP priority order is [C];
  reconstructing plane words from extracted assets remains [H]. The game
  always uses named map and priority-overlay assets through the host
  compositor.
- **Left:** R5 fade writes into CRAM remain pending.
Hook points below are given by function name, because FSR line numbers move with the thinning work.

## Source facts and host solution
- **R2 [C]:** the interactive renderer consumes the named `field_map_` and
  `field_map_priority_overlay_` images. Actors are composed between them by
  `paint_field_map_composition` (`qt_runtime/src/field/field_map_composition.cpp`).
  Live typed layer-word changes repaint the affected host image cell through
  `repaint_field_map_cell`; they do not require runtime name tables. The ROM
  writer paths include `0x406A..0x41A2`, with priority inverted through
  `FFDF58`.
- **R9 [C, emulator]:** `0x1AB0` copies `FFE00A -> FFDD0E`, and `0x26E2`
  increments `FFDD0E` and sets `FFEF14` bit 3. In 23 captured field save
  states, Plane A/B scroll deltas were equal; the field step at
  `0x3CF2..0x3CFE` advances both equally. This supports the current field
  renderer's shared camera input. Which other scene classes visibly use a
  separate plane-B offset remains [H].

## QA diagnostic model (not runtime architecture)
- `VdpModel` (`qt_runtime/src/rendering/vdp_model.{hpp,cpp}`) is compiled only
  into the headless QA executable. Its diagnostic state includes:
  - VRAM tile store: 0x800 patterns of 8x8 4bpp.
  - CRAM: 4x16 words.
  - Name tables A, B and window: 64x32 words each. Each word holds priority, palette, vflip, hflip and the 11-bit tile index.
  - Scroll: per-plane H/V scroll words.
  - `compose(QImage&)` paints the planes in priority order: B low, A low, sprites low, B high, A high, window, sprites high.
- `PlaneStreamer` reconstructs viewport writes for diagnostic comparisons. It
  is not called by the interactive game. The diagnostic plane-word inference
  from extracted RGB assets remains [H].

## Current implementation
- The game loads individually named scene map and priority-overlay assets in
  `FieldSceneRuntime::bootstrap` and
  `transition_to_field_destination` (`field_scene_runtime_transitions.cpp`).
- Terrain marks and world-effect tile writes update typed layer words and
  repaint the changed host-map cell. Optional `PlaneStreamer` synchronization
  remains only in QA.
- `paint_field_scene` always uses `paint_field_map_composition` in the game
  executable. The QA executable can opt into the mirror to compare the same
  host scene render; this is an internal synchronization check, not independent
  visual proof.
- Frame-latch and H40 sprite-limit checks are QA diagnostics. Their ROM source
  notes remain useful provenance, but these models do not drive game output.
- The mirror does not implement R5 CRAM fade writes. This remains a diagnostic
  limitation and is not part of the interactive renderer's architecture.

## Diagnostic work history
1. Pure C++ mirror primitives retain focused tests for name-table decoding,
   sprite limits, frame latching, and composition order.
2. Scene 2 and 26 mirror comparisons remain internal parity checks against the
   same extracted assets used by the host renderer.
3. Live cell-write synchronization remains covered by headless QA.
4. Field Plane A/B scroll equality was checked in 23 emulator save states.
5. The earlier optional compose swap has been removed from the game target;
   production always uses named assets and the host compositor.


## FFDD0E (plane B vertical scroll) capture, 2026-10-25
Evidence comes from a headless Genesis Plus GX trace from power-on with no input (`/tmp/boot_dd0e.py`). Captures and the trace are in `shadowrun/out/ffdd0e_capture/`: `trace.txt`, `f*_B*_A*.png` and `sheet.png`.
- **Title screen [C].** 0x1A9E is the title loop (0xE10 = 3600 frames, entered from boot at 0x7BA). 0x1AA8/0x1AB0 set FFDD0E = FFE00A = 0. In the loop, 0x26D0 runs `addq #1, FFDD0E` once every 5 frames (FFDD2E reloads with 4) and sets FFEF14 bit 3. The trace shows this: DD0E goes 1 at frame 789, 2 at 794 and so on, reaching 721 at 8864, then resets to 0 at 8965, while DD0A, DD08 and DD0C stay 0. Plane B (the circuit-board backdrop) scrolls up under a fixed plane A logo. This is the nonzero plane B offset scene.
- **Intro [C].** 0x5B06E steps FFDD0E up to 0x78. 0x5BCE0 subtracts 2 from both FFDD0A and FFDD0E (down to -0x90). During the pre-title sequence (frames 219..) the trace shows DD0A moving alone while DD0E stays 0.
- **Field [C, emulator].** 23 field save states (`shadowrun/emu/doors/*.state`) were each run 120 frames. FFDD0E - FFDD0A = 0 and FFDD0C - FFDD08 = 0 in every frame. This matches 0x3CF2..0x3CFE adding the same step to both, and the 0x1406 descriptors (+8..+15) all seed equal A/B words: 0x1FD8, 0x59D3E, 0x5B4A8, 0x5B528, 0x5B574 and 0x5C966 are all 0. The field VDP mirror's B offset of 0 is therefore correct for the field and no longer [H].
- **Port status.** The title screen already applies FFDD0E (`draw_wrapped_vertical_image` in the app window, not core-owned). [H] It wraps at the 224-px resource height, while a 64x32-cell plane wraps at 256 px. Above 224 the ROM shows name-table rows 28..31 instead of the top of the image. Owner: video/title. Verify with a frame at DD0E in 224..255 (about frame 1909+).
