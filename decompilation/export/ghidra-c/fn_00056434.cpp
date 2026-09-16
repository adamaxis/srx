// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00056434


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00056434(void)

{
  char *pcVar1;
  char cVar2;
  char cVar3;
  short sVar4;
  char extraout_D1b;
  
  FUN_0001a468();
  DAT_00fff0a3 = 0;
  sVar4 = _DAT_00fff118 * 0x14;
  pcVar1 = (char *)(sVar4 + 0xff110f);
  cVar2 = *pcVar1;
  cVar3 = cVar2 - extraout_D1b;
  *pcVar1 = cVar3;
  if (cVar3 == '\0' || cVar2 < extraout_D1b) {
    *(undefined1 *)(sVar4 + 0xff110f) = 0;
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


