// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001B5A6


void FUN_0001b5a6(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0x1bf;
  puVar2 = (undefined2 *)&DAT_00ffcb40;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  FUN_0001b56e();
  return;
}


