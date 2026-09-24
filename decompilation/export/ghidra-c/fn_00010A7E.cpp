// Ghidra headless export
// Entry: 0x00010A7E
// Name: FUN_00010a7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 FUN_00010918


void FUN_00010a7e(void)

{
  FUN_00010918();
  VDP_CTRL = 0x8134;
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  VDP_CTRL = 0x8174;
  return;
}


