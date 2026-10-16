// Ghidra headless export
// Entry: 0x00000F1C
// Name: write_transfer_header
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void write_transfer_header(void)

{
  make_vdp_transfer_command();
  VDP_DATA = DAT_00ffdd08;
  VDP_DATA = DAT_00ffdd0c;
  return;
}


