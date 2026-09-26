// Ghidra headless export
// Entry: 0x0005D1B4
// Name: prepare_pre_title_audio_variant_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530EA service_audio_command_frame
//   0x00053126 prepare_mode_entry_service
//   0x00059C34 adjust_runner_source_byte_triplet


void prepare_pre_title_audio_variant_b(void)

{
  prepare_mode_entry_service();
  service_audio_command_frame();
  adjust_runner_source_byte_triplet();
  DAT_00fff0be = 0xc;
  func_0x0005d1f6();
  prepare_mode_entry_service();
  DAT_00ffe172 = 0xff;
  return;
}


