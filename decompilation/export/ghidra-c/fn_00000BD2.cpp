// Ghidra headless export
// Entry: 0x00000BD2
// Name: wait_for_vblank
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void wait_for_vblank(void)

{
  DAT_00ffdf88 = DAT_00ffdf88 + 1;
  do {
  } while (DAT_00fff9f4._2_2_ == DAT_00fff9ec._2_2_);
  DAT_00fff9f4._2_2_ = DAT_00fff9ec._2_2_;
  return;
}


