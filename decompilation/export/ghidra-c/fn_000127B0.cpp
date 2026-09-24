// Ghidra headless export
// Entry: 0x000127B0
// Name: FUN_000127b0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x000111D0 FUN_000111d0
//   0x000127FC FUN_000127fc


void FUN_000127b0(void)

{
  FUN_000111d0();
  DAT_00fff114 = DAT_00fff114 ^ 0xff;
  FUN_000127fc();
  render_title_text_source();
  return;
}


