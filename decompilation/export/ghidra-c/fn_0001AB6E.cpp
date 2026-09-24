// Ghidra headless export
// Entry: 0x0001AB6E
// Name: FUN_0001ab6e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001A818 FUN_0001a818
//   0x0001AC50 arm_effect_after_threshold
//   0x000563CC FUN_000563cc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ab6e(void)

{
  bool bVar1;
  
  if ((_DAT_00ffe084 != 0) || (DAT_00ffe0b6 == 0)) {
    return;
  }
  if ((_DAT_00ffe0c8 != 0) && (DAT_00ffe0be = DAT_00ffe056 << 2, 0x6aff < DAT_00ffe0be)) {
    _DAT_00ffe0c8 = 0;
    DAT_00ffe0b6 = 0;
    _DAT_00ffe0f4 = 1;
    FUN_0001a818();
    return;
  }
  if (DAT_00ffe0b6 != 1) {
    DAT_00ffe0be = DAT_00ffe0c0 + DAT_00ffe0be;
    bVar1 = false;
    arm_effect_after_threshold();
    if (!bVar1) {
      FUN_000563cc();
      return;
    }
  }
  DAT_00ffe0b4 = DAT_00ffe0b4 + -1;
  if (DAT_00ffe0b4 < 0) {
    DAT_00ffe0b4 = 8;
    DAT_00ffe0b2 = DAT_00ffe0b2 + 1;
    if (0x19 < DAT_00ffe0b2) {
      DAT_00ffe0b2 = 0;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  return;
}


