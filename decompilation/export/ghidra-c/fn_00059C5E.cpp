// Ghidra headless export
// Entry: 0x00059C5E
// Name: FUN_00059c5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x000530EA FUN_000530ea
//   0x00053126 prepare_mode_entry_service
//   0x00059D12 FUN_00059d12
//   0x00059D7C FUN_00059d7c
//   0x0005AA96 FUN_0005aa96
//   0x0005AD1A run_title_intro_stage_loop
//   0x0005AF62 run_title_intro_schedule


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059c5e(void)

{
  DAT_00ffe18b = 0xff;
  sample_controller_input();
  _DAT_00ffddc2 = 0;
  FUN_000530ea();
  run_title_intro_schedule();
  if (_DAT_00ffddc2 == 0) {
    prepare_mode_entry_service();
    FUN_000530ea();
    _DAT_00ffddc2 = 0;
    FUN_00059d7c();
    if (_DAT_00ffddc2 == 0) {
      prepare_mode_entry_service();
      FUN_000530ea();
      _DAT_00ffddc2 = 0;
      FUN_0005aa96();
      if (_DAT_00ffddc2 == 0) {
        _DAT_00ffddc2 = 0;
        run_title_intro_stage_loop();
        _DAT_00ffddc2 = 0;
        step_palette_transition();
        DAT_00fffbf8 = 2;
        FUN_00059d12();
      }
    }
  }
  sample_controller_input();
  step_palette_transition();
  prepare_mode_entry_service();
  return;
}


