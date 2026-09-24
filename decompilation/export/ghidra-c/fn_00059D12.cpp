// Ghidra headless export
// Entry: 0x00059D12
// Name: FUN_00059d12
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x000012F6 advance_title_palette_fade
//   0x00001406 prepare_title_resources
//   0x000134D2 FUN_000134d2


void FUN_00059d12(void)

{
  prepare_title_resources();
  clear_and_fill_plane_b();
  clear_and_fill_plane_a();
  advance_title_palette_fade();
  DAT_00fff09e = 1;
  FUN_000134d2();
  return;
}


