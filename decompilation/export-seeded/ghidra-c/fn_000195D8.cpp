// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000195D8


void FUN_000195d8(void)

{
  short extraout_D1w;
  ushort uVar1;
  undefined1 unaff_D7b;
  int extraout_A1;
  
  FUN_000195a6();
  uVar1 = (ushort)DAT_00fff0d4;
  *(undefined1 *)(extraout_A1 + 0x1a + (int)(short)uVar1) = unaff_D7b;
  *(undefined1 *)((short)uVar1 + 0xfff0e8) = 0;
  *(short *)(&DAT_00fff0f2 + (short)(uVar1 << 1)) = extraout_D1w * 10;
  FUN_00001232();
  FUN_0001792e();
  return;
}


