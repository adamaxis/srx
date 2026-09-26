// Ghidra headless export
// Entry: 0x000562EC
// Name: convert_intro_routine_0x000562ec
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x0001792E render_menu_hud_rows
//   0x00017D90 advance_hud_panel
//   0x00017DB8 advance_hud_footer_offset
//   0x00019B40 convert_world_routine_0x00019b40
//   0x0001A12E advance_menu_countdown
//   0x00055F44 calculate_runner_action_delay
//   0x000562DE convert_intro_routine_0x000562de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x000562ec(void)

{
  char cVar1;
  short sVar2;
  char extraout_D1b;
  char *extraout_A0;
  
  calculate_runner_action_delay();
  convert_intro_routine_0x000562de();
  count_thresholded_random_rolls();
  cVar1 = DAT_00fff0a4 - DAT_00fff0dd;
  if (cVar1 != '\0') {
    if (cVar1 != '\0' && DAT_00fff0dd <= DAT_00fff0a4) {
      if ((char)(DAT_00fffbbf - cVar1) < '\0') {
        sVar2 = convert_world_routine_0x00019b40();
        if (sVar2 == 8) {
          advance_hud_panel();
          return;
        }
        advance_menu_countdown();
        if (((extraout_D1b < '\0') && (advance_hud_footer_offset(), DAT_00fffbca == '\0')) &&
           (((sVar2 = convert_world_routine_0x00019b40(), sVar2 == 3 ||
             (((sVar2 == 0xc || (sVar2 == 0xf)) || (sVar2 == 0x17)))) || (sVar2 == 0x18)))) {
          pass_through_value();
          count_thresholded_random_rolls();
          if ((DAT_00fff0dd != '\0') &&
             (cVar1 = DAT_00fff0dd - extraout_A0[1], cVar1 != '\0' && extraout_A0[1] <= DAT_00fff0dd
             )) {
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
      }
    }
    else {
      _DAT_00ffe07e = 1;
    }
  }
  render_menu_hud_rows();
  return;
}

