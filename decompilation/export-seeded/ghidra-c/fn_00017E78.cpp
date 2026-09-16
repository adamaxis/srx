// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017E78


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_00017e78(void)

{
  ushort uVar1;
  
  uVar1 = (ushort)*(byte *)((short)(_DAT_00fff118 * 0x14) + 0xff1110);
  if (-1 < DAT_00fff0a0) {
    if (DAT_00fff0a0 == '\0') {
      uVar1 = uVar1 + 1;
    }
    else {
      uVar1 = uVar1 + 2;
    }
  }
  return (uint)(ushort)(0x1b - uVar1) * 0x14;
}


