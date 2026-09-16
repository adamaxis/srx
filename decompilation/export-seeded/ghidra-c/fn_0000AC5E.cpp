// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AC5E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ac5e(void)

{
  byte bVar1;
  int in_A0;
  bool bVar2;
  
  if (((*(byte *)(in_A0 + 0x10) & 8) != 0) &&
     (((bVar2 = _DAT_00ffdf64 < 0, _DAT_00ffdf64 != 0 ||
       (bVar1 = *(byte *)(in_A0 + 0xb0), *(byte *)(in_A0 + 0xb0) = bVar1 & 0xdf, (bVar1 & 0x20) != 0
       )) && (bVar2)))) {
    FUN_0000b7b8();
  }
  return;
}


