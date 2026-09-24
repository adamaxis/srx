// Ghidra headless export
// Entry: 0x00010918
// Name: FUN_00010918
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 FUN_00001530
//   0x00010952 FUN_00010952
//   0x000566D8 FUN_000566d8


void FUN_00010918(void)

{
  VDP_CTRL = 0x8134;
  FUN_00010952();
  FUN_000566d8();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_00001530();
  return;
}


