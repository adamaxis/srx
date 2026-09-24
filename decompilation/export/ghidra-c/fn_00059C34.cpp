// Ghidra headless export
// Entry: 0x00059C34
// Name: adjust_runner_source_byte_triplet
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void adjust_runner_source_byte_triplet(void)

{
  char in_D0b;
  
  DAT_00ff018d = in_D0b + DAT_00ff018d;
  DAT_00ff028d = in_D0b + DAT_00ff028d;
  DAT_00ff038d = in_D0b + DAT_00ff038d;
  return;
}


