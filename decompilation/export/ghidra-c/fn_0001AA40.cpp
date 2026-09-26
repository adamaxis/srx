// Ghidra headless export
// Entry: 0x0001AA40
// Name: tick_world_effect_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061CC append_sprite_record
//   0x0001A4EA lookup_runner_animation_delta
//   0x0001A8F4 reset_effect_entity
//   0x0001AB6E advance_primary_world_effect_overlay
//   0x0001AC6A advance_secondary_world_effect_overlay
//   0x0001AD42 advance_background_world_effect_overlay


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void tick_world_effect_animation(void)

{
  char cVar1;
  byte bVar2;
  char extraout_D1b;
  int extraout_A0;
  bool bVar3;
  
  if (DAT_00ff164a == 0) {
    return;
  }
  if (_DAT_00ffe0f2 == 0) {
    if (_DAT_00ffe082 == 0) {
      if ((_DAT_00ffdf8c == 0) || (DAT_00ffe0bc != 0)) {
        DAT_00ff1642 = DAT_00ff1642 - 1;
        if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
          cVar1 = lookup_runner_animation_delta();
          bVar2 = extraout_D1b + cVar1;
          if ((short)((ushort)bVar2 << 8) < 0) {
            bVar2 = 0;
          }
          *(byte *)(extraout_A0 + 4) = bVar2;
          *(char *)(extraout_A0 + 5) = *(char *)(extraout_A0 + 5) + '\x01';
          if (*(byte *)(extraout_A0 + 7) <= *(byte *)(extraout_A0 + 5)) {
            *(undefined1 *)(extraout_A0 + 5) = 0;
          }
        }
      }
      else {
        DAT_00ff1642 = DAT_00ff1642 - 1;
        if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
          DAT_00ff1642 = DAT_00ff1644;
          DAT_00ff1643 = DAT_00ff1643 + 1;
          if (3 < DAT_00ff1643) {
            DAT_00ff1643 = 3;
          }
        }
      }
    }
    if (_DAT_00ffe082 == 0) {
      append_sprite_record();
    }
    build_title_sprite_records_with_attribute_offset();
    advance_primary_world_effect_overlay();
    advance_secondary_world_effect_overlay();
    advance_background_world_effect_overlay();
    return;
  }
  DAT_00ff1642 = DAT_00ff1642 - 1;
  if ((short)((ushort)DAT_00ff1642 << 8) < 0) {
    DAT_00ff1642 = 5;
    DAT_00ff1643 = DAT_00ff1643 + 1;
    if (5 < DAT_00ff1643) {
      reset_effect_entity();
      return;
    }
  }
  append_sprite_record();
  build_title_sprite_records_with_attribute_offset();
  if ((_DAT_00ffe084 == 0) && (DAT_00ffe0b6 != 0)) {
    if ((_DAT_00ffe0c8 != 0) && (DAT_00ffe0be = DAT_00ffe056 << 2, 0x6aff < DAT_00ffe0be)) {
      _DAT_00ffe0c8 = 0;
      DAT_00ffe0b6 = 0;
      _DAT_00ffe0f4 = 1;
      begin_world_effect_descriptor_transition();
      return;
    }
    if (DAT_00ffe0b6 != 1) {
      DAT_00ffe0be = DAT_00ffe0c0 + DAT_00ffe0be;
      bVar3 = false;
      arm_effect_after_threshold();
      if (!bVar3) {
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
  return;
}

