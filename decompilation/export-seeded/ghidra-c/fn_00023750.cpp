// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00023750


char FUN_00023750(void)

{
  char cVar1;
  short sVar2;
  undefined1 *puVar3;
  
  DAT_00fff0d6 = 0;
  cVar1 = '\0';
  puVar3 = &DAT_00ff0100;
  sVar2 = 2;
  do {
    if ((puVar3[0x10] & 1) != 0) {
      cVar1 = cVar1 + '\x01';
    }
    puVar3 = puVar3 + 0x100;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return cVar1;
}


