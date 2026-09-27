// Ghidra headless export
// Entry: 0x0000ABFC
// Name: update_entity_animation_delay
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000D7EA compute_entity_delay_factor
// Behavior annotation from the ROM disassembly:
//   The entry preserves D1/D2, calls 0xD7EA for the factor in D1, computes
//   0x6A - factor*4, scales for D0==2 or other nonzero D0, observes the
//   no-op bit-1 test at A0+0x10, clamps to 0x0F, and stores the result at
//   A0+0xDC. The typed conversion supplies record fields instead of A0.


void update_entity_animation_delay(void)

{
  short sVar1;
  short extraout_D1w;
  short sVar2;
  int extraout_A0;
  
  sVar1 = compute_entity_delay_factor();
  sVar2 = extraout_D1w * -4 + 0x6a;
  if (sVar1 != 0) {
    if (sVar1 == 2) {
      sVar2 = sVar2 * 2;
    }
    else {
      sVar2 = sVar2 >> 1;
    }
  }
  if (sVar2 < 0xf) {
    sVar2 = 0xf;
  }
  *(short *)(extraout_A0 + 0xdc) = sVar2;
  return;
}


