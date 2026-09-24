// Ghidra headless export
// Entry: 0x00003648
// Name: draw_hud_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void draw_hud_panel(void)

{
  short extraout_D1w;
  ushort unaff_D2w;
  ushort unaff_D5w;
  ushort uVar1;
  short unaff_D7w;
  short sVar2;
  
  uVar1 = unaff_D2w | unaff_D5w;
  do {
    make_vdp_transfer_command();
    sVar2 = unaff_D7w;
    do {
      VDP_DATA = uVar1;
      uVar1 = uVar1 + 1;
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  } while (extraout_D1w != 0);
  return;
}


