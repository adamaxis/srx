// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00005B7E


void FUN_00005b7e(void)

{
  short sVar1;
  undefined1 *puVar2;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 0xf;
  do {
    puVar2[0x11] = puVar2[0x11] | 1;
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


