// Ghidra headless export
// Entry: 0x00019CB2
// Name: FUN_00019cb2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 FUN_00017c52
//   0x00017C84 set_hud_selection_value
//   0x00017DB8 FUN_00017db8
//   0x00019B34 FUN_00019b34
//   0x00019BAE select_hud_mode


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
    cVar1 = select_hud_mode();
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
    set_hud_selection_value();
    FUN_00017c52();
  }
  sVar2 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar2] == '\x04') {
      FUN_000195aa();
      (&DAT_00fff0e8)[sVar2] = 0;
      *(short *)((int)&DAT_00fff0f2 + (int)(short)(sVar2 << 1)) = extraout_D1w * 10;
      FUN_00017bc0();
      FUN_00019762();
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


