// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000564D6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000564d6(void)

{
  short sVar1;
  ushort in_D1w;
  ushort uVar2;
  
  sVar1 = _DAT_00fff118 * 0x14;
  uVar2 = in_D1w >> 1;
  if (uVar2 == 0 || (short)in_D1w < 0) {
    uVar2 = 1;
  }
  *(char *)(sVar1 + 0xff1106) = *(char *)(sVar1 + 0xff1106) - (char)uVar2;
  if ((char)(*(char *)(sVar1 + 0xff1106) + -2) < '\0') {
    *(undefined1 *)(sVar1 + 0xff1106) = 2;
  }
  return;
}


