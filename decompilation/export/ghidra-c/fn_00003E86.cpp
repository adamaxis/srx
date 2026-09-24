// Ghidra headless export
// Entry: 0x00003E86
// Name: FUN_00003e86
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00003e86(void)

{
  short sVar1;
  int in_A0;
  int unaff_A2;
  byte *pbVar2;
  byte *pbVar3;
  
  pbVar3 = *(byte **)(unaff_A2 + 2);
  pbVar2 = *(byte **)(in_A0 + 0x48);
  sVar1 = *(short *)(unaff_A2 + 6) + -1;
  do {
    *pbVar2 = (&DAT_00003edc)[(short)(char)(*pbVar3 >> 4)] << 4 |
              (&DAT_00003edc)[(short)(char)(*pbVar3 & 0xf)];
    sVar1 = sVar1 + -1;
    pbVar2 = pbVar2 + 1;
    pbVar3 = pbVar3 + 1;
  } while (sVar1 != -1);
  return;
}


