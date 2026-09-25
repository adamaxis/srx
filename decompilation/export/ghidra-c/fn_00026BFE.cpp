// Ghidra headless export
// Entry: 0x00026BFE
// Name: write_masked_transfer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 write_masked_transfer(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  ushort unaff_D2w;
  short unaff_D3w;
  ushort *extraout_A0;
  ushort *extraout_A1;
  
  make_vdp_transfer_command();
  do {
    *extraout_A0 = unaff_D2w | *extraout_A1;
    unaff_D3w = unaff_D3w + -1;
  } while (unaff_D3w != -1);
  return CONCAT44(in_D0,in_D1);
}


