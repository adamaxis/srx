// Ghidra headless export
// Entry: 0x00005662
// Name: FUN_00005662
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061A0 append_entity_sprite_record


void FUN_00005662(void)

{
  byte bVar1;
  int in_A0;
  bool bVar2;
  
  bVar1 = *(byte *)(in_A0 + 2) - 1;
  *(byte *)(in_A0 + 2) = bVar1;
  bVar2 = (short)((ushort)bVar1 << 8) < 0;
  if (bVar2) {
    *(undefined1 *)(in_A0 + 2) = 6;
    *(char *)(in_A0 + 3) = *(char *)(in_A0 + 3) + '\x01';
  }
  if ('\x04' < *(char *)(in_A0 + 3)) {
    *(undefined1 *)(in_A0 + 0xb2) = 0;
    return;
  }
  if (bVar2) {
    append_entity_sprite_record();
  }
  build_title_sprite_records_with_attribute_offset();
  return;
}


