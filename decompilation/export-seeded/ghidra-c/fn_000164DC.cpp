// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000164DC


void FUN_000164dc(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  puVar2 = (undefined2 *)&DAT_00ffd740;
  sVar1 = 9;
  do {
    *puVar2 = 0;
    puVar2 = puVar2 + 0xf;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


