// Ghidra headless export
// Entry: 0x00056482
// Name: FUN_00056482
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A154 FUN_0001a154
//   0x0001A468 FUN_0001a468


void FUN_00056482(void)

{
  char cVar1;
  short sVar2;
  char extraout_D1b;
  
  FUN_0001a468();
  DAT_00fff0a3 = 0;
  sVar2 = DAT_00fff118 * 0x14;
  cVar1 = (&DAT_00ff1110)[sVar2] - extraout_D1b;
  (&DAT_00ff1110)[sVar2] = cVar1;
  if (cVar1 < '\0') {
    (&DAT_00ff1110)[sVar2] = 0;
    FUN_0001a154();
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


