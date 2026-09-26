// Ghidra headless export
// Entry: 0x00010660
// Name: calculate_action_residual
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00055F02 calculate_item18_skill_value
//   0x00055F20 select_posture_adjusted_value


void calculate_action_residual(void)

{
  calculate_item18_skill_value();
  select_posture_adjusted_value();
  return;
}


