// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019CB2


void FUN_00019cb2(void)

{
  char cVar1;
  short extraout_D1w;
  short sVar2;
  bool bVar3;
  
  if (DAT_00fff0d3 < '\x01') {
    FUN_00019b34();
  }
  else {
    cVar1 = FUN_00019bae();
    if ((cVar1 == '\b') && (DAT_00ffdf84 != '\0')) {
      bVar3 = SCARRY1(DAT_00fff0d3,DAT_00fffbca);
      DAT_00fffbca = DAT_00fff0d3 + DAT_00fffbca;
      if (DAT_00fffbca == '\0' || bVar3 != DAT_00fffbca < '\0') {
        DAT_00fffbca = '\0';
      }
      else if ('\n' < DAT_00fffbca) {
        DAT_00fffbca = '\n';
      }
    }
    else {
      FUN_00017db8();
    }
    FUN_00017c84();
    FUN_00017c52();
  }
  sVar2 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar2] == '\x04') {
      FUN_000195aa();
      *(undefined1 *)(sVar2 + 0xfff0e8) = 0;
      *(short *)(&DAT_00fff0f2 + (short)(sVar2 << 1)) = extraout_D1w * 10;
      FUN_00017bc0();
      FUN_00019762();
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


