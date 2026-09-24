// Ghidra headless export
// Entry: 0x0001C1F6
// Name: FUN_0001c1f6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019F7E FUN_00019f7e
//   0x00053112 service_audio_command_wrapper


void FUN_0001c1f6(void)

{
  int extraout_A0;
  
  DAT_00ffe0ec = DAT_00ffe0ec + -1;
  if (DAT_00ffe0ec < 0) {
    DAT_00ffe0ec = DAT_00ffe0ea;
    DAT_00ffe169 = DAT_00ffe169 + -1;
    if (DAT_00ffe169 == '\0') {
      service_audio_command_wrapper();
      *(byte *)(extraout_A0 + 0x12) = *(byte *)(extraout_A0 + 0x12) & 0xfd;
      FUN_00019f7e();
    }
  }
  return;
}


