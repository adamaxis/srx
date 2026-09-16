// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000CEC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000cec(void)

{
  short sVar1;
  undefined2 *in_A0;
  undefined2 *puVar2;
  
  _DAT_00ffdd16 = *in_A0;
  _DAT_00ffdd14 = in_A0[1];
  _DAT_00ffdd10 = in_A0[2];
  _DAT_00ffdd12 = in_A0[3];
  sVar1 = 0xe;
  puVar2 = in_A0 + 4;
  do {
    _DAT_00c00004 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


