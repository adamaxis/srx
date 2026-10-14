// Ghidra headless export
// Entry: 0x00011190
// Name: transfer_menu_glyph_triple
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 transfer_menu_glyph_triple(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D6w;
  short sVar1;
  short *extraout_A1;
  short *psVar2;
  
  do {
    sVar1 = 2;
    make_vdp_transfer_command();
    psVar2 = extraout_A1;
    do {
      VDP_DATA = unaff_D6w | *psVar2 + 0x3e9U;
      sVar1 = sVar1 + -1;
      psVar2 = psVar2 + 1;
    } while (sVar1 != -1);
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


