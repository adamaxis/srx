// Ghidra headless export
// Entry: 0x00052EEC
// Name: initialize_audio_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052DE0 initialize_audio_ram
//   0x00052E26 reset_audio_processor
//   0x00052EB0 append_one_with_audio_bus
//   0x00052EC6 append_three_with_audio_bus


void initialize_audio_sequence
               (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  initialize_audio_ram();
  reset_audio_processor();
  append_one_with_audio_bus(0xffffffff);
  append_one_with_audio_bus(0xb);
  append_three_with_audio_bus(param_1);
  append_three_with_audio_bus(param_2);
  append_three_with_audio_bus(param_3);
  append_three_with_audio_bus(param_4);
  return;
}


