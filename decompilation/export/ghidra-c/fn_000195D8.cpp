// Ghidra headless export
// Entry: 0x000195D8
// Name: FUN_000195d8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001792E render_menu_hud_rows
//   0x000195A6 FUN_000195a6


void FUN_000195d8(void)

{
  short extraout_D1w;
  ushort uVar1;
  undefined1 unaff_D7b;
  int extraout_A1;
  
  FUN_000195a6();
  uVar1 = (ushort)DAT_00fff0d4;
  *(undefined1 *)(extraout_A1 + 0x1a + (int)(short)uVar1) = unaff_D7b;
  (&DAT_00fff0e8)[(short)uVar1] = 0;
  *(short *)((int)&DAT_00fff0f2 + (int)(short)(uVar1 << 1)) = extraout_D1w * 10;
  write_vdp_grid();
  render_menu_hud_rows();
  return;
}


