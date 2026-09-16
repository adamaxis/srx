// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000D7EA


void FUN_0000d7ea(void)

{
  uint uVar1;
  undefined4 unaff_D5;
  short sVar2;
  int in_A0;
  
  sVar2 = 2;
  uVar1 = 0xc;
  do {
    if ((CONCAT22((short)((uint)unaff_D5 >> 0x10),*(undefined2 *)(in_A0 + 0x90)) &
        1 << (uVar1 & 0x1f)) != 0) break;
    uVar1 = (uint)(ushort)((short)uVar1 + 1);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  FUN_0000d81c();
  return;
}


