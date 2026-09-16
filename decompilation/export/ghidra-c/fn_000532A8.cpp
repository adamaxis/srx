// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000532A8


void FUN_000532a8(void)

{
  short sVar1;
  undefined4 *puVar2;
  
  sVar1 = 0x22;
  puVar2 = (undefined4 *)&DAT_00fffaa4;
  do {
    *puVar2 = 0xffffffff;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


