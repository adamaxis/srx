// Ghidra headless export
// Entry: 0x00016F3C
// Name: FUN_00016f3c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014640 FUN_00014640


void FUN_00016f3c(void)

{
  short in_D0w;
  short sVar1;
  ushort extraout_D1w;
  ushort unaff_D5w;
  ushort unaff_D6w;
  int unaff_A2;
  undefined2 uStack_6;
  
  sVar1 = FUN_00014640();
  if ((((extraout_D1w <= unaff_D5w) && ((ushort)(sVar1 + 0x200U) <= unaff_D6w)) &&
      (unaff_D5w < (ushort)(extraout_D1w + ((in_D0w + 1U) * 0x200 | (ushort)(in_D0w + 1U) >> 7))))
     && (unaff_D6w < (ushort)(sVar1 + 0x200U + (uStack_6 << 9 | uStack_6 >> 7)))) {
    *(undefined1 *)(unaff_A2 + -1) = 0xff;
    return;
  }
  return;
}


