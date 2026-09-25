// Ghidra headless export
// Entry: 0x0001A468
// Name: initialize_audio_frame
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530EA service_audio_command_frame


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_audio_frame(void)

{
  DAT_00ffe0d0 = 0;
  DAT_00ffe0d2 = 2;
  _DAT_00ffe0d4 = 1;
  service_audio_command_frame();
  return;
}


