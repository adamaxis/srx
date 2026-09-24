// Ghidra headless export
// Entry: 0x00017D3E
// Name: advance_audio_service_tick
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530DE service_audio_if_enabled


void advance_audio_service_tick(void)

{
  DAT_00ffe18d = DAT_00ffe18d + '\x01';
  service_audio_if_enabled();
  return;
}


