// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001460E


void FUN_0001460e(void)

{
  ushort uVar1;
  int extraout_A0;
  
  uVar1 = FUN_00015ca4();
  if (uVar1 < 0x1e0) {
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) & 0x80;
    *(undefined1 *)(extraout_A0 + 0xb1) = 0;
    *(byte *)(extraout_A0 + 0xb0) = *(byte *)(extraout_A0 + 0xb0) | 4;
    *(undefined4 *)(extraout_A0 + 0x3a) = 0xb45c;
    return;
  }
  return;
}


