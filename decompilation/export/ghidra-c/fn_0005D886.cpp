// Ghidra headless export
// Entry: 0x0005D886
// Name: FUN_0005d886
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 FUN_00000e34
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x00001232 upload_hud_tile_block
//   0x000012F6 advance_title_palette_fade


void FUN_0005d886(void)

{
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  FUN_00000e34();
  upload_hud_tile_block();
  advance_title_palette_fade();
  return;
}


