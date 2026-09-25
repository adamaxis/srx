// Ghidra headless export
// Entry: 0x0001B218
// Name: advance_title_phase_cycle
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record


void advance_title_phase_cycle(void)

{
  DAT_00ffe0fa = DAT_00ffe0fa + -1;
  if (DAT_00ffe0fa < 0) {
    DAT_00ffe0fa = 3;
    DAT_00ffe0f8 = DAT_00ffe0f8 + 1;
    if (5 < DAT_00ffe0f8) {
      DAT_00ffe0f8 = 0;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  return;
}


