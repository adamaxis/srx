// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000187AA


void FUN_000187aa(void)

{
  short sVar1;
  short sVar2;
  
  sVar2 = 0;
  sVar1 = 0xb;
  do {
    if ((&DAT_00fffbcc)[sVar2] != '\0') {
      FUN_00001232();
    }
    sVar2 = sVar2 + 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar2 = 3;
  do {
    FUN_00001232();
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


