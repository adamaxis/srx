// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00011136


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_00011136(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = 7;
  do {
    FUN_00003602();
    FUN_00011190();
    uVar1 = uVar2 >> 1;
    uVar2 = uVar2 - 1;
  } while (uVar2 != 0xffff);
  return uVar1;
}


