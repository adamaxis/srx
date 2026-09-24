// Ghidra headless export
// Entry: 0x000531AA
// Name: FUN_000531aa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00052EEC FUN_00052eec
//   0x00052FA0 flush_with_audio_bus


void FUN_000531aa(void)

{
  FUN_00052eec(&DAT_000284e4,&DAT_00029595,&DAT_00029609,&DAT_000311bf);
  flush_with_audio_bus();
  return;
}


