// Ghidra headless export
// Entry: 0x00052F5A
// Name: run_audio_bus_transaction
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access


void run_audio_bus_transaction(void)

{
  begin_audio_bus_access();
  flush_audio_command();
  append_audio_command();
  end_audio_bus_access();
  return;
}


