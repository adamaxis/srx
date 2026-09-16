// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019BAE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 FUN_00019bae(void)

{
  byte bVar1;
  ushort uVar2;
  
  bVar1 = *(byte *)((short)(_DAT_00fff118 * 0x14) + 0xff1113);
  if (((DAT_00fff0a0 < '\x01') || ((char)(bVar1 - 7) < '\0')) || (-1 < (char)(bVar1 - 0x11))) {
    uVar2 = (ushort)bVar1 << 1;
  }
  else {
    uVar2 = (ushort)bVar1 * 2 + 1;
  }
  return (&DAT_000e31a7)[uVar2];
}


