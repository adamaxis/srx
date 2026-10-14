// Ghidra headless export
// Entry: 0x0001033E
// Name: resolve_first_aid_item_use
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A guard_entity_phase_window
//   0x00000DBA count_thresholded_random_rolls
//   0x000037EC advance_entity_behavior_meter
//   0x000104F6 apply_action_item_phase_modifiers
//   0x00010660 calculate_action_residual
//   0x00015830 dispatch_entity_effect
//   0x00055740 apply_target_condition_modifier
//   0x00055786 apply_dynamic_status_modifier
//   0x00055F02 calculate_item18_skill_value
//   0x00055F20 select_posture_adjusted_value


/* WARNING: Removing unreachable block (ram,0x000103a8) */

void resolve_first_aid_item_use(void)

{
  byte *pbVar1;
  byte bVar2;
  byte bVar3;
  char unaff_D2b;
  short sVar4;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int iVar5;
  char cVar6;
  bool bVar7;
  char cVar8;
  
  if (*(char *)(in_A0 + 0x9c) == '\x01') {
    DAT_00fff0dd = 0;
    *(undefined1 *)(in_A0 + 0x9f) = 0;
    if (-1 < (short)(*(short *)(in_A0 + 0x3e) + -3)) {
      *(undefined1 *)(in_A0 + 0x56) = 0;
      *(undefined1 *)(in_A0 + 0x57) = *(undefined1 *)(in_A0 + 0x66);
    }
    return;
  }
  calculate_item18_skill_value();
  select_posture_adjusted_value();
  apply_action_item_phase_modifiers();
  apply_target_condition_modifier();
  apply_dynamic_status_modifier();
  guard_entity_phase_window();
  count_thresholded_random_rolls();
  cVar6 = DAT_00fff0dd;
  *(char *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  if (cVar6 != '\0') {
    dispatch_entity_effect();
    iVar5 = DAT_00fff10e;
    sVar4 = *(byte *)(extraout_A0_00 + 0x57) - 1;
    if (sVar4 == 5) {
      *(undefined1 *)(DAT_00fff10e + 0x1e) = *(undefined1 *)(extraout_A0_00 + 0x9f);
      cVar8 = *(char *)(extraout_A0_00 + 0x9d);
      *(char *)(iVar5 + 0x1f) = cVar8;
      cVar6 = cVar8 < '\0';
      bVar7 = cVar8 == '\0';
      cVar8 = false;
    }
    else {
      cVar8 = SBORROW2(sVar4,0xc);
      cVar6 = (short)(*(byte *)(extraout_A0_00 + 0x57) - 0xd) < 0;
      bVar7 = sVar4 == 0xc;
      if (bVar7) {
        *(undefined1 *)(DAT_00fff10e + 0x20) = *(undefined1 *)(extraout_A0_00 + 0x9f);
        cVar8 = *(char *)(extraout_A0_00 + 0x9d);
        *(char *)(iVar5 + 0x21) = cVar8;
        cVar6 = cVar8 < '\0';
        bVar7 = cVar8 == '\0';
        cVar8 = false;
      }
    }
    calculate_action_residual();
    iVar5 = extraout_A0_01;
    if (bVar7 || cVar8 != cVar6) goto LAB_0001046e;
    sVar4 = 6;
    do {
      if (*(char *)(extraout_A0_01 + 0x66 + (int)sVar4) == '\x13') {
        if (unaff_D2b <= *(char *)(extraout_A0_01 + 0x6e + (int)sVar4)) {
          pbVar1 = (byte *)(extraout_A0_01 + 0x6e + (int)sVar4);
          bVar2 = *pbVar1;
          bVar3 = bVar2 - unaff_D2b;
          *pbVar1 = bVar3;
          if (bVar3 != 0 && SBORROW1(bVar2,unaff_D2b) == (short)((ushort)bVar3 << 8) < 0) {
            return;
          }
          *(undefined1 *)(extraout_A0_01 + 0x66 + (int)sVar4) = 0;
          *(undefined1 *)(extraout_A0_01 + 0x6e + (int)sVar4) = 0;
          return;
        }
        unaff_D2b = unaff_D2b - *(char *)(extraout_A0_01 + 0x6e + (int)sVar4);
        *(undefined1 *)(extraout_A0_01 + 0x66 + (int)sVar4) = 0;
        *(undefined1 *)(extraout_A0_01 + 0x6e + (int)sVar4) = 0;
      }
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    do {
      advance_entity_behavior_meter();
      iVar5 = extraout_A0_02;
LAB_0001046e:
    } while (*(char *)(iVar5 + 0x9c) == '\0');
    return;
  }
  return;
}


