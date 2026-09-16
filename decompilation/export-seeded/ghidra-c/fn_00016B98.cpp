// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00016B98


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00016b98(void)

{
  ushort uVar1;
  short sVar2;
  int in_A0;
  short *extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  short *extraout_A0_02;
  short *extraout_A1;
  int extraout_A1_00;
  int extraout_A1_01;
  short *extraout_A1_02;
  short *psVar3;
  ushort *puVar4;
  bool bVar5;
  
  if ((*(short *)(in_A0 + 0x2c) != 0) ||
     (bVar5 = *(ushort *)(in_A0 + 0xe) < 7, *(ushort *)(in_A0 + 0xe) != 7)) {
    if (*(short *)(in_A0 + 0xe) == 0xd) {
      if ((DAT_00fffc40 & 1) == 0) {
        return;
      }
      bVar5 = false;
      FUN_00016e08();
      if (!bVar5) {
        *extraout_A1_02 = *extraout_A0_02 + 0x80;
        extraout_A1_02[2] = extraout_A0_02[2] + 0x80;
        *(byte *)(extraout_A1_02 + 8) = *(byte *)(extraout_A1_02 + 8) | 1;
        extraout_A1_02[1] = 100;
        return;
      }
      return;
    }
    if (*(short *)(in_A0 + 0xe) == 0xc) {
      if ((DAT_00fffc40 & 2) == 0) {
        return;
      }
      bVar5 = false;
      FUN_00016d8a();
      if (!bVar5) {
        *(short *)(extraout_A1_01 + 6) = (short)extraout_A0_01;
        *(undefined2 *)(extraout_A0_01 + 0x16) = 0x80;
        *(byte *)(extraout_A0_01 + 0x11) = *(byte *)(extraout_A0_01 + 0x11) | 1;
        *(undefined1 **)(extraout_A0_01 + 0x3a) = &LAB_000049a0;
      }
      return;
    }
    if ((_DAT_00ffdf40 != 2) && (_DAT_00ffdf40 != 0)) {
      return;
    }
    if (*(short *)(in_A0 + 0xe) == 0xe) {
      return;
    }
    bVar5 = false;
    if (_DAT_00fffc0e != 0) {
      return;
    }
  }
  FUN_00016e08();
  if (bVar5) {
    return;
  }
  *extraout_A1 = *extraout_A0 + 0x80;
  extraout_A1[2] = extraout_A0[2] + 0x80;
  *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 1;
  if (_DAT_00ffdf40 == 2) {
    psVar3 = &DAT_00016cf0;
  }
  else {
    psVar3 = (short *)&DAT_00016d50;
    if ((extraout_A0[0x16] != 0) || (extraout_A0[7] != 7)) {
      psVar3 = *(short **)((int)&PTR_DAT_00016cc8 + (int)(short)(_DAT_00ffddc4 * 4));
    }
  }
  uVar1 = FUN_00000d3a();
  do {
    puVar4 = (ushort *)(psVar3 + 1);
    sVar2 = *psVar3;
    psVar3 = psVar3 + 2;
    bVar5 = *puVar4 <= uVar1;
    uVar1 = uVar1 - *puVar4;
  } while (bVar5);
  if (sVar2 == 0x5a) {
    if (((*(char *)(extraout_A0_00 + 0x56) < '\0') || (*(char *)(extraout_A0_00 + 0x57) == '\0')) ||
       ('\x0e' < *(char *)(extraout_A0_00 + 0x57))) {
LAB_00016cb8:
      *(byte *)(extraout_A1_00 + 0x10) = *(byte *)(extraout_A1_00 + 0x10) & 0xfe;
      return;
    }
    sVar2 = (short)*(char *)(extraout_A0_00 + 0x57);
  }
  else if (sVar2 == 0x5b) goto LAB_00016cb8;
  *(short *)(extraout_A1_00 + 2) = sVar2;
  return;
}


