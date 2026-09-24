// Ghidra headless export
// Entry: 0x00052F6E
// Name: FUN_00052f6e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access


void FUN_00052f6e(void)

{
  begin_audio_bus_access();
  flush_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


