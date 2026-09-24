// Ghidra headless export
// Entry: 0x0000ABFC
// Name: FUN_0000abfc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D7EA FUN_0000d7ea


void FUN_0000abfc(void)

{
  short sVar1;
  short extraout_D1w;
  short sVar2;
  int extraout_A0;
  
  sVar1 = FUN_0000d7ea();
  sVar2 = extraout_D1w * -4 + 0x6a;
  if (sVar1 != 0) {
    if (sVar1 == 2) {
      sVar2 = sVar2 * 2;
    }
    else {
      sVar2 = sVar2 >> 1;
    }
  }
  if (sVar2 < 0xf) {
    sVar2 = 0xf;
  }
  *(short *)(extraout_A0 + 0xdc) = sVar2;
  return;
}


