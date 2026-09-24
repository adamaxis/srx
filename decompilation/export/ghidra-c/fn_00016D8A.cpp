// Ghidra headless export
// Entry: 0x00016D8A
// Name: FUN_00016d8a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016E08 all_scene_record_slots_active


void FUN_00016d8a(void)

{
  short unaff_D6w;
  short *extraout_A0;
  short *extraout_A1;
  undefined1 in_CF;
  
  all_scene_record_slots_active();
  if ((bool)in_CF) {
    return;
  }
  *extraout_A1 = *extraout_A0 + 0x80;
  extraout_A1[2] = extraout_A0[2] + 0x80;
  *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 1;
  extraout_A1[1] = unaff_D6w;
  return;
}


