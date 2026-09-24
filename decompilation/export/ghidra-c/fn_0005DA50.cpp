// Ghidra headless export
// Entry: 0x0005DA50
// Name: setup_pre_title_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x00001232 write_vdp_grid
//   0x000012F6 advance_title_palette_fade
//   0x00001406 prepare_title_resources
//   0x00001530 decode_and_upload_source_stream
//   0x000176BA FUN_000176ba
//   0x0005DAE8 FUN_0005dae8


void setup_pre_title_resources(void)

{
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  prepare_title_resources();
  FUN_0005dae8();
  FUN_000176ba();
  decode_and_upload_source_stream();
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  advance_title_palette_fade();
  write_vdp_grid();
  return;
}


