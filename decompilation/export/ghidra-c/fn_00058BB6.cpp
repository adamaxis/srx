// Ghidra headless export
// Entry: 0x00058BB6
// Name: FUN_00058bb6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA FUN_0000c0da


void FUN_00058bb6(void)

{
  if (((DAT_00fff0b7 < '\0') || (DAT_00fff0b7 == '\0')) || (DAT_00fff0b7 != '\x01')) {
    FUN_00001232();
    FUN_0000c0da();
  }
  return;
}


