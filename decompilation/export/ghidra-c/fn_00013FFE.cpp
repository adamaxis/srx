// Ghidra headless export
// Entry: 0x00013FFE
// Name: dispatch_entity_animation_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 upload_field_asset_setup
//   0x0000ABFC update_entity_animation_delay
//   0x0000AE6E update_entity_target_heading
//   0x0000AF7C prepare_entity_delayed_animation_primary
//   0x0000AFAE prepare_entity_delayed_animation_alternate
//   0x0000B11C synchronize_entity_behavior_spawn
//   0x0000B1DC configure_entity_target_transition
//   0x0000B39A set_entity_heading_with_field_animation
//   0x0000B3FE set_entity_heading_with_alternate_animation
//   0x00014226 service_entity_animation_action
//   0x00014492 select_entity_animation_link
//   0x00014508 prepare_entity_animation_fallback_target
//   0x00014C7C resolve_entity_target_motion
//   0x00014D8E probe_entity_spawn_collision_window
//   0x0001514E process_entity_spawn_cycle
//   0x00015646 spawn_entity_effect_record


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint dispatch_entity_animation_state(void)

{
  ushort uVar1;
  uint uVar2;
  undefined2 *puVar3;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int iVar4;
  undefined2 *extraout_A0_03;
  undefined1 uVar5;
  bool bVar6;
  
  DAT_00ffdfe0 = 0;
  select_entity_animation_link();
  uVar2 = *(uint *)(extraout_A0 + 0xa2);
  if (uVar2 == 0) {
    if ((*(byte *)(extraout_A0 + 0x10) & 8) == 0) {
      return uVar2;
    }
    uVar2 = prepare_entity_animation_fallback_target();
    return uVar2;
  }
  uVar5 = 0;
  if (*(char *)(extraout_A0 + 0x57) != '\0') {
    iVar4 = extraout_A0;
    if (*(short *)(extraout_A0 + 0xdc) < 0) {
      if (*(short *)(extraout_A0 + 0xc) == 0) {
        return 0x12345678;
      }
      if (*(char *)(extraout_A0 + 0x56) < '\0') {
        if ((*(char *)(extraout_A0 + 0x57) == '\x04') ||
           (bVar6 = *(byte *)(extraout_A0 + 0x57) < 0xb, *(byte *)(extraout_A0 + 0x57) == 0xb)) {
          return uVar2;
        }
    probe_entity_spawn_collision_window();
        if (bVar6) {
          return 0x12345678;
        }
        if ((*(char *)(extraout_A0_01 + 0x57) == '\f') &&
           (((puVar3 = &DAT_00ff0100, DAT_00ffe190 == '\0' ||
             (puVar3 = &DAT_00ff0200, DAT_00ffe190 == '\x01')) ||
            (puVar3 = &DAT_00ff0300, DAT_00ffe190 == '\x02')))) {
          *(undefined2 **)(extraout_A0_01 + 0xa2) = puVar3;
        }
        update_entity_target_heading();
        if (DAT_00fff0dd == '\0') {
          uVar2 = prepare_entity_delayed_animation_primary();
          return uVar2;
        }
        uVar2 = prepare_entity_delayed_animation_alternate();
        return uVar2;
      }
      if (('\f' < *(char *)(extraout_A0 + 0x57)) &&
         (bVar6 = *(byte *)(extraout_A0 + 0x57) < 0x10, (char)*(byte *)(extraout_A0 + 0x57) < '\x10'
         )) {
        probe_entity_spawn_collision_window();
        if (bVar6) {
          return 0x12345678;
        }
        uVar2 = spawn_entity_effect_record();
        if (!bVar6) {
          uVar2 = set_entity_heading_with_alternate_animation();
          return uVar2;
        }
        return uVar2;
      }
      uVar5 = false;
      if (*(char *)(extraout_A0 + 0x57) == '\0') goto LAB_000141a0;
      uVar2 = probe_entity_spawn_collision_window();
      if ((bool)uVar5) {
        _DAT_00ffdfde = 0;
        return 0x12345678;
      }
      if ((DAT_00ffe186 == '\0') && ((*(byte *)(extraout_A0_00 + 0xb0) & 0x10) == 0)) {
        return uVar2;
      }
      _DAT_00ffdfde = 1;
      bVar6 = false;
      uVar2 = process_entity_spawn_cycle();
      iVar4 = extraout_A0_02;
      if (!bVar6) {
        uVar2 = set_entity_heading_with_field_animation();
        return uVar2;
      }
    }
    bVar6 = false;
    if (*(char *)(iVar4 + 0x4e) != '\0') {
      uVar2 = probe_entity_spawn_collision_window();
      if (bVar6) {
        uVar2 = 0x12345678;
      }
      return uVar2;
    }
    if (((*(byte *)(iVar4 + 0x10) & 8) != 0) && (*(char *)(iVar4 + 0x4f) != '\0')) {
      if (*(short *)(iVar4 + 0xdc) < 0) {
        uVar2 = service_entity_animation_action();
        return uVar2;
      }
      return uVar2;
    }
    *(undefined1 *)(iVar4 + 0x57) = 0;
    *(undefined1 *)(iVar4 + 0x56) = 0x7f;
    uVar5 = 0;
    update_entity_animation_delay();
    upload_field_asset_setup();
  }
LAB_000141a0:
  uVar2 = resolve_entity_target_motion();
  if (!(bool)uVar5) {
    DAT_00ffdfe0 = 2;
    return uVar2;
  }
  extraout_A0_03[0x4b] = *extraout_A0_03;
  extraout_A0_03[0x4c] = extraout_A0_03[2];
  if ((short)extraout_A0_03[0x6e] < 0) {
    DAT_00ffdfe0 = 1;
    if ((*(byte *)((int)extraout_A0_03 + 0x11) & 0x40) != 0) {
      uVar2 = synchronize_entity_behavior_spawn();
      return uVar2;
    }
    uVar2 = configure_entity_target_transition();
    return uVar2;
  }
  DAT_00ffdfe0 = 3;
  uVar1 = extraout_A0_03[0xb];
  if ((short)uVar1 < 0x3d) {
    *(undefined4 *)(extraout_A0_03 + 0x1d) = 0xb45c;
    return (uint)uVar1;
  }
  return (uint)uVar1;
}

