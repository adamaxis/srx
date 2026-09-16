// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055EB4


undefined1 FUN_00055eb4(void)

{
  undefined1 uVar1;
  int in_A1;
  
  uVar1 = 0;
  if (*(byte *)(in_A1 + 0x76) != 0) {
    uVar1 = (&DAT_000d4d76)[(short)((*(byte *)(in_A1 + 0x76) - 1) * 0x30)];
  }
  return uVar1;
}


