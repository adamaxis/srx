// Ghidra headless export
// Entry: 0x00002A94
// Name: initialize_mode_nine
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_mode_nine(void)

{
  prepare_mode_entry_service();
  DAT_00ffe114 = &DAT_00002ada;
  DAT_00ffddc4 = 9;
  DAT_00ffdf40 = 0;
  _DAT_00ffdf8e = 0xffff;
  DAT_00fffc10 = 3;
  _DAT_00fffc0e = 1;
  _DAT_00ffe10e = 1;
  return;
}


