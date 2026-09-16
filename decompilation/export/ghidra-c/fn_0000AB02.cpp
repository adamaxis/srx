// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AB02


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ab02(void)

{
  short sVar1;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int iVar2;
  
  if ((*(byte *)(in_A0 + 0xb0) & 0x80) != 0) {
    _DAT_00ffdd2c = &DAT_0001f210;
    FUN_0001e2ec();
  }
  if (((((*(byte *)(in_A0 + 0x10) & 8) == 0) &&
       (FUN_00016b98(), in_A0 = extraout_A0, (*(byte *)(extraout_A0 + 0x9e) & 0x40) != 0)) &&
      (DAT_00ffe177 = DAT_00ffe177 + -1, DAT_00ffe177 == '\0')) &&
     (FUN_0001dbf0(), in_A0 = extraout_A0_00, _DAT_00ffe12e != (undefined *)0x0)) {
    _DAT_00ffdd2c = _DAT_00ffe12e;
    FUN_0001e2ec();
    _DAT_00ffe12e = (undefined *)0x0;
    in_A0 = extraout_A0_01;
  }
  *(byte *)(in_A0 + 0x10) = *(byte *)(in_A0 + 0x10) | 0x40;
  if ((*(byte *)(in_A0 + 0x10) & 0x20) != 0) {
    iVar2 = 0xff0400;
    sVar1 = 6;
    do {
      if (((*(byte *)(iVar2 + 0x10) & 1) != 0) && (in_A0 == *(int *)(iVar2 + 0xa8))) {
        *(byte *)(iVar2 + 0x10) = *(byte *)(iVar2 + 0x10) | 0x20;
      }
      iVar2 = iVar2 + 0x100;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  }
  if ((*(byte *)(in_A0 + 0x10) & 8) != 0) {
    if ((((DAT_00ff0110 & 0x40) != 0) && (((DAT_00ff0210 & 1) == 0 || ((DAT_00ff0210 & 0x40) != 0)))
        ) && (((DAT_00ff0310 & 1) == 0 || ((DAT_00ff0310 & 0x40) != 0)))) {
      _DAT_00ffe166 = 0x3c;
    }
    return;
  }
  return;
}


