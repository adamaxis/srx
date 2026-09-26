// Ghidra headless export
// Entry: 0x000563CC
// Name: advance_primary_world_effect_pending_roll
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00019BF6 select_hud_mode_auxiliary_value
//   0x0001B1FC reset_title_transition


// Direct disassembly obtains the status-adjusted trial count, rolls against
// FFFBBE+0x00 with a minimum difficulty of two, subtracts the excess over
// FFFBBE+0x01, decrements FFFBBE+0x07, and resets the title transition. The
// typed host conversion is in ghidra_conversion/world_effect_pending_roll_conversion.
void advance_primary_world_effect_pending_roll(void)

{
  char cVar1;
  char *extraout_A0;
  
  select_hud_mode_auxiliary_value();
  count_thresholded_random_rolls();
  if ((DAT_00fff0dd != '\0') &&
     (cVar1 = DAT_00fff0dd - extraout_A0[1], cVar1 != '\0' && extraout_A0[1] <= DAT_00fff0dd)) {
    cVar1 = *extraout_A0 - cVar1;
    *extraout_A0 = cVar1;
    if (cVar1 < '\0') {
      *extraout_A0 = '\0';
      extraout_A0[7] = '\0';
    }
    if (extraout_A0[7] != '\0') {
      extraout_A0[7] = extraout_A0[7] + -1;
    }
    reset_title_transition();
  }
  return;
}

