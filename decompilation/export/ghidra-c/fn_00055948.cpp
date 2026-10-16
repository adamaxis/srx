// Ghidra headless export
// Entry: 0x00055948
// Name: resolve_equipment_action_status
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00055786 apply_dynamic_status_modifier
//   0x000559BC resolve_dynamic_entity_action
//   0x00055ED6 calculate_equipment_command_average
//   0x00055F20 select_posture_adjusted_value


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void resolve_equipment_action_status(void)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  undefined2 extraout_D0u;
  uint uVar4;
  ushort uVar5;
  short sVar6;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A1;
  char cVar7;
  undefined1 uVar8;
  char cVar9;
  
  DAT_00fff0cc = '\0';
  bVar3 = -(&DAT_00166ad4)
           [(short)(ushort)(byte)(&DAT_000d4565)[(short)((*(byte *)(in_A0 + 0x57) - 1) * 0x30)]];
  _DAT_00fff112 = 0;
  calculate_equipment_command_average();
  select_posture_adjusted_value();
  apply_dynamic_status_modifier();
  sVar6 = 2;
  uVar4 = 9;
  do {
    if ((CONCAT22(extraout_D0u,*(undefined2 *)(extraout_A0 + 0x90)) & 1 << (uVar4 & 0x1f)) != 0) {
      resolve_dynamic_entity_action();
      return;
    }
    uVar4 = (uint)(ushort)((short)uVar4 + 1);
    sVar6 = sVar6 + -1;
  } while (sVar6 != -1);
  count_thresholded_random_rolls();
  bVar2 = *(char *)(extraout_A0_00 + 0x9f) - DAT_00fff0dd;
  if ((short)((ushort)bVar2 << 8) < 0) {
    bVar1 = bVar2 + bVar3;
    uVar5 = (ushort)bVar1;
    if (bVar1 == 0 || SCARRY1(bVar2,bVar3) != (short)((ushort)bVar1 << 8) < 0) {
      return;
    }
  }
  else {
    uVar5 = (ushort)bVar2 + (ushort)bVar3;
    if (-1 < (short)(uVar5 - 0xb)) {
      uVar5 = 10;
    }
  }
  cVar9 = '\0';
  cVar7 = _DAT_00fff112 < 0;
  uVar8 = _DAT_00fff112 == 0;
  if ((bool)uVar8) {
    subtract_armor_primary_modifier();
    if ((bool)uVar8 || cVar9 != cVar7) {
      return;
    }
  }
  else if ((_DAT_00fff112 < 1) &&
          (subtract_armor_secondary_modifier(), (bool)uVar8 || cVar9 != cVar7)) {
    return;
  }
  apply_equipment_defense_action_modifier();
  if (!(bool)uVar8 && cVar9 == cVar7) {
    cVar7 = -(char)uVar5;
    if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
      if (DAT_00ffe186 != '\x03') {
        if (DAT_00fff0cc != '\0') {
          DAT_00fff0cc = 0;
          advance_entity_behavior_meter();
          return;
        }
        advance_entity_delay_meter();
        return;
      }
    }
    else {
      if (*(char *)(extraout_A1 + 0x9a) < '\0') {
        cVar7 = -1;
      }
      if (DAT_00fff0cc == '\0') {
        bVar3 = *(byte *)(extraout_A1 + 0x4c);
        bVar2 = cVar7 + bVar3;
        *(byte *)(extraout_A1 + 0x4c) = bVar2;
        if (bVar2 != 0 && SCARRY1(cVar7,bVar3) == (short)((ushort)bVar2 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x4c) = 0;
      }
      else {
        DAT_00fff0cc = '\0';
        bVar3 = *(byte *)(extraout_A1 + 0x9c);
        bVar2 = cVar7 + bVar3;
        *(byte *)(extraout_A1 + 0x9c) = bVar2;
        if (bVar2 != 0 && SCARRY1(cVar7,bVar3) == (short)((ushort)bVar2 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x9c) = 0;
      }
      if ((((char)DAT_00fffbec == '\0') && (*(char *)(extraout_A1 + 0x9b) == 'B')) &&
         (sVar6 = DAT_00fffbf0 + -0x14, DAT_00fffbf0 = DAT_00fffbf0 + 1, -1 < sVar6)) {
        DAT_00fffbf0 = 0x14;
      }
      DAT_00fff0ce = DAT_00fff0ce - 1;
      if ((short)((ushort)DAT_00fff0ce << 8) < 0) {
        DAT_00fff0ce = 0xf;
        DAT_00ff018d = DAT_00ff018d + '\x01';
        DAT_00ff028d = DAT_00ff028d + '\x01';
        DAT_00ff038d = DAT_00ff038d + '\x01';
      }
      if ((*(char *)(extraout_A1 + 0x9b) == '\x13') &&
         (cVar7 = DAT_00fffbfa + -8, DAT_00fffbfa = DAT_00fffbfa + '\x01', -1 < cVar7)) {
        DAT_00fffbfa = '\b';
      }
    }
  }
  return;
}


