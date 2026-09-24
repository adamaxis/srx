// Ghidra headless export
// Entry: 0x0000CFFE
// Name: FUN_0000cffe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0C2 load_title_font
//   0x00010A82 FUN_00010a82
//   0x000566D8 FUN_000566d8
//   0x0005670E refresh_mode_palette_data


void FUN_0000cffe(void)

{
  FUN_000566d8();
  refresh_mode_palette_data();
  FUN_00010a82();
  load_title_font();
  return;
}


