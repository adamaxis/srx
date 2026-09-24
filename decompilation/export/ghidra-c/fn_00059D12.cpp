// Ghidra headless export
// Entry: 0x00059D12
// Name: FUN_00059d12
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x000012F6 advance_title_palette_fade
//   0x00001406 prepare_title_resources
//   0x000134D2 FUN_000134d2


void FUN_00059d12(void)

{
  prepare_title_resources();
  FUN_0000102a();
  FUN_00000ffe();
  advance_title_palette_fade();
  DAT_00fff09e = 1;
  FUN_000134d2();
  return;
}


