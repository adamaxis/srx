// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001957E


void FUN_0001957e(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *extraout_A1;
  
  puVar2 = &DAT_00fffbbe;
  sVar1 = 4;
  do {
    if (-1 < (char)puVar2[sVar1 + 0x1a]) {
      FUN_0001952c();
      puVar2 = extraout_A1;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


