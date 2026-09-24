// Ghidra headless export
// Entry: 0x00017998
// Name: FUN_00017998
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216
//   0x00017C3C FUN_00017c3c


void FUN_00017998(void)

{
  short unaff_D3w;
  undefined1 uVar1;
  
  uVar1 = 0;
  FUN_0000c0da();
  FUN_00017c3c();
  if (!(bool)uVar1) {
    FUN_0000c0da();
    if (-1 < (short)(unaff_D3w + -3)) {
      FUN_0000c0da();
    }
    if (-1 < (short)(unaff_D3w + -2)) {
      FUN_0000c216();
      if (-1 < (short)(unaff_D3w + -3)) {
        return;
      }
      goto LAB_00017a46;
    }
  }
  FUN_0000c0da();
LAB_00017a46:
  FUN_0000c0da();
  return;
}


