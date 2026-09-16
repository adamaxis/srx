// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0002679E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0002679e(void)

{
  byte bVar1;
  ushort uVar2;
  ushort uVar3;
  byte *pbVar4;
  byte *pbVar5;
  byte *pbVar6;
  
  DAT_00fff0e1 = 0xff;
  if ((&DAT_00ff1100)[(short)(_DAT_00fff118 * 0x14)] == '\0') {
    pbVar5 = &DAT_0014c11e;
  }
  else {
    pbVar5 = (byte *)0x14c186;
  }
  _DAT_00ffe1aa = 0xff9700;
  _DAT_00ffe1ae = &DAT_00ff8700;
  pbVar4 = (byte *)0x14c1eb;
  pbVar6 = pbVar5 + 1;
  uVar3 = (ushort)*pbVar5;
  do {
    uVar2 = 2;
    pbVar5 = pbVar6;
    while( true ) {
      while( true ) {
        do {
          FUN_00000bd2();
          FUN_00000bf2();
          FUN_00017cc8();
          FUN_00017fda();
          uVar2 = uVar2 - 1;
        } while (uVar2 != 0xffff);
        bVar1 = *pbVar5;
        if (-1 < (char)bVar1) break;
        if ((bVar1 >> 4 & 7) != 0) {
          _DAT_00ffdd28 = _DAT_00ffdd28 - (ushort)(bVar1 >> 4 & 7);
        }
        if ((*pbVar5 & 7) != 0) {
          _DAT_00ffdd2a = _DAT_00ffdd2a - (ushort)(*pbVar5 & 7);
        }
        uVar2 = 0;
        pbVar5 = pbVar5 + 1;
      }
      _DAT_00ffe1a2 = (ushort)(bVar1 >> 4) + _DAT_00ffe1a2;
      pbVar6 = pbVar5 + 1;
      if (*pbVar5 != 0) break;
      uVar2 = (ushort)*pbVar4;
      pbVar4 = pbVar4 + 1;
      pbVar5 = pbVar6;
    }
    _DAT_00ffe1a4 = (*pbVar5 & 0xff0f) + _DAT_00ffe1a4;
    _DAT_00ffe1ae = &DAT_00ff8700;
    FUN_00024e20();
    FUN_00000f98();
    _DAT_00ffdd02 = 1;
    FUN_0001c3fa();
    FUN_000268e6();
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
    uVar3 = uVar3 - 1;
  } while (uVar3 != 0xffff);
  return;
}


