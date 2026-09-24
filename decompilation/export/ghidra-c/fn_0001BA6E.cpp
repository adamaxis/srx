// Ghidra headless export
// Entry: 0x0001BA6E
// Name: FUN_0001ba6e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ba6e(void)

{
  DAT_00ffe0b0 = DAT_00ffe0b0 + -1;
  if (DAT_00ffe0b0 < 0) {
    DAT_00ffe0b0 = 4;
    DAT_00ffe0ae = DAT_00ffe0ae + 1;
    if (3 < DAT_00ffe0ae) {
      _DAT_00ffe0ac = 0;
      return;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  return;
}


