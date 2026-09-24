// Ghidra headless export
// Entry: 0x00054A8E
// Name: FUN_00054a8e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 FUN_00000c24
//   0x00000D3A sample_source_random_bound
//   0x00000D94 FUN_00000d94
//   0x0000C0DA render_title_text_source
//   0x0001989A FUN_0001989a
//   0x000549E4 FUN_000549e4
//   0x000549F6 FUN_000549f6


void FUN_00054a8e(void)

{
  char cVar1;
  
  cVar1 = sample_source_random_bound();
  if (DAT_00fff0a8 <= cVar1) {
    return;
  }
  DAT_00fff0a8 = 0;
  FUN_0001989a();
  if (DAT_00fff0b1 == '\0') {
    FUN_000549f6();
    render_title_text_source();
    FUN_00000c24();
    FUN_00000d94();
  }
  FUN_000549e4();
  return;
}


