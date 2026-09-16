// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000196F2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000196f2(void)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar5;
  undefined2 *puVar4;
  
  uVar1 = _DAT_00ffd8ae;
  DAT_00fff10a = DAT_00fff10a + -1;
  if (DAT_00fff10a < '\0') {
    DAT_00fff10a = '\x04';
    sVar2 = 2;
    puVar3 = (undefined2 *)&DAT_00ffd8ae;
    puVar5 = (undefined2 *)&DAT_00ffd8b0;
    do {
      puVar4 = puVar3 + 1;
      *puVar3 = *puVar5;
      sVar2 = sVar2 + -1;
      puVar3 = puVar4;
      puVar5 = puVar5 + 1;
    } while (sVar2 != -1);
    *puVar4 = uVar1;
    DAT_00ffef14 = DAT_00ffef14 | 1;
  }
  return;
}


