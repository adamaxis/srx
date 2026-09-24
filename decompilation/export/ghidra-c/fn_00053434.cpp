// Ghidra headless export
// Entry: 0x00053434
// Name: FUN_00053434
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053434 FUN_00053434


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00053434(void)

{
  short sVar1;
  ushort uVar2;
  short sVar3;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int in_A1;
  int extraout_A1;
  int extraout_A1_00;
  int extraout_A1_01;
  char *pcVar4;
  
  if (((_DAT_00fffbfc & 0x200) == 0) && (sVar1 = DAT_00fff104 + -0xe, -1 < sVar1)) {
    if ((char)('\v' - *(char *)(in_A0 + sVar1)) < '\0') {
      sVar3 = 6;
      pcVar4 = "\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar4 == *(char *)(in_A0 + sVar1)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          uVar2 = _DAT_00fffbfc;
          _DAT_00fffbfc = 0x800;
          FUN_00053434();
          DAT_00fff104 = DAT_00fff104 + 0xe;
          in_A0 = extraout_A0;
          in_A1 = extraout_A1;
          _DAT_00fffbfc = uVar2;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
    else {
      sVar3 = 6;
      pcVar4 = "\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar4 == *(char *)(in_A0 + DAT_00fff104)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          DAT_00fff104 = DAT_00fff104 + 0xe;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
  }
  if (((_DAT_00fffbfc & 0x400) == 0) && (sVar1 = DAT_00fff104 + 1, -1 < (short)(0x8b - sVar1))) {
    if ((char)('\v' - *(char *)(in_A0 + sVar1)) < '\0') {
      sVar3 = 6;
      pcVar4 = 
      "\f\x0e\x0f\x12\x13\x15\x16\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16"
      ;
      do {
        if (*pcVar4 == *(char *)(in_A0 + sVar1)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          uVar2 = _DAT_00fffbfc;
          _DAT_00fffbfc = 0x1000;
          FUN_00053434();
          DAT_00fff104 = DAT_00fff104 + -1;
          in_A0 = extraout_A0_00;
          in_A1 = extraout_A1_00;
          _DAT_00fffbfc = uVar2;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
    else {
      sVar3 = 6;
      pcVar4 = "\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar4 == *(char *)(in_A0 + DAT_00fff104)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          DAT_00fff104 = DAT_00fff104 + -1;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
  }
  if (((_DAT_00fffbfc & 0x800) == 0) && (sVar1 = DAT_00fff104 + 0xe, -1 < (short)(0x8b - sVar1))) {
    if ((char)('\v' - *(char *)(in_A0 + sVar1)) < '\0') {
      sVar3 = 6;
      pcVar4 = "\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar4 == *(char *)(in_A0 + sVar1)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          uVar2 = _DAT_00fffbfc;
          _DAT_00fffbfc = 0x200;
          FUN_00053434();
          DAT_00fff104 = DAT_00fff104 + -0xe;
          in_A0 = extraout_A0_01;
          in_A1 = extraout_A1_01;
          _DAT_00fffbfc = uVar2;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
    else {
      sVar3 = 6;
      pcVar4 = "\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar4 == *(char *)(in_A0 + DAT_00fff104)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          DAT_00fff104 = DAT_00fff104 + -0xe;
          break;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
    }
  }
  if ((_DAT_00fffbfc & 0x1000) != 0) {
    return;
  }
  sVar1 = DAT_00fff104 + -1;
  if (-1 < sVar1) {
    if (-1 < (char)('\v' - *(char *)(in_A0 + sVar1))) {
      sVar3 = 6;
      pcVar4 = 
      "\f\x0e\x0f\x12\x13\x15\x16\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16"
      ;
      do {
        if (*pcVar4 == *(char *)(in_A0 + DAT_00fff104)) {
          DAT_00fff104 = sVar1;
          *(undefined1 *)(in_A1 + sVar1) = 0xff;
          DAT_00fff104 = DAT_00fff104 + 1;
          return;
        }
        sVar3 = sVar3 + -1;
        pcVar4 = pcVar4 + 1;
      } while (sVar3 != -1);
      return;
    }
    sVar3 = 6;
    pcVar4 = "\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
    do {
      if (*pcVar4 == *(char *)(in_A0 + sVar1)) {
        DAT_00fff104 = sVar1;
        *(undefined1 *)(in_A1 + sVar1) = 0xff;
        uVar2 = _DAT_00fffbfc;
        _DAT_00fffbfc = 0x400;
        FUN_00053434();
        _DAT_00fffbfc = uVar2;
        DAT_00fff104 = DAT_00fff104 + 1;
        return;
      }
      sVar3 = sVar3 + -1;
      pcVar4 = pcVar4 + 1;
    } while (sVar3 != -1);
  }
  return;
}


