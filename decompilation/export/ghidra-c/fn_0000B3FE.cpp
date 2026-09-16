// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B3FE


void FUN_0000b3fe(void)

{
  ushort in_D0w;
  ushort uVar1;
  int in_A0;
  
  *(ushort *)(in_A0 + 0x2e) = in_D0w;
  *(undefined1 **)(in_A0 + 0x3a) = &LAB_0000b432;
  uVar1 = in_D0w << 3 | in_D0w >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(in_A0 + 0x16) = *(undefined2 *)(&DAT_0000c092 + (short)((uVar1 & 7) << 1));
  FUN_0000af6c();
  FUN_0000abfc();
  return;
}


