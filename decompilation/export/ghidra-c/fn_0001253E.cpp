// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001253E


void FUN_0001253e(void)

{
  int extraout_A0;
  int in_A1;
  
  if ((*(char *)(in_A1 + 0x4c) == '\n') && (*(char *)(in_A1 + 0x9c) == '\n')) {
    return;
  }
  FUN_00000dba();
  FUN_00003674();
  FUN_000037ec();
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xbf;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xef;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) | 1;
  *(undefined4 *)(extraout_A0 + 0x3a) = 0xb45c;
  return;
}


