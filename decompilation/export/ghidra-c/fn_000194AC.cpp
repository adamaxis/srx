// Ghidra headless export
// Entry: 0x000194AC
// Name: FUN_000194ac
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001792E render_menu_hud_rows
//   0x00019504 FUN_00019504


void FUN_000194ac(void)

{
  char unaff_D5b;
  
  FUN_00019504();
  DAT_00fff0d4 = unaff_D5b + DAT_00fff0d4;
  if ((short)((ushort)DAT_00fff0d4 << 8) < 0) {
    DAT_00fff0d4 = 4;
  }
  else if (-1 < (char)(DAT_00fff0d4 - 5)) {
    DAT_00fff0d4 = 0;
  }
  write_vdp_grid();
  render_menu_hud_rows();
  return;
}


