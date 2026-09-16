// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000566D8


void FUN_000566d8(void)

{
  short sVar1;
  undefined4 *in_A0;
  undefined4 *puVar2;
  
  sVar1 = 7;
  puVar2 = (undefined4 *)&DAT_00ffd87e;
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


