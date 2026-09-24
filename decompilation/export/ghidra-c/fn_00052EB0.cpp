// Ghidra headless export
// Entry: 0x00052EB0
// Name: append_one_with_audio_bus
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access
//   0x00052E84 end_audio_bus_access
//   0x00052EA2 append_audio_command


void append_one_with_audio_bus(void)

{
  begin_audio_bus_access();
  append_audio_command();
  end_audio_bus_access();
  return;
}


