// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000531EA


void FUN_000531ea(void)

{
  short sVar1;
  undefined4 *extraout_A0;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  FUN_00053216();
  sVar1 = 0x9f;
  puVar2 = extraout_A0;
  puVar3 = (undefined4 *)&DAT_00ff1100;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


