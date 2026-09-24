// Ghidra headless export
// Entry: 0x00056632
// Name: FUN_00056632
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


int FUN_00056632(void)

{
  int iVar1;
  short unaff_D4w;
  
  if ((short)(unaff_D4w + -2) < 0) {
    unaff_D4w = 2;
  }
  if ((ushort)(7U - unaff_D4w) == 0 || 7 < unaff_D4w) {
    iVar1 = 10;
  }
  else {
    iVar1 = (uint)(ushort)(7U - unaff_D4w) * 0x14;
  }
  return iVar1;
}


