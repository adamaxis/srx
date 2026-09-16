// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00005622


void FUN_00005622(void)

{
  short sVar1;
  byte in_D1b;
  undefined1 *puVar2;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    puVar2[0xde] = 0;
    puVar2[0xdf] = in_D1b & puVar2[0xdf];
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


