// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001A310


void FUN_0001a310(void)

{
  short sVar1;
  undefined1 *puVar2;
  
  sVar1 = 0x11;
  puVar2 = &DAT_00ffd86c;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


