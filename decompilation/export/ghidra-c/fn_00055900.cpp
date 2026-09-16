// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055900


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00055900(void)

{
  byte bVar1;
  byte bVar2;
  undefined2 extraout_D0u;
  uint uVar3;
  short sVar4;
  ushort uVar5;
  short sVar6;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A1;
  char cVar7;
  undefined1 uVar8;
  char cVar9;
  
  if (*(char *)(in_A0 + 0x57) == '\x0f') {
    sVar4 = 3;
    DAT_00fff0cc = '\x01';
  }
  else if (*(char *)(in_A0 + 0x57) == '\x0e') {
    sVar4 = 3;
  }
  else {
    sVar4 = 6;
  }
  _DAT_00fff112 = -1;
  FUN_00055ed6();
  FUN_00055f20();
  FUN_00055786();
  sVar6 = 2;
  uVar3 = 9;
  do {
    if ((CONCAT22(extraout_D0u,*(undefined2 *)(extraout_A0 + 0x90)) & 1 << (uVar3 & 0x1f)) != 0)
    break;
    uVar3 = (uint)(ushort)((short)uVar3 + 1);
    sVar6 = sVar6 + -1;
  } while (sVar6 != -1);
  FUN_00000dba();
  bVar2 = *(char *)(extraout_A0_00 + 0x9f) - DAT_00fff0dd;
  if ((short)((ushort)bVar2 << 8) < 0) {
    bVar1 = bVar2 + (char)sVar4;
    uVar5 = (ushort)bVar1;
    if (bVar1 == 0 || SCARRY1(bVar2,(char)sVar4) != (short)((ushort)bVar1 << 8) < 0) {
      return;
    }
  }
  else {
    uVar5 = (ushort)bVar2 + sVar4;
    if (-1 < (short)(uVar5 - 0xb)) {
      uVar5 = 10;
    }
  }
  cVar9 = '\0';
  cVar7 = _DAT_00fff112 < 0;
  uVar8 = _DAT_00fff112 == 0;
  if ((bool)uVar8) {
    FUN_00055e92();
    if ((bool)uVar8 || cVar9 != cVar7) {
      return;
    }
  }
  else if ((_DAT_00fff112 < 1) && (FUN_00055eb4(), (bool)uVar8 || cVar9 != cVar7)) {
    return;
  }
  FUN_00055e62();
  if (!(bool)uVar8 && cVar9 == cVar7) {
    cVar7 = -(char)uVar5;
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
        cVar7 = -1;
      }
      if (DAT_00fff0cc == '\0') {
        bVar2 = *(byte *)(extraout_A1 + 0x4c);
        bVar1 = cVar7 + bVar2;
        *(byte *)(extraout_A1 + 0x4c) = bVar1;
        if (bVar1 != 0 && SCARRY1(cVar7,bVar2) == (short)((ushort)bVar1 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x4c) = 0;
      }
      else {
        DAT_00fff0cc = '\0';
        bVar2 = *(byte *)(extraout_A1 + 0x9c);
        bVar1 = cVar7 + bVar2;
        *(byte *)(extraout_A1 + 0x9c) = bVar1;
        if (bVar1 != 0 && SCARRY1(cVar7,bVar2) == (short)((ushort)bVar1 << 8) < 0) {
          return;
        }
        *(undefined1 *)(extraout_A1 + 0x9c) = 0;
      }
      if (((DAT_00fffbed == '\0') && (*(char *)(extraout_A1 + 0x9b) == 'B')) &&
         (sVar4 = _DAT_00fffbf0 + -0x14, _DAT_00fffbf0 = _DAT_00fffbf0 + 1, -1 < sVar4)) {
        _DAT_00fffbf0 = 0x14;
      }
      DAT_00fff0ce = DAT_00fff0ce - 1;
      if ((short)((ushort)DAT_00fff0ce << 8) < 0) {
        DAT_00fff0ce = 0xf;
        DAT_00ff018d = DAT_00ff018d + '\x01';
        DAT_00ff028d = DAT_00ff028d + '\x01';
        DAT_00ff038d = DAT_00ff038d + '\x01';
      }
      if ((*(char *)(extraout_A1 + 0x9b) == '\x13') &&
         (cVar7 = DAT_00fffbfa + -8, DAT_00fffbfa = DAT_00fffbfa + '\x01', -1 < cVar7)) {
        DAT_00fffbfa = '\b';
      }
    }
  }
  return;
}


