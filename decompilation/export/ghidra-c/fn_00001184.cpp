// Ghidra headless export
// Entry: 0x00001184
// Name: write_masked_vdp_row
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 write_masked_vdp_row(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  ushort unaff_D2w;
  short unaff_D3w;
  ushort *extraout_A0;
  ushort *extraout_A1;
  ushort *puVar1;
  
  if ((short)in_D1 != 0) {
    VDP_CTRL = 0x8f80;
  }
  make_vdp_transfer_command();
  puVar1 = extraout_A1;
  do {
    *extraout_A0 = unaff_D2w | *puVar1;
    unaff_D3w = unaff_D3w + -1;
    puVar1 = puVar1 + 1;
  } while (unaff_D3w != -1);
  VDP_CTRL = 0x8f02;
  return CONCAT44(in_D0,in_D1);
}


