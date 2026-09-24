// Ghidra headless export
// Entry: 0x0005C8C6
// Name: FUN_0005c8c6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined8 FUN_0005c8c6(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  ushort *extraout_A1;
  
  do {
    FUN_00000e9c();
    VDP_DATA = unaff_D2w | *extraout_A1;
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


