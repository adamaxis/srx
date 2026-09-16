// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B530


void FUN_0000b530(void)

{
  ushort uVar1;
  int in_A0;
  
  uVar1 = *(short *)(in_A0 + 0x2e) - *(short *)(in_A0 + 0x34);
  if (*(short *)(in_A0 + 0x2e) != *(short *)(in_A0 + 0x34)) {
    if ((short)uVar1 < 0) {
      if (uVar1 < 0xf800) {
        *(short *)(in_A0 + 0x2e) = *(short *)(in_A0 + 0x2e) + 0x800;
        return;
      }
    }
    else if (0x7ff < uVar1) {
      *(short *)(in_A0 + 0x2e) = *(short *)(in_A0 + 0x2e) + -0x800;
      return;
    }
    *(undefined2 *)(in_A0 + 0x2e) = *(undefined2 *)(in_A0 + 0x34);
  }
  return;
}


