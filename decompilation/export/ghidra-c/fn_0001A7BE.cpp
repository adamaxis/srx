// Ghidra headless export
// Entry: 0x0001A7BE
// Name: append_title_vertical_sprite
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//
// Direct ROM trace:
//   select ((FFDF88 >> 1) & 3) + 8 from the 0xF1AEE pointer table;
//   append key 0x06E0; add 0x60 to FFE07A; continue only while the unsigned
//   word is below 0x1280; build with D1=FFE078>>4, D2=FFE07A>>4,
//   D4=0x06E0, and D6=0xA000 (the 16-bit form of -0x6000).
// The typed host request is ghidra_conversion::TitleVerticalSpriteBuildRequest.


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


