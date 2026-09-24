// Ghidra headless export
// Entry: 0x00052EC6
// Name: append_three_with_audio_bus
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access
//   0x00052E84 end_audio_bus_access
//   0x00052EA2 append_audio_command


void append_three_with_audio_bus(void)

{
  begin_audio_bus_access();
  append_audio_command();
  append_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


