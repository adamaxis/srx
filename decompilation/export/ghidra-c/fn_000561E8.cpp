// Ghidra headless export
// Entry: 0x000561E8
// Name: select_world_effect_animation_mode
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00017D72 draw_hud_panel
//   0x00017D90 advance_hud_panel
//   0x00019B40 convert_world_routine_0x00019b40
//   0x00019BF6 pass_through_value
//   0x00053216 select_secondary_runner_record_table
//   0x00056116 advance_world_effect_intro_gate
//   0x000562D4 add_world_effect_timer_modifier

// Direct disassembly at 0x561E8 is authoritative for the compact-record
// status ranges. The typed host conversion is in
// ghidra_conversion/world_effect_mode_selection_conversion.

undefined4 select_world_effect_animation_mode(void)

{
  char cVar1;
  undefined2 extraout_D0u;
  undefined2 extraout_D0u_00;
  undefined2 extraout_D0u_01;
  undefined2 uVar2;
  undefined2 extraout_D0u_02;
  undefined4 uVar3;
  short sVar4;
  int extraout_A0;
  
  select_secondary_runner_record_table();
  sVar4 = DAT_00fff118 * 0x14;
  if (*(char *)(extraout_A0 + 0x10 + (int)sVar4) != (&DAT_00ff1110)[sVar4]) {
    (&DAT_00ff1110)[sVar4] = (&DAT_00ff1110)[sVar4] + '\x01';
  }
  cVar1 = (&DAT_00ff1113)[sVar4];
  if (-1 < (char)(cVar1 + -2)) {
    if ((char)(cVar1 + -5) < '\0') goto LAB_00056268;
    if ((char)(cVar1 + -7) < '\0') {
LAB_000560aa:
      if (DAT_00fff0e6 < '\0') {
        DAT_00fff0e7 = DAT_00fff0e7 + -1;
      }
      else {
        pass_through_value();
        add_world_effect_timer_modifier();
        count_thresholded_random_rolls();
        DAT_00fff0dd = DAT_00fff0dd + -1;
        if (-1 < DAT_00fff0dd) {
          DAT_00fff0e7 = '\n' - DAT_00fff0dd;
          DAT_00fff0e6 = 0xff;
          return 3;
        }
      }
      return 0xffffffff;
    }
    if ((char)(cVar1 + -0xb) < '\0') {
      if (DAT_00fff0a0 < '\x01') {
        uVar3 = advance_world_effect_intro_gate();
        return uVar3;
      }
      goto LAB_000560aa;
    }
    if ((char)(cVar1 + -0x11) < '\0') {
      if (DAT_00fff0a0 < '\x01') {
        uVar3 = advance_world_effect_intro_gate();
        return uVar3;
      }
LAB_00056268:
      pass_through_value();
      add_world_effect_timer_modifier();
      count_thresholded_random_rolls();
      sVar4 = convert_world_routine_0x00019b40();
      if (sVar4 == 8) {
        draw_hud_panel();
        advance_hud_panel();
      }
      uVar3 = 0;
      DAT_00fff0a4 = DAT_00fff0dd;
      if (DAT_00fff0dd < '\x01') {
        uVar3 = 0xffff;
        DAT_00fff0a4 = '\x01';
      }
      return uVar3;
    }
    if (-1 < (char)(cVar1 + -0x15)) goto LAB_00056268;
  }
  if (DAT_00fff0a3 < '\0') {
    pass_through_value();
    add_world_effect_phase_modifier();
    count_thresholded_random_rolls();
    cVar1 = DAT_00fff0dd;
    calculate_runner_action_delay();
    add_world_effect_phase_modifier();
    count_thresholded_random_rolls();
    if ((DAT_00fff0dd < cVar1) &&
       (convert_world_routine_0x0001989a(), uVar2 = extraout_D0u, DAT_00fff0b1 < '\0'))
    goto LAB_000561e2;
    convert_world_routine_0x00017a5e();
    if (DAT_00fff0a0 < '\x01') {
      DAT_00ffdf84 = 0;
      uVar2 = extraout_D0u_00;
    }
    else {
      initialize_world_effect_mode();
      uVar2 = extraout_D0u_01;
    }
  }
  else {
    uVar2 = extraout_D0u_02;
    if (DAT_00fff0a3 < '\x01') {
      if (DAT_00fff0a0 != '\x01') {
        DAT_00fff0a3 = 0xff;
        return 2;
      }
      goto LAB_000561e2;
    }
  }
  DAT_00fff0a3 = '\0';
LAB_000561e2:
  return CONCAT22(uVar2,0xffff);
}

