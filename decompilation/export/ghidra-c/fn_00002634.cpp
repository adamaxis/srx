// Ghidra headless export
// Entry: 0x00002634
// Name: FUN_00002634
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined2 FUN_00002634(void)

{
  undefined2 in_D0w;
  short sVar1;
  
  if (DAT_00ffdd5a != 2) {
    return in_D0w;
  }
  DAT_00ffdf90 = DAT_00ffdf90 + -1;
  if (DAT_00ffdf90 < 0) {
    DAT_00ffdf90 = 4;
    DAT_00ffdf92 = DAT_00ffdf92 + 1;
    if (6 < DAT_00ffdf92) {
      DAT_00ffdf92 = 0;
    }
    sVar1 = DAT_00ffdf92 << 1;
    VDP_CTRL = 0xc050;
    VDP_CTRL = 0;
    VDP_DATA = *(word *)((int)&DAT_000026f2 + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026f4 + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026f6 + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026f8 + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026fa + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026fc + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_000026fe + (int)sVar1);
    VDP_DATA = *(word *)((int)&DAT_00002700 + (int)sVar1);
    return 0xc050;
  }
  return 0xc050;
}


