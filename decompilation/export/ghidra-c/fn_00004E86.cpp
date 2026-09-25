// Ghidra headless export
// Entry: 0x00004E86
// Name: convert_field_routine_0x00004e86
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061A0 append_entity_sprite_record
//   0x0001A304 clear_effect_slot


void convert_field_routine_0x00004e86(void)

{
  char cVar1;
  int in_A0;
  int extraout_A0;
  
  cVar1 = *(char *)(in_A0 + 2) + -1;
  *(char *)(in_A0 + 2) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(in_A0 + 2) = 6;
    *(char *)(in_A0 + 3) = *(char *)(in_A0 + 3) + '\x01';
    if ('\x03' < *(char *)(in_A0 + 3)) {
      clear_effect_slot();
      *(byte *)(extraout_A0 + 0x11) = *(byte *)(extraout_A0 + 0x11) & 0xbf;
      return;
    }
  }
  append_entity_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  return;
}


