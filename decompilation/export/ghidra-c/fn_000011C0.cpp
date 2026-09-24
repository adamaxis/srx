// Ghidra headless export
// Entry: 0x000011C0
// Name: write_repeated_vdp_grid
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 write_repeated_vdp_grid(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  short unaff_D7w;
  short sVar1;
  ushort *extraout_A1;
  
  do {
    make_vdp_transfer_command();
    sVar1 = unaff_D7w;
    do {
      VDP_DATA = unaff_D2w | *extraout_A1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


