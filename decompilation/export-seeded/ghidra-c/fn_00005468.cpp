// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00005468


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00005468(void)

{
  int in_A0;
  int in_A1;
  
  _DAT_00ffe004 = 4;
  *(undefined1 *)(in_A1 + 0xb2) = 0xd;
  *(undefined2 *)(in_A1 + 0x32) = *(undefined2 *)(in_A0 + 0x32);
  *(undefined2 *)(in_A1 + 0x2a) = *(undefined2 *)(in_A0 + 0x2a);
  *(undefined1 *)(in_A1 + 2) = 5;
  *(undefined1 *)(in_A1 + 3) = 0;
  *(undefined4 *)(in_A1 + 0xd8) = *(undefined4 *)(in_A0 + 0xd8);
  return;
}


