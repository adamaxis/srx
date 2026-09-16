// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005DAE8


void FUN_0005dae8(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0x1bf;
  puVar2 = (undefined2 *)&DAT_00ffb740;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  FUN_0005dafc();
  return;
}


