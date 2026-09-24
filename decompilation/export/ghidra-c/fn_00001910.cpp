// Ghidra headless export
// Entry: 0x00001910
// Name: write_alternating_word_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined4 write_alternating_word_pair(void)

{
  byte bVar1;
  undefined4 in_D0;
  word *pwVar2;
  
  Ram00c00004 = 0xc05e0000;
  DAT_00fff9f0 = DAT_00fff9f0 + 1;
  if ((DAT_00fff9f0 & 1) == 0) {
    pwVar2 = &DAT_00ffd8dc;
  }
  else {
    pwVar2 = &DAT_00ffd8bc;
  }
  do {
    bVar1 = VDP_CTRL._1_1_;
  } while ((bVar1 & 4) == 0);
  VDP_DATA = *pwVar2;
  VDP_DATA = pwVar2[1];
  return in_D0;
}


