// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000313A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000313a(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 0xe0f;
  puVar2 = _DAT_00ffddda;
  puVar3 = (undefined2 *)&DAT_00ff2700;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  sVar1 = 0xe0f;
  puVar2 = _DAT_00ffdde2;
  puVar3 = (undefined2 *)&DAT_00ff4700;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


