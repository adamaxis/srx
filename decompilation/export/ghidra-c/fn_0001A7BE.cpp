// Ghidra headless export
// Entry: 0x0001A7BE
// Name: append_title_vertical_sprite
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record


void append_title_vertical_sprite(void)

{
  append_sprite_record();
  DAT_00ffe07a = DAT_00ffe07a + 0x60;
  if (0x127f < DAT_00ffe07a) {
    return;
  }
  build_title_sprite_records_with_attribute_offset();
  return;
}


