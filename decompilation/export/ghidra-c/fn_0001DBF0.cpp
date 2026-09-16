// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001DBF0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001dbf0(void)

{
  short sVar1;
  undefined1 *puVar2;
  
  _DAT_00ffdf84 = 0;
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    *(undefined4 *)(puVar2 + 0xa2) = 0;
    *(undefined4 *)(puVar2 + 0xac) = 0;
    puVar2[0x43] = 0;
    puVar2[0xb0] = puVar2[0xb0] & 0xbf;
    puVar2[0x11] = puVar2[0x11] & 0xef;
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


