// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014446


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014446(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar2 = &DAT_00ff0100;
  _DAT_00ffdfc8 = 0;
  sVar1 = 2;
  puVar3 = (undefined4 *)&DAT_00ffdfca;
  do {
    puVar4 = puVar3;
    if (((((puVar2[0x10] & 1) != 0) && ((puVar2[0x10] & 0x40) == 0)) && ((puVar2[0x10] & 0x10) == 0)
        ) && (*(short *)(puVar2 + 0xc) != 0)) {
      _DAT_00ffdfc8 = _DAT_00ffdfc8 + 1;
      puVar4 = puVar3 + 1;
      *puVar3 = puVar2;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    puVar3 = puVar4;
  } while (sVar1 != -1);
  return;
}


