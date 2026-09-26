// Ghidra headless export
// Entry: 0x0001FA78
// Name: select_field_mode_service
// Decompile status: source-shaped annotation
//
// The native conversion is ghidra_conversion::select_field_mode_service. The
// source route table is preserved as scene/submode cases, menu fields, signed
// prompt variants, and the FFF112 service-result handshake.
//
// Calls recovered from direct call instructions:
//   0x0001F246 run_mode_exit_services
//   0x00056AC0 run_field_mode_service

void select_field_mode_service(void)
{
  // Each matched branch installs A5/A0 source resources, represented by a
  // native profile and prompt-variant alias in the conversion.
  if (DAT_00fffbf8 == 2 && DAT_00fffbf9 == 0x0B) {
    DAT_00fff0b6 = 1;
    DAT_00fff0b7 = -1;
  }
  else if (DAT_00fffbf8 == 1 && DAT_00fffbf9 == 5) {
    DAT_00fff0b6 = 2;
    if (DAT_00fff0d5 != 0) {
      run_mode_exit_services();
      return;
    }
    DAT_00fff112 = 0;
    run_mode_exit_services();
    if (DAT_00fff112 == 0) return;
    DAT_00fff0b7 = ((signed char)DAT_00fff112 < 0) ? -1 : 2;
    run_field_mode_service();
    return;
  }
  else if (DAT_00fffbf8 == 1 && DAT_00fffbf9 == 3) {
    DAT_00fff0b6 = 4;
    DAT_00fff0b7 = 0;
  }
  else if (DAT_00fffbf8 == 5 && DAT_00fffbf9 >= 2 &&
           DAT_00fffbf9 <= 5) {
    DAT_00fff0b6 = menu_mode_for_scene_five_submode();
    DAT_00fff0b7 = menu_result_for_scene_five_submode();
  }
  else if (DAT_00fffbf8 == 4 && DAT_00fffbf9 == 3) {
    DAT_00fff0b6 = 3;
    DAT_00fff0b7 = 0;
    if (DAT_00fff0d5 == 0) {
      DAT_00fff112 = 0;
      run_mode_exit_services();
      if (DAT_00fff112 == 0) return;
      DAT_00fff0b7 = ((signed char)DAT_00fff112 < 0) ? -1 : 2;
      run_field_mode_service();
      return;
    }
    run_mode_exit_services();
    return;
  }
  else {
    return;
  }

  // Shared scene-five/scene-two-eleven/scene-one-three flow: nonzero
  // alternate results only present the selected prompt; the default variant
  // proceeds into the intro stage after mode-exit services.
  if (DAT_00fff0d5 != 0) {
    run_mode_exit_services();
    return;
  }
  run_mode_exit_services();
  run_field_mode_service();
}
