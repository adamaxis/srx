// Ghidra headless export
// Entry: 0x00026C26
// Name: FUN_00026c26
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 FUN_00026c26(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  ushort unaff_D2w;
  short unaff_D3w;
  ushort *extraout_A0;
  ushort *extraout_A1;
  
  VDP_CTRL = 0x8f80;
  make_vdp_transfer_command();
  do {
    *extraout_A0 = unaff_D2w | *extraout_A1;
    unaff_D3w = unaff_D3w + -1;
  } while (unaff_D3w != -1);
  VDP_CTRL = 0x8f02;
  return CONCAT44(in_D0,in_D1);
}


