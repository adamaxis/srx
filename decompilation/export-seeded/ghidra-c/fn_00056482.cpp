// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00056482


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00056482(void)

{
  char *pcVar1;
  char cVar2;
  short sVar3;
  char extraout_D1b;
  
  FUN_0001a468();
  DAT_00fff0a3 = 0;
  sVar3 = _DAT_00fff118 * 0x14;
  pcVar1 = (char *)(sVar3 + 0xff1110);
  cVar2 = *pcVar1 - extraout_D1b;
  *pcVar1 = cVar2;
  if (cVar2 < '\0') {
    *(undefined1 *)(sVar3 + 0xff1110) = 0;
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


