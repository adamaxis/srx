// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001047E


void FUN_0001047e(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 2;
  do {
    if (((puVar2[0x10] & 1) != 0) && ((char)puVar2[0x9b] < '\x02')) {
      FUN_00055f02();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


