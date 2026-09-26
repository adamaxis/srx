// Ghidra headless export
// Entry: 0x0001F65A
// Name: select_field_exit_prompt
// Decompile status: source-shaped annotation
//
// The native conversion is ghidra_conversion::select_field_exit_prompt. The
// source routine is a five-route field scene/submode table. It chooses a
// prompt descriptor and resource profile, invokes the mode-exit coordinator,
// then refreshes the mode display when the coordinator leaves a negative
// FFF10D marker.
//
// Calls recovered from direct call instructions:
//   0x0000CF38 run_field_exit_prompt_menu
//   0x0001F246 run_mode_exit_services

void select_field_exit_prompt(void)
{
  const signed char alternate_result = (signed char)DAT_00fff0d5;
  const uint32_t world_flags = DAT_00fffc1e;

  if (DAT_00fffbf8 == 0 && DAT_00fffbf9 == 3) {
    select_prompt_resource(/* scene 0/submode 3 */, /* marker 0x4B */);
    select_prompt_variant(alternate_result,
                          /* default 0xF4910, positive 0xF495C,
                             negative 0xF4954 */);
  }
  else if (DAT_00fffbf8 == 1 && DAT_00fffbf9 == 1) {
    select_prompt_resource(/* scene 1/submode 1 */, /* marker 0x1E */);
    if (alternate_result > 0) {
      select_prompt(/* positive 0xF49E0 */);
    }
    else if (alternate_result < 0) {
      select_prompt(/* negative 0xF49D8 */);
    }
    else if ((world_flags & (1UL << 13)) != 0 &&
             (world_flags & (1UL << 30)) == 0) {
      select_prompt(/* flagged 0xF4990 */);
    }
    else {
      select_prompt(/* default 0xF4964 */);
    }
  }
  else if (DAT_00fffbf8 == 2 && DAT_00fffbf9 == 2) {
    select_prompt_resource(/* scene 2/submode 2 */, /* marker 0x14 */);
    select_prompt_variant(alternate_result,
                          /* default 0xF4AA0, positive 0xF4AF4,
                             negative 0xF4AEC */);
  }
  else if (DAT_00fffbf8 == 3 && DAT_00fffbf9 == 1) {
    select_prompt_resource(/* scene 3/submode 1 */, /* marker 0x32 */);
    select_prompt_variant(alternate_result,
                          /* default 0xF4A18, positive 0xF49E0,
                             negative 0xF4A50 */);
  }
  else if (DAT_00fffbf8 == 4 && DAT_00fffbf9 == 0) {
    select_prompt_resource(/* scene 4/submode 0 */, /* marker 0x28 */);
    if (alternate_result > 0) {
      select_prompt(/* positive 0xF4A98 */);
    }
    else if (alternate_result < 0) {
      select_prompt_resource(/* negative-variant profile 0x122024 */);
      select_prompt(/* negative 0xF4A90 */);
    }
    else {
      select_prompt(/* default 0xF4A60 */);
    }
  }
  else {
    return;
  }

  run_mode_exit_services();
  if ((signed char)DAT_00fff10d < 0) {
    run_field_exit_prompt_menu();
  }
  return;
}
