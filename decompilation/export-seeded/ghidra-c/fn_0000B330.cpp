// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B330


void FUN_0000b330(void)

{
  ushort uVar1;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  
  *(byte *)(in_A0 + 0x10) = *(byte *)(in_A0 + 0x10) | 0x40;
  FUN_0000abb6();
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xbf;
  FUN_000530de();
  *(byte *)(extraout_A0_00 + 0x10) = *(byte *)(extraout_A0_00 + 0x10) | 0x10;
  *(undefined1 **)(extraout_A0_00 + 0x3a) = &LAB_0000b38a;
  uVar1 = *(ushort *)(extraout_A0_00 + 0x2e) << 3 | *(ushort *)(extraout_A0_00 + 0x2e) >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(extraout_A0_00 + 0x16) =
       *(undefined2 *)(&LAB_0000c0b2 + (short)((uVar1 & 7) << 1));
  FUN_0000af6c();
  return;
}


