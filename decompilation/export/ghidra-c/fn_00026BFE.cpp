// Ghidra headless export
// Entry: 0x00026BFE
// Name: FUN_00026bfe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined8 FUN_00026bfe(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  ushort unaff_D2w;
  short unaff_D3w;
  ushort *extraout_A0;
  ushort *extraout_A1;
  
  FUN_00000e9c();
  do {
    *extraout_A0 = unaff_D2w | *extraout_A1;
    unaff_D3w = unaff_D3w + -1;
  } while (unaff_D3w != -1);
  return CONCAT44(in_D0,in_D1);
}


