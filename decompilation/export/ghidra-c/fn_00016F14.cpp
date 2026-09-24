// Ghidra headless export
// Entry: 0x00016F14
// Name: FUN_00016f14
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016F3C FUN_00016f3c


void FUN_00016f14(void)

{
  short in_D0w;
  short sVar1;
  ushort extraout_D1w;
  short unaff_D2w;
  ushort unaff_D5w;
  ushort unaff_D6w;
  int unaff_A2;
  bool bVar2;
  ushort uStack_6;
  
  if (unaff_D2w < 0) {
    bVar2 = false;
    FUN_00016f3c();
    if (bVar2) {
      return;
    }
    FUN_00016f3c();
    return;
  }
  sVar1 = FUN_00014640();
  if ((((extraout_D1w <= unaff_D5w) && ((ushort)(sVar1 + 0x200U) <= unaff_D6w)) &&
      (unaff_D5w < (ushort)(extraout_D1w + ((in_D0w + 1U) * 0x200 | (ushort)(in_D0w + 1U) >> 7))))
     && (unaff_D6w < (ushort)(sVar1 + 0x200U + (uStack_6 << 9 | uStack_6 >> 7)))) {
    *(undefined1 *)(unaff_A2 + -1) = 0xff;
    return;
  }
  return;
}


