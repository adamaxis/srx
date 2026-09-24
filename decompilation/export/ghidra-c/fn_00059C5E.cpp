// Ghidra headless export
// Entry: 0x00059C5E
// Name: run_title_intro_stages
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service
//   0x00059D12 enter_post_intro_mode
//   0x00059D7C run_title_intro_stage2
//   0x0005AA96 run_title_intro_orchestrator
//   0x0005AD1A run_title_intro_stage_loop
//   0x0005AF62 run_title_intro_schedule


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_title_intro_stages(void)

{
  DAT_00ffe18b = 0xff;
  sample_controller_input();
  _DAT_00ffddc2 = 0;
  service_audio_command_frame();
  run_title_intro_schedule();
  if (_DAT_00ffddc2 == 0) {
    prepare_mode_entry_service();
    service_audio_command_frame();
    _DAT_00ffddc2 = 0;
    run_title_intro_stage2();
    if (_DAT_00ffddc2 == 0) {
      prepare_mode_entry_service();
      service_audio_command_frame();
      _DAT_00ffddc2 = 0;
      run_title_intro_orchestrator();
      if (_DAT_00ffddc2 == 0) {
        _DAT_00ffddc2 = 0;
        run_title_intro_stage_loop();
        _DAT_00ffddc2 = 0;
        step_palette_transition();
        DAT_00fffbf8 = 2;
        enter_post_intro_mode();
      }
    }
  }
  sample_controller_input();
  step_palette_transition();
  prepare_mode_entry_service();
  return;
}


