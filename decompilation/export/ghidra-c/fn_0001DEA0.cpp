// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001DEA0


void FUN_0001dea0(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 9;
  do {
    if ((((puVar2[0x10] & 0x40) != 0) && ((puVar2[0x10] & 8) == 0)) &&
       (*(short *)(puVar2 + 0xc) == 0)) {
      FUN_0001e514();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


