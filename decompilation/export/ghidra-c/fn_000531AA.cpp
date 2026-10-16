// Ghidra headless export
// Entry: 0x000531AA
// Name: initialize_title_audio_assets
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052EEC initialize_audio_sequence
//   0x00052FA0 flush_with_audio_bus


void initialize_title_audio_assets(void)

{
  initialize_audio_sequence(&DAT_000284e4,&DAT_00029595,&DAT_00029609,&DAT_000311bf);
  flush_with_audio_bus();
  return;
}


