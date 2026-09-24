// Ghidra headless export
// Entry: 0x00052FA0
// Name: FUN_00052fa0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access
//   0x00052E84 end_audio_bus_access
//   0x00052E96 flush_audio_command


void FUN_00052fa0(void)

{
  begin_audio_bus_access();
  flush_audio_command();
  end_audio_bus_access();
  return;
}


