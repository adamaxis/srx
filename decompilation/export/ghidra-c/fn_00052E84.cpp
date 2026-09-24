// Ghidra headless export
// Entry: 0x00052E84
// Name: end_audio_bus_access
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void end_audio_bus_access(void)

{
  IO_Z80BUS = 0;
  return;
}


