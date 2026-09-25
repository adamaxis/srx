// Ghidra headless export
// Entry: 0x000537FA
// Name: refresh_runner_selection_sprites
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::refresh_runner_selection_sprites.
// The source resolves the currently selected 0x14-byte roster record, scales
// its two sprite-origin bytes by 0x10, uses animation seed 0x2000, and rebuilds
// the sprite records. The native callback supplies the already-resolved typed
// record; no source pointer arithmetic or ROM-address state is retained.
//
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00053248 build_title_sprite_once

void refresh_runner_selection_sprites(void)
{
  build_runner_selection_sprite_records_once();
  const auto source = resolve_selected_runner_sprite_source();
  if (!source) {
    return;
  }
  build_runner_sprites({
      static_cast<short>(source->screen_x) * 0x10,
      static_cast<short>(source->screen_y) * 0x10,
      0x2000});
}
