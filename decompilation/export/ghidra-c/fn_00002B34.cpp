// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00002B34


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002b34(void)

{
  ushort *puVar1;
  
  puVar1 = _DAT_00ffe114;
  if (_DAT_00ffe114 == (ushort *)0x0) {
    _DAT_00ff0100 = _DAT_00ffde08;
    _DAT_00ff0104 = _DAT_00ffde0a;
    _DAT_00ff0200 = _DAT_00ffde0c;
    _DAT_00ff0204 = _DAT_00ffde0e;
    _DAT_00ff0300 = _DAT_00ffde10;
    _DAT_00ff0304 = _DAT_00ffde12;
    return;
  }
  _DAT_00ffe114 = (ushort *)0x0;
  _DAT_00ffddc6 = *puVar1 & 0xff80;
  _DAT_00ffddc8 = puVar1[1] & 0xff80;
  _DAT_00ffdf36 = 0xffff;
  _DAT_00ffdf38 = 0xffff;
  _DAT_00ff0100 = puVar1[2];
  _DAT_00ff0104 = puVar1[3];
  _DAT_00ff0200 = puVar1[4];
  _DAT_00ff0204 = puVar1[5];
  _DAT_00ff0300 = puVar1[6];
  _DAT_00ff0304 = puVar1[7];
  return;
}


