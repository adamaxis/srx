// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C0C2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c0c2(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  _DAT_00ffe0aa = _DAT_00ffe0aa - 1;
  if (_DAT_00ffe0aa != 0) {
    puVar2 = &DAT_0001c126;
    if ((_DAT_00ffe0aa & 1) != 0) {
      puVar2 = &DAT_0001c128;
    }
    sVar1 = 6;
    puVar3 = (undefined2 *)&DAT_00ffd880;
    do {
      *puVar3 = *puVar2;
      sVar1 = sVar1 + -1;
      puVar2 = puVar2 + 1;
      puVar3 = puVar3 + 1;
    } while (sVar1 != -1);
    DAT_00ffef14 = DAT_00ffef14 | 1;
    return;
  }
  FUN_00019de8();
  return;
}


