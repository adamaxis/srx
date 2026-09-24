// Ghidra headless export
// Entry: 0x000559BC
// Name: FUN_000559bc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00003674 FUN_00003674
//   0x000037EC FUN_000037ec
//   0x00055E62 FUN_00055e62
//   0x00055E92 FUN_00055e92
//   0x00055EB4 FUN_00055eb4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000559bc(void)

{
  byte bVar1;
  byte bVar2;
  short sVar3;
  short unaff_D5w;
  ushort uVar4;
  int extraout_A0;
  int extraout_A1;
  char cVar5;
  undefined1 uVar6;
  char cVar7;
  
  count_thresholded_random_rolls();
  bVar2 = *(char *)(extraout_A0 + 0x9f) - DAT_00fff0dd;
  if ((char)bVar2 < '\0') {
    bVar1 = bVar2 + (char)unaff_D5w;
    uVar4 = (ushort)bVar1;
    if (bVar1 == 0 || SCARRY1(bVar2,(char)unaff_D5w) != (short)((ushort)bVar1 << 8) < 0) {
      return;
    }
  }
  else {
    uVar4 = (ushort)bVar2 + unaff_D5w;
    if (-1 < (short)(uVar4 - 0xb)) {
      uVar4 = 10;
    }
  }
  cVar7 = '\0';
  cVar5 = _DAT_00fff112 < 0;
  uVar6 = _DAT_00fff112 == 0;
  if ((bool)uVar6) {
    FUN_00055e92();
    if ((bool)uVar6 || cVar7 != cVar5) {
      return;
    }
  }
  else if ((_DAT_00fff112 < 1) && (FUN_00055eb4(), (bool)uVar6 || cVar7 != cVar5)) {
    return;
  }
  FUN_00055e62();
  if (!(bool)uVar6 && cVar7 == cVar5) {
    cVar5 = -(char)uVar4;
    if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
      if (DAT_00ffe186 != '\x03') {
        if (DAT_00fff0cc == '\0') {
          FUN_00003674();
          return;
        }
        DAT_00fff0cc = 0;
        FUN_000037ec();
        return;
      }
    }
    else {
      if (*(char *)(extraout_A1 + 0x9a) < '\0') {
        cVar5 = -1;
      }
      if (DAT_00fff0cc == '\0') {
        bVar2 = *(byte *)(extraout_A1 + 0x4c);
        bVar1 = cVar5 + bVar2;
        *(byte *)(extraout_A1 + 0x4c) = bVar1;
        if (bVar1 != 0 && SCARRY1(cVar5,bVar2) == (short)((ushort)bVar1 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x4c) = 0;
      }
      else {
        DAT_00fff0cc = '\0';
        bVar2 = *(byte *)(extraout_A1 + 0x9c);
        bVar1 = cVar5 + bVar2;
        *(byte *)(extraout_A1 + 0x9c) = bVar1;
        if (bVar1 != 0 && SCARRY1(cVar5,bVar2) == (short)((ushort)bVar1 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x9c) = 0;
      }
      if ((((char)DAT_00fffbec == '\0') && (*(char *)(extraout_A1 + 0x9b) == 'B')) &&
         (sVar3 = DAT_00fffbf0 + -0x14, DAT_00fffbf0 = DAT_00fffbf0 + 1, -1 < sVar3)) {
        DAT_00fffbf0 = 0x14;
      }
      DAT_00fff0ce = DAT_00fff0ce + -1;
      if (DAT_00fff0ce < '\0') {
        DAT_00fff0ce = '\x0f';
        DAT_00ff018d = DAT_00ff018d + '\x01';
        DAT_00ff028d = DAT_00ff028d + '\x01';
        DAT_00ff038d = DAT_00ff038d + '\x01';
      }
      if ((*(char *)(extraout_A1 + 0x9b) == '\x13') &&
         (cVar5 = DAT_00fffbfa + -8, DAT_00fffbfa = DAT_00fffbfa + '\x01', -1 < cVar5)) {
        DAT_00fffbfa = '\b';
      }
    }
  }
  return;
}


