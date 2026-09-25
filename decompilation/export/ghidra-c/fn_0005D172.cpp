// Ghidra headless export
// Entry: 0x0005D172
// Name: convert_intro_routine_0x0005d172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service


void convert_intro_routine_0x0005d172(void)

{
  prepare_mode_entry_service();
  service_audio_command_frame();
  adjust_runner_source_byte_triplet();
  DAT_00fff0be = 7;
  func_0x0005d1f6();
  prepare_mode_entry_service();
  DAT_00ffe172 = 0xff;
  return;
}


