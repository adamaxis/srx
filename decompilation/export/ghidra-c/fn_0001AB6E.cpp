// Ghidra headless export
// Entry: 0x0001AB6E
// Name: advance_primary_world_effect_overlay
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001A818 begin_world_effect_descriptor_transition
//   0x0001AC50 arm_effect_after_threshold
//   0x000563CC advance_primary_world_effect_pending_roll
//
// The primary phase compare is CMP.W #0x1A followed by BCS, so phase values
// 0 through 0x19 are valid and a post-increment value of 0x1A wraps to zero.
// Its sprite list is the 0xF1BDE table, with D1=FFE0BE>>6, D2=0x00B9,
// D4=0x07A8, and D6=0x2000.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_primary_world_effect_overlay(void)

{
  bool bVar1;
  
  if ((_DAT_00ffe084 != 0) || (DAT_00ffe0b6 == 0)) {
    return;
  }
  if ((_DAT_00ffe0c8 != 0) && (DAT_00ffe0be = DAT_00ffe056 << 2, 0x6aff < DAT_00ffe0be)) {
    _DAT_00ffe0c8 = 0;
    DAT_00ffe0b6 = 0;
    _DAT_00ffe0f4 = 1;
    begin_world_effect_descriptor_transition();
    return;
  }
  if (DAT_00ffe0b6 != 1) {
    DAT_00ffe0be = DAT_00ffe0c0 + DAT_00ffe0be;
    bVar1 = false;
    arm_effect_after_threshold();
    if (!bVar1) {
      advance_primary_world_effect_pending_roll();
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

