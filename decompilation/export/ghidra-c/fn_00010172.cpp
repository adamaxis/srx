// Ghidra headless export
// Entry: 0x00010172
// Name: FUN_00010172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 FUN_000011c0
//   0x00001232 write_vdp_grid


undefined8 FUN_00010172(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  FUN_000011c0();
  if (-1 < (short)(DAT_00fff096 + -2)) {
    FUN_000011c0();
  }
  write_vdp_grid();
  if (DAT_00fff098 != DAT_00fff096) {
    FUN_000011c0();
  }
  FUN_000011c0();
  return CONCAT44(in_D0,in_D1);
}


