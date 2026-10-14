// Ghidra headless export
// Entry: 0x00056434
// Name: advance_menu_selection_service_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A468 initialize_audio_frame


// Direct disassembly subtracts the positive menu-service delta from the
// selected record's +0x0F byte, clears FFF0A3, and rolls FFF0CE plus the
// three phase bytes when the timer expires. The typed host conversion is in
// ghidra_conversion/menu_selection_timing_conversion.
// The expiry branch is SUB.B/BGT and therefore retains the 68000 overflow
// flag rather than using only the wrapped result's sign bit.
void advance_menu_selection_service_timer(void)

{
  char cVar1;
  char cVar2;
  short sVar3;
  char extraout_D1b;
  
  initialize_audio_frame();
  DAT_00fff0a3 = 0;
  sVar3 = DAT_00fff118 * 0x14;
  cVar1 = (&DAT_00ff110f)[sVar3];
  cVar2 = cVar1 - extraout_D1b;
  (&DAT_00ff110f)[sVar3] = cVar2;
  if (cVar2 == '\0' || cVar1 < extraout_D1b) {
    (&DAT_00ff110f)[sVar3] = 0;
    DAT_00fff0ce = DAT_00fff0ce + -2;
    if (DAT_00fff0ce < '\0') {
      DAT_00fff0ce = '\x0f';
      DAT_00ff018d = DAT_00ff018d + '\x01';
      DAT_00ff028d = DAT_00ff028d + '\x01';
      DAT_00ff038d = DAT_00ff038d + '\x01';
    }
  }
  return;
}

