// Ghidra headless export
// Entry: 0x00000EE6
// Name: FUN_00000ee6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 FUN_00000e34
//   0x00000F98 FUN_00000f98


void FUN_00000ee6(void)

{
  if (0x4f < DAT_00ffdd02) {
    DAT_00ffdbf9 = 0;
  }
  if (DAT_00ffdd02 != 0xffff) {
    FUN_00000f98();
    return;
  }
  FUN_00000e34();
  return;
}


