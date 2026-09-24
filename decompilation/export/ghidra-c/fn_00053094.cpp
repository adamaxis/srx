// Ghidra headless export
// Entry: 0x00053094
// Name: begin_bus_then_flush_append_pair
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052E4E begin_audio_bus_access
//   0x00052FD2 flush_append_pair


void begin_bus_then_flush_append_pair(void)

{
  begin_audio_bus_access();
  flush_append_pair();
  return;
}


