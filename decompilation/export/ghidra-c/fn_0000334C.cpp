// Ghidra headless export
// Entry: 0x0000334C
// Name: upload_field_asset_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001232 write_vdp_grid
//   0x000033AA draw_field_asset_grid


void upload_field_asset_sequence(void)

{
  write_vdp_word_block_with_source_wrap();
  write_vdp_grid();
  write_vdp_grid();
  draw_field_asset_grid();
  draw_field_asset_grid();
  draw_field_asset_grid();
  return;
}


