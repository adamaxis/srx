// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A818


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001a818(void)

{
  if (_DAT_00ffe0d4 != 0) {
    return;
  }
  _DAT_00ffe076 = 0;
  _DAT_00ffe082 = 1;
  if (_DAT_00ffe0f4 == 0) {
    _DAT_00ffe0f2 = 1;
    DAT_00ff1642 = 6;
    DAT_00ff1643 = 0;
    return;
  }
  _DAT_00ffe0f2 = 0;
  _DAT_00ffdf7e = *(undefined4 *)(*(int *)(_DAT_00ff1646 + (short)((ushort)DAT_00ff1643 << 2)) + 2);
  _DAT_00ffdf76 = 0xd280;
  DAT_00ff1642 = 0;
  DAT_00ff1644 = 7;
  FUN_0001b680();
  return;
}


