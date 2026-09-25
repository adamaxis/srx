// Ghidra headless export
// Entry: 0x00010172
// Name: convert_menu_routine_0x00010172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x00001232 write_vdp_grid


undefined8 convert_menu_routine_0x00010172(void)

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


