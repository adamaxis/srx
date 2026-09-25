// Ghidra headless export
// Entry: 0x000531EA
// Name: copy_audio_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053216 select_audio_secondary_stream


void copy_audio_state(void)

{
  short sVar1;
  undefined4 *extraout_A0;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  select_audio_secondary_stream();
  sVar1 = 0x9f;
  puVar2 = extraout_A0;
  puVar3 = &DAT_00ff1100;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}

