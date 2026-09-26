// Ghidra headless export
// Entry: 0x00055C76
// Name: advance_entity_action_resolution
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00003674 advance_entity_delay_meter
//   0x000037EC advance_entity_behavior_meter
//   0x00055740 apply_target_condition_modifier
//   0x00055786 apply_dynamic_status_modifier
//   0x00055E62 apply_equipment_defense_action_modifier
//   0x00055EB4 subtract_armor_secondary_modifier
//   0x00055ED6 calculate_equipment_command_average
//   0x00055F20 select_posture_adjusted_value


/* WARNING: Removing unreachable block (ram,0x00055caa) */
/* WARNING: Removing unreachable block (ram,0x00055d12) */

void advance_entity_action_resolution(void)

{
  byte bVar1;
  undefined2 extraout_D0u;
  undefined2 extraout_D0u_00;
  undefined2 unaff_D5w;
  byte bVar2;
  uint uVar3;
  short sVar4;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A1;
  char cVar5;
  bool bVar6;
  char cVar7;
  
  calculate_equipment_command_average();
  select_posture_adjusted_value();
  apply_target_condition_modifier();
  apply_dynamic_status_modifier();
  count_thresholded_random_rolls();
  sVar4 = CONCAT11((char)((ushort)unaff_D5w >> 8),DAT_00fff0dd);
  if (DAT_00fff0dd == '\0') {
    return;
  }
  cVar7 = SCARRY2(sVar4,3);
  sVar4 = sVar4 + 3;
  cVar5 = sVar4 < 0;
  bVar6 = sVar4 == 0;
  subtract_armor_secondary_modifier();
  if (bVar6 || cVar7 != cVar5) {
    return;
  }
  apply_equipment_defense_action_modifier();
  if (bVar6 || cVar7 != cVar5) {
    return;
  }
  calculate_equipment_command_average();
  select_posture_adjusted_value();
  apply_target_condition_modifier();
  apply_dynamic_status_modifier();
  count_thresholded_random_rolls();
  bVar1 = (char)sVar4 - DAT_00fff0dd;
  if (bVar1 == 0 || SBORROW1((char)sVar4,DAT_00fff0dd) != (short)((ushort)bVar1 << 8) < 0) {
    return;
  }
  sVar4 = 3;
  uVar3 = 5;
  do {
    if ((CONCAT22(extraout_D0u,*(undefined2 *)(extraout_A0 + 0x90)) & 1 << (uVar3 & 0x1f)) != 0)
    break;
    uVar3 = (uint)(ushort)((short)uVar3 + 1);
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  calculate_equipment_command_average();
  select_posture_adjusted_value();
  apply_dynamic_status_modifier();
  sVar4 = 2;
  uVar3 = 9;
  do {
    if ((CONCAT22(extraout_D0u_00,*(undefined2 *)(extraout_A0_00 + 0x90)) & 1 << (uVar3 & 0x1f)) !=
        0) break;
    uVar3 = (uint)(ushort)((short)uVar3 + 1);
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  count_thresholded_random_rolls();
  bVar2 = bVar1 - DAT_00fff0dd;
  if (bVar2 == 0 || SBORROW1(bVar1,DAT_00fff0dd) != (short)((ushort)bVar2 << 8) < 0) {
    return;
  }
  if ((*(ushort *)(extraout_A0_01 + 0x90) & 8) == 0) {
    if ((*(ushort *)(extraout_A0_01 + 0x90) & 4) == 0) {
      cVar5 = -bVar2;
      if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
        if (DAT_00ffe186 == '\x03') {
          return;
        }
        advance_entity_behavior_meter();
        return;
      }
      if (*(char *)(extraout_A1 + 0x9a) < '\0') {
        cVar5 = -1;
      }
      bVar1 = cVar5 + *(byte *)(extraout_A1 + 0x9c);
      *(byte *)(extraout_A1 + 0x9c) = bVar1;
      if ((short)((ushort)bVar1 << 8) < 0) {
        *(undefined1 *)(extraout_A1 + 0x9c) = 0;
      }
      goto LAB_00055dfc;
    }
    bVar6 = SBORROW1(bVar2,'\x02');
    bVar2 = bVar2 - 2;
    if (bVar2 == 0 || bVar6 != (short)((ushort)bVar2 << 8) < 0) {
      return;
    }
  }
  cVar5 = -bVar2;
  if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
    if (DAT_00ffe186 == '\x03') {
      return;
    }
    advance_entity_delay_meter();
    return;
  }
  if (*(char *)(extraout_A1 + 0x9a) < '\0') {
    cVar5 = -1;
  }
  bVar1 = cVar5 + *(byte *)(extraout_A1 + 0x4c);
  *(byte *)(extraout_A1 + 0x4c) = bVar1;
  if (-1 < (short)((ushort)bVar1 << 8)) {
    return;
  }
  *(undefined1 *)(extraout_A1 + 0x4c) = 0;
LAB_00055dfc:
  if (-1 < (short)((ushort)(byte)(DAT_00fff0ce - 1) << 8)) {
    DAT_00fff0ce = DAT_00fff0ce - 1;
    return;
  }
  DAT_00fff0ce = 0xf;
  DAT_00ff018d = DAT_00ff018d + '\x01';
  DAT_00ff028d = DAT_00ff028d + '\x01';
  DAT_00ff038d = DAT_00ff038d + '\x01';
  return;
}


