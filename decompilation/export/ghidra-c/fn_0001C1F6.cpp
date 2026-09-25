// Ghidra headless export
// Entry: 0x0001C1F6
// Name: advance_audio_menu_countdown
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019F7E clear_first_ready_menu_slot
//   0x00053112 service_audio_command_wrapper


void advance_audio_menu_countdown(void)

{
  int extraout_A0;
  
  DAT_00ffe0ec = DAT_00ffe0ec + -1;
  if (DAT_00ffe0ec < 0) {
    DAT_00ffe0ec = DAT_00ffe0ea;
    DAT_00ffe169 = DAT_00ffe169 + -1;
    if (DAT_00ffe169 == '\0') {
      service_audio_command_wrapper();
      *(byte *)(extraout_A0 + 0x12) = *(byte *)(extraout_A0 + 0x12) & 0xfd;
      clear_first_ready_menu_slot();
    }
  }
  return;
}


