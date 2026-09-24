// Ghidra headless export
// Entry: 0x00058316
// Name: FUN_00058316
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA FUN_0000c0da


void FUN_00058316(void)

{
  undefined4 extraout_A1;
  
  if ((short)(DAT_00fff0d0 - 4) < 0) {
    FUN_0000c0da();
    FUN_0000c0da();
    FUN_00001232(extraout_A1);
  }
  FUN_0000c0da();
  return;
}


