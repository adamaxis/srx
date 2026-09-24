// Ghidra headless export
// Entry: 0x00019762
// Name: FUN_00019762
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void FUN_00019762(void)

{
  short unaff_D4w;
  int unaff_A4;
  
  *(undefined1 *)(unaff_A4 + unaff_D4w) = 0;
  write_vdp_grid();
  return;
}


