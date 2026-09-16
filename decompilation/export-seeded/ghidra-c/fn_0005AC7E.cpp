// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AC7E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ac7e(void)

{
  int iVar1;
  
  if (0x3b < (ushort)(_DAT_00ffdf88 - _DAT_00fff11e)) {
    if ((_DAT_00fff1a6 <= _DAT_00ffdf88) && (_DAT_00fff1a4 < 3)) {
      iVar1 = (int)(short)(_DAT_00fff1a4 << 3);
      _DAT_00fff1a0 = *(undefined4 *)((int)&PTR_DAT_0005ad02 + iVar1);
      _DAT_00fff1a8 = *(undefined2 *)(&DAT_0005ad06 + iVar1);
      _DAT_00fff1aa = *(undefined2 *)(&DAT_0005ad08 + iVar1);
      _DAT_00fff1a6 = _DAT_00ffdf88 + 6;
      _DAT_00fff1a4 = _DAT_00fff1a4 + 1;
    }
    FUN_00001378();
  }
  return;
}


