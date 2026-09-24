// Ghidra headless export
// Entry: 0x00010276
// Name: FUN_00010276
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x00010172 FUN_00010172
//   0x00010260 FUN_00010260
//   0x00011190 FUN_00011190


void FUN_00010276(void)

{
  uint unaff_D7;
  
  FUN_00010260();
  FUN_00010172();
  FUN_00011190();
  render_title_text_source();
  render_title_text_source();
  render_title_text_source();
  render_title_text_source();
  FUN_000107ce((unaff_D7 & 0xffff) * 0x30);
  FUN_0001088c();
  FUN_0000c216();
  return;
}


