// Ghidra headless export
// Entry: 0x0000073A
// Name: wait_for_vdp_ready
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void wait_for_vdp_ready(void)

{
  byte bVar1;
  short unaff_D6w;
  short sVar2;
  
  do {
    sVar2 = 0xfff;
    do {
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    unaff_D6w = unaff_D6w + -1;
  } while (unaff_D6w != -1);
  do {
    bVar1 = VDP_CTRL._1_1_;
  } while ((bVar1 & 8) == 0);
  sVar2 = 0xff;
  do {
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


