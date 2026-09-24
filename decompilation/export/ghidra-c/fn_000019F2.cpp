// Ghidra headless export
// Entry: 0x000019F2
// Name: FUN_000019f2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052DC2 FUN_00052dc2
//   0x00052DD6 FUN_00052dd6


void FUN_000019f2(void)

{
  byte bVar1;
  byte bVar2;
  short sVar3;
  
  FUN_00052dc2();
  IO_CT1_CTRL._1_1_ = 0x40;
  IO_CT1_DATA._1_1_ = 0;
  sVar3 = 10;
  do {
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  bVar1 = IO_CT1_DATA._1_1_;
  IO_CT1_DATA._1_1_ = 0x40;
  sVar3 = 10;
  do {
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  bVar2 = IO_CT1_DATA._1_1_;
  DAT_00ffef0e = ~(bVar2 & 0x3f | (bVar1 & 0x30) << 2);
  FUN_00052dd6();
  return;
}


