// Ghidra headless export
// Entry: 0x000100A8
// Name: FUN_000100a8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00010156 write_repeated_menu_grid
//   0x00010172 FUN_00010172
//   0x0001055E FUN_0001055e
//   0x000105E6 FUN_000105e6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000100a8(void)

{
  short sVar1;
  short sVar2;
  short in_D1w;
  
  DAT_00fff096 = in_D1w + DAT_00fff096;
  if (DAT_00fff098 < DAT_00fff096) {
    DAT_00fff096 = DAT_00fff098;
  }
  if (DAT_00fff096 == 0) {
    DAT_00fff096 = 1;
  }
  *(char *)(DAT_00fffc04 + 0x9d) = (char)DAT_00fff096;
  build_grouped_decimal_glyphs();
  FUN_00010172();
  write_repeated_vdp_grid();
  sVar2 = DAT_00fff098;
  sVar1 = DAT_00fff096;
  FUN_0001055e();
  DAT_00fff098 = sVar2;
  DAT_00fff096 = sVar1;
  write_repeated_menu_grid();
  FUN_000105e6();
  return;
}


