// Ghidra headless export
// Entry: 0x0001AD42
// Name: FUN_0001ad42
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ad42(void)

{
  if ((_DAT_00ffe084 != 0) || (_DAT_00ffe0c2 == 0)) {
    return;
  }
  if (DAT_00fff0a3 == '\0') {
    _DAT_00ffe0c2 = 0;
  }
  DAT_00ffe0c6 = DAT_00ffe0c4 + DAT_00ffe0c6;
  DAT_00ffe0b4 = DAT_00ffe0b4 + -1;
  if (DAT_00ffe0b4 < 0) {
    DAT_00ffe0b4 = 3;
    DAT_00ffe0b2 = DAT_00ffe0b2 + 1;
    if (3 < DAT_00ffe0b2) {
      DAT_00ffe0b2 = 0;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  if (DAT_00ffe0b2 != 0) {
    return;
  }
  return;
}


