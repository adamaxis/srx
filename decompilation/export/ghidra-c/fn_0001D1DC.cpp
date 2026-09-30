// Ghidra headless export
// Entry: 0x0001D1DC
// Name: transfer_world_word_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C write_vdp_word_block_with_source_wrap

// The native conversion names the three register inputs loaded here from
// FFDF7E, FFDF76, and FFE006 before crossing the host wrapped-transfer
// boundary; it does not leave those values as implicit machine globals.

void transfer_world_word_block(void)

{
  write_vdp_word_block_with_source_wrap();
  return;
}


