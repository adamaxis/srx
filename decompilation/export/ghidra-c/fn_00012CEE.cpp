// Ghidra headless export
// Entry: 0x00012CEE
// Name: update_equipment_combat_bars
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x000104F6 apply_action_item_phase_modifiers
//   0x00012148 read_entity_action_code
//   0x00012C5A write_menu_repeated_grid_with_metadata
//   0x00012C74 merge_equipment_action_skill_value
//   0x00012CD6 render_menu_text_asset
//   0x00012FA4 advance_entity_action_menu
//   0x00055E62 apply_equipment_defense_action_modifier
//   0x00055ED6 calculate_equipment_command_average
//   0x00055F02 calculate_item18_skill_value
//   0x00055F20 select_posture_adjusted_value


/* WARNING: Removing unreachable block (ram,0x00012ddc) */
/* WARNING: Removing unreachable block (ram,0x00012d50) */

void update_equipment_combat_bars(void)

{
  undefined1 uVar1;
  undefined2 extraout_D0u;
  short sVar2;
  char cVar3;
  uint uVar4;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  
  write_menu_repeated_grid_with_metadata();
  sVar2 = calculate_equipment_command_average();
  if (sVar2 == 0) {
    if ((char)(DAT_00fff0d0 + -2) < '\0') {
      select_posture_adjusted_value();
      cVar3 = read_entity_action_code();
      if (cVar3 != '\x17') {
        if ((char)(cVar3 + -0xd) < '\0') {
          uVar1 = *(undefined1 *)(extraout_A0_00 + 0x57);
          *(char *)(extraout_A0_00 + 0x57) = cVar3;
          merge_equipment_action_skill_value();
          *(undefined1 *)(extraout_A0_01 + 0x57) = uVar1;
        }
        else if (-1 < (char)(cVar3 + -0x10)) {
          return;
        }
      }
    }
    else {
      if (*(char *)(extraout_A0 + 0x56) < '\0') {
        advance_entity_action_menu();
        calculate_item18_skill_value();
        select_posture_adjusted_value();
        apply_action_item_phase_modifiers();
        goto LAB_00012e7a;
      }
      select_posture_adjusted_value();
      if ((*(char *)(extraout_A0_02 + 0x56) != '\x7f') &&
         ((char)(*(char *)(extraout_A0_02 + 0x57) + -0xd) < '\0')) {
        merge_equipment_action_skill_value();
      }
    }
    render_menu_text_asset();
  }
  else {
    if ((((char)(DAT_00fff0d0 + -2) < '\0') && (cVar3 = read_entity_action_code(), cVar3 != '\x17'))
       && (-1 < (char)(cVar3 + -0x10))) {
      return;
    }
    select_posture_adjusted_value();
    apply_equipment_defense_action_modifier();
    sVar2 = 2;
    uVar4 = 9;
    do {
      if ((CONCAT22(extraout_D0u,*(undefined2 *)(extraout_A0_03 + 0x90)) & 1 << (uVar4 & 0x1f)) != 0
         ) break;
      uVar4 = (uint)(ushort)((short)uVar4 + 1);
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    render_menu_text_asset();
  }
LAB_00012e7a:
  write_vdp_grid();
  return;
}


