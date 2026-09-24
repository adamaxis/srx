// Ghidra headless export
// Entry: 0x0001AC6A
// Name: FUN_0001ac6a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001A818 FUN_0001a818
//   0x0001AC50 arm_effect_after_threshold


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ac6a(void)

{
  if ((_DAT_00ffe084 != 0) || (DAT_00ffe0b8 == 0)) {
    return;
  }
  if ((_DAT_00ffe0c8 != 0) && (DAT_00ffe0ba = DAT_00ffe056 << 2, 0x6aff < DAT_00ffe0ba)) {
    DAT_00ffe0b8 = 0;
    _DAT_00ffe0c8 = 0;
    _DAT_00ffe0f4 = 1;
    FUN_0001a818();
    return;
  }
  if (DAT_00ffe0b8 != 1) {
    DAT_00ffe0ba = DAT_00ffe0c0 + DAT_00ffe0ba;
    arm_effect_after_threshold();
  }
  DAT_00ffe0b4 = DAT_00ffe0b4 + -1;
  if (DAT_00ffe0b4 < 0) {
    DAT_00ffe0b4 = 6;
    DAT_00ffe0b2 = DAT_00ffe0b2 + 1;
    if (3 < DAT_00ffe0b2) {
      DAT_00ffe0b2 = 0;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  return;
}


