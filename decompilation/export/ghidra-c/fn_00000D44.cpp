// Ghidra headless export
// Entry: 0x00000D44
// Name: advance_source_prng
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void advance_source_prng(void)

{
  DAT_00fff9f8 = DAT_00fff9f8 * 0x20e5 + 1;
  return;
}


