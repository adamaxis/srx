// Ghidra headless export
// Entry: 0x00010AF0
// Name: FUN_00010af0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00010918 FUN_00010918


void FUN_00010af0(void)

{
  FUN_00010918();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  VDP_CTRL = 0x8174;
  return;
}


