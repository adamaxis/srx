// Ghidra headless export
// Entry: 0x00000F1C
// Name: FUN_00000f1c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void FUN_00000f1c(void)

{
  make_vdp_transfer_command();
  VDP_DATA = DAT_00ffdd08;
  VDP_DATA = DAT_00ffdd0c;
  return;
}


