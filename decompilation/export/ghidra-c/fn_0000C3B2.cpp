// Ghidra headless export
// Entry: 0x0000C3B2
// Name: FUN_0000c3b2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


void FUN_0000c3b2(void)

{
  word extraout_D1w;
  
  make_vdp_transfer_command();
  VDP_DATA = extraout_D1w;
  return;
}


