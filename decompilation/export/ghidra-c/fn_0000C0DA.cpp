// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C0DA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0000c0da(void)

{
  byte bVar1;
  undefined4 in_D0;
  short sVar2;
  undefined4 in_D1;
  ushort extraout_D1w;
  int extraout_A0;
  byte *extraout_A1;
  byte *pbVar3;
  
  do {
    FUN_00000e9c();
    pbVar3 = extraout_A1;
    while( true ) {
      bVar1 = *pbVar3;
      if ((char)bVar1 < '\0') break;
      sVar2 = bVar1 - 0x20;
      if (-1 < (char)((char)sVar2 + -0x3d)) {
        sVar2 = bVar1 - 0x24;
      }
      _DAT_00c00000 = extraout_D1w | *(ushort *)(extraout_A0 + (short)(sVar2 << 1)) | 0x8000;
      pbVar3 = pbVar3 + 1;
    }
  } while (-1 < (char)(bVar1 << 1));
  return CONCAT44(in_D0,in_D1);
}


