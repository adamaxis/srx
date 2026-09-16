// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00059BCC


void FUN_00059bcc(void)

{
  short sVar1;
  undefined1 *puVar2;
  int extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 2;
  do {
    puVar2[0x10] = puVar2[0x10] & 0xbf;
    puVar2[0x10] = puVar2[0x10] & 0xef;
    FUN_00003674();
    FUN_000037ec();
    puVar2 = (undefined1 *)(extraout_A0 + 0x100);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ff014c = 10;
  DAT_00ff019c = 10;
  DAT_00ff024c = 10;
  DAT_00ff029c = 10;
  DAT_00ff034c = 10;
  DAT_00ff039c = 10;
  return;
}


