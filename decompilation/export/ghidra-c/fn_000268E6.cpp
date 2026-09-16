// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000268E6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000268e6(void)

{
  char unaff_D5b;
  short sVar1;
  undefined2 *puVar2;
  short *psVar3;
  short *unaff_A3;
  
  puVar2 = (undefined2 *)&DAT_00ffb742;
  sVar1 = _DAT_00ffb740;
  do {
    *(char *)(unaff_A3 + 1) = (char)*puVar2;
    unaff_A3[2] = puVar2[1] + 0x393;
    psVar3 = puVar2 + 3;
    *unaff_A3 = _DAT_00ffdd2a + puVar2[2];
    puVar2 = puVar2 + 4;
    unaff_A3[3] = _DAT_00ffdd28 + *psVar3;
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


