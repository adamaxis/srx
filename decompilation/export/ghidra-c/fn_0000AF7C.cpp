// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AF7C


void FUN_0000af7c(void)

{
  ushort uVar1;
  int extraout_A0;
  
  FUN_0000abfc();
  *(undefined **)(extraout_A0 + 0x3a) = &DAT_0000afe0;
  uVar1 = *(ushort *)(extraout_A0 + 0x2e) << 3 | *(ushort *)(extraout_A0 + 0x2e) >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(extraout_A0 + 0x16) = *(undefined2 *)(&DAT_0000c082 + (short)((uVar1 & 7) << 1));
  FUN_0000af6c();
  return;
}


