// Ghidra headless export
// Entry: 0x00010984
// Name: FUN_00010984
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010918 FUN_00010918


void FUN_00010984(void)

{
  VDP_CTRL = 0x8134;
  FUN_00010918();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  if (DAT_00fff0af != '\0') {
    FUN_00001232();
  }
  VDP_CTRL = 0x8174;
  return;
}


