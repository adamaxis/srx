// Ghidra headless export
// Entry: 0x00019762
// Name: clear_menu_status_at
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void clear_menu_status_at(void)

{
  short unaff_D4w;
  int unaff_A4;
  
  *(undefined1 *)(unaff_A4 + unaff_D4w) = 0;
  write_vdp_grid();
  return;
}


