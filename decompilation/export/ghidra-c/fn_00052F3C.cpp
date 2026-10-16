// Ghidra headless export
// Entry: 0x00052F3C
// Name: service_audio_command_queue
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access
//   0x00052E84 end_audio_bus_access
//   0x00052E96 flush_audio_command
//   0x00052EA2 append_audio_command


void service_audio_command_queue(void)

{
  begin_audio_bus_access();
  flush_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


