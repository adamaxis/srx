// Ghidra headless export
// Entry: 0x00000E9C
// Name: make_vdp_transfer_command
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void make_vdp_transfer_command(void)

{
  ushort in_D0w;
  
  VDP_CTRL = in_D0w & 0x3fff | 0x4000;
  VDP_CTRL = in_D0w >> 0xe;
  return;
}


