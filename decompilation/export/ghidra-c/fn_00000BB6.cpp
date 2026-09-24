// Ghidra headless export
// Entry: 0x00000BB6
// Name: wait_for_even_frame
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void wait_for_even_frame(void)

{
  do {
  } while ((DAT_00fff9ec & 0xfffffffe) == DAT_00fff9f4);
  DAT_00fff9f4 = DAT_00fff9ec & 0xfffffffe;
  return;
}


