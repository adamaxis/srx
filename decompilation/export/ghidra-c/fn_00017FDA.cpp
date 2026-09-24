// Ghidra headless export
// Entry: 0x00017FDA
// Name: FUN_00017fda
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A FUN_00000d3a
//   0x000011F6 FUN_000011f6
//   0x00001232 FUN_00001232
//   0x00017FB4 FUN_00017fb4


void FUN_00017fda(void)

{
  short sVar1;
  
  if (DAT_00fff106 < 0) {
    sVar1 = FUN_00000d3a();
    if (sVar1 != 0) {
      return;
    }
    DAT_00fff106 = 7;
    DAT_00fff09a = FUN_00000d3a();
    sVar1 = FUN_00000d3a();
    if (sVar1 == 0) {
      DAT_00fff09a = DAT_00fff09a + 0x1c;
    }
    DAT_00fff09a = DAT_00fff09a * 2;
  }
  else if (DAT_00fff106 < 1) {
    FUN_00017fb4();
    goto LAB_00018148;
  }
  FUN_00017fb4();
  FUN_00001232();
  if ((DAT_00fff106 != 2) && (DAT_00fff106 != 7)) {
    FUN_00000d3a();
    FUN_000011f6();
    if ((DAT_00fff106 == 4) || (DAT_00fff106 == 5)) {
      FUN_00000d3a();
      FUN_000011f6();
      if (DAT_00fff106 == 5) {
        FUN_00001232();
      }
    }
  }
LAB_00018148:
  DAT_00fff106 = DAT_00fff106 + -1;
  return;
}


