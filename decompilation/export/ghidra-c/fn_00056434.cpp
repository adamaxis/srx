// Ghidra headless export
// Entry: 0x00056434
// Name: convert_intro_routine_0x00056434
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A468 initialize_audio_frame


void convert_intro_routine_0x00056434(void)

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


