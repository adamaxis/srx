// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055B16


/* WARNING: Removing unreachable block (ram,0x00055b36) */
/* WARNING: Removing unreachable block (ram,0x000559c4) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00055b16(void)

{
  byte bVar1;
  short sVar2;
  byte bVar3;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A1;
  int extraout_A1_00;
  char cVar4;
  undefined1 uVar5;
  char cVar6;
  
  FUN_00055740();
  FUN_00055786();
  FUN_00000d5a();
  FUN_00000dba();
  *(char *)(extraout_A0_00 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A0_00 + 0x9d) = 6;
  if ((short)(*(short *)(extraout_A1_00 + 0x3e) + -3) < 0) {
    FUN_0001047e();
  }
  _DAT_00fff112 = 1;
  FUN_00000dba();
  bVar1 = *(char *)(extraout_A0 + 0x9f) - DAT_00fff0dd;
  if ((char)bVar1 < '\0') {
    bVar3 = bVar1 + 2;
    if (bVar3 == 0 || SCARRY1(bVar1,'\x02') != (short)((ushort)bVar3 << 8) < 0) {
      return;
    }
  }
  else {
    bVar3 = bVar1 + 2;
    if (-1 < (short)(bVar1 - 9)) {
      bVar3 = 10;
    }
  }
  cVar6 = '\0';
  cVar4 = _DAT_00fff112 < 0;
  uVar5 = _DAT_00fff112 == 0;
  if ((bool)uVar5) {
    FUN_00055e92();
    if ((bool)uVar5 || cVar6 != cVar4) {
      return;
    }
  }
  else if ((_DAT_00fff112 < 1) && (FUN_00055eb4(), (bool)uVar5 || cVar6 != cVar4)) {
    return;
  }
  FUN_00055e62();
  if (!(bool)uVar5 && cVar6 == cVar4) {
    cVar4 = -bVar3;
    if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
      if (DAT_00ffe186 != '\x03') {
        if (DAT_00fff0cc != '\0') {
          DAT_00fff0cc = 0;
          FUN_000037ec();
          return;
        }
        FUN_00003674();
        return;
      }
    }
    else {
      if (*(char *)(extraout_A1 + 0x9a) < '\0') {
        cVar4 = -1;
      }
      if (DAT_00fff0cc == '\0') {
        bVar1 = *(byte *)(extraout_A1 + 0x4c);
        bVar3 = cVar4 + bVar1;
        *(byte *)(extraout_A1 + 0x4c) = bVar3;
        if (bVar3 != 0 && SCARRY1(cVar4,bVar1) == (short)((ushort)bVar3 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x4c) = 0;
      }
      else {
        DAT_00fff0cc = '\0';
        bVar1 = *(byte *)(extraout_A1 + 0x9c);
        bVar3 = cVar4 + bVar1;
        *(byte *)(extraout_A1 + 0x9c) = bVar3;
        if (bVar3 != 0 && SCARRY1(cVar4,bVar1) == (short)((ushort)bVar3 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x9c) = 0;
      }
      if (((DAT_00fffbed == '\0') && (*(char *)(extraout_A1 + 0x9b) == 'B')) &&
         (sVar2 = _DAT_00fffbf0 + -0x14, _DAT_00fffbf0 = _DAT_00fffbf0 + 1, -1 < sVar2)) {
        _DAT_00fffbf0 = 0x14;
      }
      DAT_00fff0ce = DAT_00fff0ce + -1;
      if (DAT_00fff0ce < '\0') {
        DAT_00fff0ce = '\x0f';
        DAT_00ff018d = DAT_00ff018d + '\x01';
        DAT_00ff028d = DAT_00ff028d + '\x01';
        DAT_00ff038d = DAT_00ff038d + '\x01';
      }
      if ((*(char *)(extraout_A1 + 0x9b) == '\x13') &&
         (cVar4 = DAT_00fffbfa + -8, DAT_00fffbfa = DAT_00fffbfa + '\x01', -1 < cVar4)) {
        DAT_00fffbfa = '\b';
      }
    }
  }
  return;
}


