// Ghidra headless export
// Entry: 0x00010A82
// Name: FUN_00010a82
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232


void FUN_00010a82(void)

{
  VDP_CTRL = 0x8134;
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  VDP_CTRL = 0x8174;
  return;
}


