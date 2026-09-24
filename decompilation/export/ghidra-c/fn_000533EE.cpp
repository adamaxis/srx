// Ghidra headless export
// Entry: 0x000533EE
// Name: FUN_000533ee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053200 FUN_00053200


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000533ee(void)

{
  ushort uVar1;
  int iVar2;
  short sVar3;
  short sVar4;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  undefined4 *puVar5;
  undefined4 *extraout_A1;
  undefined4 *extraout_A1_00;
  undefined4 *extraout_A1_01;
  char *pcVar6;
  
  FUN_00053200();
  iVar2 = (uint)*(byte *)((int)&DAT_00ff1104 + (int)(short)(DAT_00fff118 * 0x14)) * 0xe;
  DAT_00fff102 = (short)CONCAT31((int3)((uint)iVar2 >> 8),
                                 *(char *)((int)&DAT_00ff1100 + (short)(DAT_00fff118 * 0x14) + 3) +
                                 (char)iVar2);
  puVar5 = &DAT_00fffaa4;
  *(undefined1 *)((int)&DAT_00fffaa4 + (int)DAT_00fff102) = 0xff;
  _DAT_00fffbfc = 0;
  DAT_00fff104 = DAT_00fff102 + -0xe;
  iVar2 = extraout_A0;
  if (-1 < DAT_00fff104) {
    if (-1 < (char)('\v' - *(char *)(extraout_A0 + DAT_00fff104))) {
      sVar3 = 6;
      pcVar6 = "\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar6 == *(char *)(extraout_A0 + DAT_00fff102)) {
          *(undefined1 *)((int)&DAT_00fffaa4 + (int)DAT_00fff104) = 0xff;
          DAT_00fff104 = DAT_00fff104 + 0xe;
          goto LAB_000534e8;
        }
        sVar3 = sVar3 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar3 != -1);
      DAT_00fff104 = DAT_00fff102;
      goto LAB_000534e8;
    }
    sVar3 = 6;
    pcVar6 = "\r\x0f\x11\x13\x14\x15\x16";
    do {
      if (*pcVar6 == *(char *)(extraout_A0 + DAT_00fff104)) {
        *(undefined1 *)((int)&DAT_00fffaa4 + (int)DAT_00fff104) = 0xff;
        uVar1 = _DAT_00fffbfc;
        _DAT_00fffbfc = 0x800;
        FUN_00053434();
        DAT_00fff104 = DAT_00fff104 + 0xe;
        iVar2 = extraout_A0_00;
        puVar5 = extraout_A1;
        _DAT_00fffbfc = uVar1;
        goto LAB_000534e8;
      }
      sVar3 = sVar3 + -1;
      pcVar6 = pcVar6 + 1;
    } while (sVar3 != -1);
  }
  DAT_00fff104 = DAT_00fff102;
LAB_000534e8:
  if (((_DAT_00fffbfc & 0x400) == 0) && (sVar3 = DAT_00fff104 + 1, -1 < (short)(0x8b - sVar3))) {
    if ((char)('\v' - *(char *)(iVar2 + sVar3)) < '\0') {
      sVar4 = 6;
      pcVar6 = 
      "\f\x0e\x0f\x12\x13\x15\x16\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16"
      ;
      do {
        if (*pcVar6 == *(char *)(iVar2 + sVar3)) {
          DAT_00fff104 = sVar3;
          *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
          uVar1 = _DAT_00fffbfc;
          _DAT_00fffbfc = 0x1000;
          FUN_00053434();
          DAT_00fff104 = DAT_00fff104 + -1;
          iVar2 = extraout_A0_01;
          puVar5 = extraout_A1_00;
          _DAT_00fffbfc = uVar1;
          break;
        }
        sVar4 = sVar4 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar4 != -1);
    }
    else {
      sVar4 = 6;
      pcVar6 = "\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar6 == *(char *)(iVar2 + DAT_00fff104)) {
          DAT_00fff104 = sVar3;
          *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
          DAT_00fff104 = DAT_00fff104 + -1;
          break;
        }
        sVar4 = sVar4 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar4 != -1);
    }
  }
  if (((_DAT_00fffbfc & 0x800) == 0) && (sVar3 = DAT_00fff104 + 0xe, -1 < (short)(0x8b - sVar3))) {
    if ((char)('\v' - *(char *)(iVar2 + sVar3)) < '\0') {
      sVar4 = 6;
      pcVar6 = "\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar6 == *(char *)(iVar2 + sVar3)) {
          DAT_00fff104 = sVar3;
          *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
          uVar1 = _DAT_00fffbfc;
          _DAT_00fffbfc = 0x200;
          FUN_00053434();
          DAT_00fff104 = DAT_00fff104 + -0xe;
          iVar2 = extraout_A0_02;
          puVar5 = extraout_A1_01;
          _DAT_00fffbfc = uVar1;
          break;
        }
        sVar4 = sVar4 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar4 != -1);
    }
    else {
      sVar4 = 6;
      pcVar6 = "\r\x0f\x11\x13\x14\x15\x16";
      do {
        if (*pcVar6 == *(char *)(iVar2 + DAT_00fff104)) {
          DAT_00fff104 = sVar3;
          *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
          DAT_00fff104 = DAT_00fff104 + -0xe;
          break;
        }
        sVar4 = sVar4 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar4 != -1);
    }
  }
  if ((_DAT_00fffbfc & 0x1000) != 0) {
    return;
  }
  sVar3 = DAT_00fff104 + -1;
  if (-1 < sVar3) {
    if (-1 < (char)('\v' - *(char *)(iVar2 + sVar3))) {
      sVar4 = 6;
      pcVar6 = 
      "\f\x0e\x0f\x12\x13\x15\x16\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16"
      ;
      do {
        if (*pcVar6 == *(char *)(iVar2 + DAT_00fff104)) {
          DAT_00fff104 = sVar3;
          *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
          DAT_00fff104 = DAT_00fff104 + 1;
          return;
        }
        sVar4 = sVar4 + -1;
        pcVar6 = pcVar6 + 1;
      } while (sVar4 != -1);
      return;
    }
    sVar4 = 6;
    pcVar6 = "\f\x10\x11\x12\x13\x14\x16\r\x0e\x10\x12\x14\x15\x16\r\x0f\x11\x13\x14\x15\x16";
    do {
      if (*pcVar6 == *(char *)(iVar2 + sVar3)) {
        DAT_00fff104 = sVar3;
        *(undefined1 *)((int)puVar5 + (int)sVar3) = 0xff;
        uVar1 = _DAT_00fffbfc;
        _DAT_00fffbfc = 0x400;
        FUN_00053434();
        _DAT_00fffbfc = uVar1;
        DAT_00fff104 = DAT_00fff104 + 1;
        return;
      }
      sVar4 = sVar4 + -1;
      pcVar6 = pcVar6 + 1;
    } while (sVar4 != -1);
  }
  return;
}


