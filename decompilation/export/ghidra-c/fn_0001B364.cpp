// Ghidra headless export
// Entry: 0x0001B364
// Name: advance_title_figure_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001C3FA advance_title_sprite_layers

// The four source sprite requests are independent: 0x1DAF4/0x1DB08 build
// key 0x780 at (0xF8,0xE4)/(0x148,0xE4), while 0xEA490/0xEA4A4 build key
// 0x7C0 at (0xE2,0x114)/(0x15E,0x114). Each five-entry table contributes its
// selected pointer plus eight bytes to the attribute-offset builder, with
// D6=0x6000. The typed conversion retains all four table and placement
// boundaries.


void advance_title_figure_animation(void)

{
  advance_title_sprite_layers();
  DAT_00ffe0b0 = DAT_00ffe0b0 + -1;
  if (DAT_00ffe0b0 < 0) {
    DAT_00ffe0b0 = 0;
    DAT_00ffe0ae = DAT_00ffe0ae + 1;
    if (4 < DAT_00ffe0ae) {
      DAT_00ffe0ae = 0;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  build_title_sprite_records_with_attribute_offset();
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  build_title_sprite_records_with_attribute_offset();
  return;
}

