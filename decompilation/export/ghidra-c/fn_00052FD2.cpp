// Ghidra headless export
// Entry: 0x00052FD2
// Name: flush_append_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E84 end_audio_bus_access
//   0x00052E96 flush_audio_command
//   0x00052EA2 append_audio_command


void flush_append_pair(void)

{
  flush_audio_command();
  append_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


