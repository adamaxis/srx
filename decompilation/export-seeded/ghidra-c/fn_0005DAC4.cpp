// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005DAC4


void FUN_0005dac4(void)

{
  ushort uVar1;
  short sVar2;
  ushort *extraout_A0;
  
  sVar2 = 0xdf;
  do {
    uVar1 = FUN_00000d44();
    *extraout_A0 = uVar1 >> 5 & 0x1f;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  FUN_0005dafc();
  return;
}


