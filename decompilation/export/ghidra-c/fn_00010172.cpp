// Ghidra headless export
// Entry: 0x00010172
// Name: refresh_magic_force_display
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid


undefined8 refresh_magic_force_display(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  write_repeated_vdp_grid();
  if (-1 < (short)(DAT_00fff096 + -2)) {
    write_repeated_vdp_grid();
  }
  write_vdp_grid();
  if (DAT_00fff098 != DAT_00fff096) {
    write_repeated_vdp_grid();
  }
  write_repeated_vdp_grid();
  return CONCAT44(in_D0,in_D1);
}


