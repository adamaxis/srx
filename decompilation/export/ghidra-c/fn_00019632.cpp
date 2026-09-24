// Ghidra headless export
// Entry: 0x00019632
// Name: FUN_00019632
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid
//   0x0001792E render_menu_hud_rows


void FUN_00019632(void)

{
  short unaff_D4w;
  
  if ((&DAT_00fffbd8)[unaff_D4w] == '\x03') {
    DAT_00ff1592 = DAT_00ff1592 & 0xdf;
    DAT_00fff0d9 = 0;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\x05') {
    DAT_00fff0d2 = 0;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\x06') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfb;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\a') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfd;
    DAT_00ffe169 = 0;
  }
  (&DAT_00fffbd8)[unaff_D4w] = 0xff;
  (&DAT_00fff0ed)[unaff_D4w] = 6;
  write_repeated_vdp_grid();
  write_vdp_grid();
  render_menu_hud_rows();
  return;
}


