// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001D1F4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001d1f4(void)

{
  ushort uVar1;
  short sVar2;
  int in_A0;
  int extraout_A1;
  
  _DAT_00ffe008 = _DAT_00ffe008 + -1;
  if (-1 < _DAT_00ffe008) {
    if ((_DAT_00ffdf88 & 3) == 0) {
      FUN_0001d1dc();
    }
    sVar2 = 0x46;
    do {
      uVar1 = FUN_00000d3a();
      FUN_00000e9c();
      _DAT_00c00000 = FUN_0001d3f4();
      _DAT_00c00000 = *(uint *)(extraout_A1 + (short)(uVar1 & 0xfffc)) & _DAT_00c00000;
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    return;
  }
  *(byte *)(in_A0 + 0x12) = *(byte *)(in_A0 + 0x12) & 0xef;
  FUN_0001d1dc();
  if (DAT_00ffe16b != '\0') {
    _DAT_00ffe0ae = 0;
    _DAT_00ffe0b0 = 4;
    _DAT_00ffdf8c = 1;
    _DAT_00ffdf76 = 0x70;
    _DAT_00ffdf78 = 0xaf;
    _DAT_00ffdf72 = 0xaf;
    _DAT_00ffdf7a = 0x1d428;
    _DAT_00ffdf7e = 0xda038;
    _DAT_00ffdf74 = 0x811b;
    _DAT_00ffdf70 = 0x51;
    _DAT_00ffdf82 = 0;
    return;
  }
  FUN_0001a01e();
  return;
}


