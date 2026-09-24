// Ghidra headless export
// Entry: 0x00052FD2
// Name: FUN_00052fd2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E84 end_audio_bus_access
//   0x00052E96 flush_audio_command
//   0x00052EA2 append_audio_command


void FUN_00052fd2(void)

{
  flush_audio_command();
  append_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


