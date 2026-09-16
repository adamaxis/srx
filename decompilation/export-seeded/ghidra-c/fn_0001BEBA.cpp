// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001BEBA


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001beba(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 6;
  puVar2 = (undefined2 *)((int)&DAT_0001c10a + (int)(short)((_DAT_00ffdf88 & 7) << 1));
  puVar3 = (undefined2 *)&DAT_00ffd880;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  _DAT_00ffe00c = _DAT_00ffe00c + -1;
  if (_DAT_00ffe00c < 0) {
    FUN_0001bf5c();
    DAT_00ff1592 = DAT_00ff1592 & 0x7f;
    FUN_0001b612();
    _DAT_00ffe0d6 = 0;
    FUN_00019d66();
    FUN_0001a908();
    return;
  }
  FUN_00001378(*(undefined4 *)(&DAT_0001c136 + (short)(((ushort)-_DAT_00ffe00c >> 1 & 3) << 2)));
  FUN_00001378();
  return;
}


