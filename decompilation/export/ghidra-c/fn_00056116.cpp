// Ghidra headless export
// Entry: 0x00056116
// Name: advance_world_effect_intro_gate
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00017A5E convert_world_routine_0x00017a5e
//   0x0001989A convert_world_routine_0x0001989a
//   0x00019BF6 pass_through_value
//   0x0001A908 initialize_world_effect_mode
//   0x00055F44 calculate_runner_action_delay
//   0x000562DE add_world_effect_phase_modifier

// Direct disassembly at 0x56116 is authoritative for the marker state
// machine. The typed host conversion is in
// ghidra_conversion/world_effect_mode_selection_conversion and keeps the
// random-roll, HUD, menu, and world-mode services as explicit boundaries.

undefined4 advance_world_effect_intro_gate(void)

{
  char cVar1;
  undefined2 extraout_D0u;
  undefined2 extraout_D0u_00;
  undefined2 extraout_D0u_01;
  undefined2 uVar2;
  undefined4 in_D0;
  
  uVar2 = (undefined2)((uint)in_D0 >> 0x10);
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
  else if (DAT_00fff0a3 < '\x01') {
    if (DAT_00fff0a0 != '\x01') {
      DAT_00fff0a3 = 0xff;
      return 2;
    }
    goto LAB_000561e2;
  }
  DAT_00fff0a3 = '\0';
LAB_000561e2:
  return CONCAT22(uVar2,0xffff);
}

