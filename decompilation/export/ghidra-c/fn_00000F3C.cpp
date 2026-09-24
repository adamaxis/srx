// Ghidra headless export
// Entry: 0x00000F3C
// Name: write_fixed_transfer_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void write_fixed_transfer_pair(void)

{
  Ram00c00004 = 0x40000010;
  VDP_DATA = DAT_00ffdd0a;
  VDP_DATA = DAT_00ffdd0e;
  return;
}


