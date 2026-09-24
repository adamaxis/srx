// Ghidra headless export
// Entry: 0x00011B58
// Name: caseD_2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 FUN_00000d94
//   0x0000195C sample_controller_input
//   0x00003404 FUN_00003404
//   0x000036FC FUN_000036fc
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216
//   0x000111D0 FUN_000111d0
//   0x00011454 FUN_00011454
//   0x00011712 FUN_00011712
//   0x000118C6 FUN_000118c6
//   0x00011900 FUN_00011900
//   0x00011928 FUN_00011928
//   0x00012062 FUN_00012062
//   0x0001209A FUN_0001209a
//   0x000120E4 FUN_000120e4
//   0x000127B0 FUN_000127b0
//   0x00012C36 FUN_00012c36
//   0x000530DE FUN_000530de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void switchD_00011768::caseD_2(void)

{
  byte bVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  uint uVar4;
  ushort uVar5;
  short sVar6;
  undefined2 *puVar7;
  int extraout_A0;
  undefined4 *puVar8;
  undefined4 *puVar9;
  int iVar10;
  char cVar11;
  undefined1 uVar12;
  char cVar13;
  
  FUN_00012c36();
  FUN_00011900();
  FUN_0000c0da();
  FUN_0000c216();
  uVar2 = _DAT_00fff0d0;
  _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
  FUN_000111d0();
  _DAT_00fff116 = 0xffff;
  FUN_00011454();
  FUN_00011712();
  do {
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      uVar3 = 0xffff;
LAB_00011be6:
      FUN_000530de(uVar3);
      FUN_000111d0();
      FUN_000118c6();
      _DAT_00fff116 = 0xffff;
      FUN_00011454();
      FUN_00011712();
    }
    if ((DAT_00ffdd22._0_1_ & 8) != 0) {
      uVar3 = 1;
      goto LAB_00011be6;
    }
  } while ((((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
          ((DAT_00ffdd22._0_1_ & 0x20) == 0));
  cVar11 = '\0';
  uVar12 = 0;
  cVar13 = '\0';
  FUN_000530de();
  FUN_00012062();
  if ((bool)uVar12 || cVar13 != cVar11) goto LAB_00011c9c;
  _DAT_00fff0d0 = uVar2;
  if (DAT_00fff0d0 == '\x02') {
    FUN_00012c36();
    if ((-1 < (char)(*(char *)(DAT_00fffc04 + 0x56) << 1)) &&
       ((char)(*(char *)(DAT_00fffc04 + 0x57) + -0xd) < '\0')) {
      if (*(char *)(DAT_00fffc04 + 0x4e) != '\0') {
        DAT_00fff114 = '\0';
        FUN_000111d0();
        FUN_000127b0();
        FUN_0000c0da();
        do {
          while( true ) {
            wait_for_vblank();
            latch_controller_events();
            sample_controller_input();
            if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
            FUN_000530de();
            FUN_000127b0();
          }
        } while ((DAT_00ffdd22._0_1_ & 0x40) == 0);
        FUN_000530de();
        if (DAT_00fff114 < '\0') {
          return;
        }
      }
      iVar10 = DAT_00fffc04;
      bVar1 = *(byte *)(DAT_00fffc04 + 0x4f) - 1;
      *(byte *)(DAT_00fffc04 + 0x4f) = bVar1;
      if ((short)((ushort)bVar1 << 8) < 0) {
        if (*(char *)(DAT_00fffc04 + 0x4f) < '\x01') {
          FUN_00012c36();
          FUN_0000c0da();
          FUN_0000c0da();
          *(undefined1 *)(extraout_A0 + 0x4f) = 0;
        }
        return;
      }
      if (*(char *)(iVar10 + 0x56) < '\0') {
        *(undefined1 *)(iVar10 + 0x56) = 0x7f;
        *(undefined1 *)(iVar10 + 0x57) = 0;
        FUN_00003404();
      }
      FUN_000036fc();
      FUN_00012c36();
    }
    FUN_0000c0da();
    goto LAB_00011cb6;
  }
  if (DAT_00fff0d0 == '\x03') {
    FUN_00012c36();
    sVar6 = 2;
    puVar7 = &DAT_00ff0100;
    puVar8 = (undefined4 *)&DAT_00ffdd48;
    do {
      puVar9 = puVar8;
      if (((*(byte *)(puVar7 + 8) & 2) == 0) && ((*(byte *)(puVar7 + 8) & 1) != 0)) {
        puVar9 = puVar8 + 1;
        *puVar8 = puVar7;
      }
      puVar7 = puVar7 + 0x80;
      sVar6 = sVar6 + -1;
      puVar8 = puVar9;
    } while (sVar6 != -1);
    *puVar9 = 0xffffffff;
    if (_DAT_00ffdd48 < 0) goto LAB_00011e14;
    DAT_00ffdd28 = 0;
    FUN_0000c0da();
    _DAT_00ffdd2a = (uint)*(byte *)(DAT_00fffc04 + 0x4f);
    FUN_0000c216();
    FUN_0001209a();
    uVar2 = _DAT_00fff0d0;
    _DAT_00fff0d0 = CONCAT11(2,DAT_00fff0d1);
    FUN_000111d0();
    _DAT_00fff116 = 0xffff;
    FUN_00011454();
    FUN_00011712();
LAB_00011ea4:
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
        uVar3 = 0xffff;
LAB_00011ec4:
        FUN_000530de(uVar3);
        FUN_000111d0();
        FUN_000118c6();
        _DAT_00fff116 = 0xffff;
        FUN_00011454();
        FUN_00011712();
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) {
        uVar3 = 1;
        goto LAB_00011ec4;
      }
      if ((((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
         ((DAT_00ffdd22._0_1_ & 0x20) != 0)) {
        FUN_000530de();
        if (DAT_00fff0d0 == '\x02') {
          FUN_000120e4();
        }
        else if (DAT_00fff0d0 == '\x03') {
          FUN_000120e4();
        }
        else {
          if (DAT_00fff0d0 == '\x04') {
            iVar10 = DAT_00ffdd4c;
            if (DAT_00ffdd28 != 0) {
              iVar10 = _DAT_00ffdd48;
            }
            cVar11 = (char)_DAT_00ffdd2a;
            _DAT_00fff0d0 = uVar2;
            *(char *)(DAT_00fffc04 + 0x4f) = *(char *)(DAT_00fffc04 + 0x4f) - cVar11;
            *(char *)(iVar10 + 0x4f) = cVar11 + *(char *)(iVar10 + 0x4f);
            FUN_00012c36();
            FUN_0000c0da();
            goto LAB_00011cb6;
          }
          FUN_0001209a();
        }
        goto LAB_00011ea4;
      }
    } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
  }
  else if (DAT_00fff0d0 == '\x04') {
    FUN_00012c36();
    uVar4 = 0;
    uVar5 = 0;
    sVar6 = 2;
    puVar7 = &DAT_00ff0100;
    do {
      if ((*(byte *)(puVar7 + 8) & 1) != 0) {
        uVar5 = uVar5 + 1;
        uVar4 = (uint)(byte)(*(char *)((int)puVar7 + 0x4f) + (char)uVar4);
      }
      puVar7 = puVar7 + 0x80;
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
    if (-1 < (short)(uVar5 - 2)) {
      if ((short)uVar4 != 0) {
        sVar6 = 2;
        puVar7 = &DAT_00ff0100;
        do {
          *(char *)((int)puVar7 + 0x4f) = (char)(uVar4 / uVar5);
          if ((*(byte *)(puVar7 + 8) & 2) != 0) {
            *(char *)((int)puVar7 + 0x4f) =
                 (char)(uVar4 % (uint)uVar5) + *(char *)((int)puVar7 + 0x4f);
          }
          puVar7 = puVar7 + 0x80;
          sVar6 = sVar6 + -1;
        } while (sVar6 != -1);
      }
      FUN_00012c36();
      FUN_0000c0da();
      goto LAB_00011cb6;
    }
LAB_00011e14:
    FUN_0000c0da();
    goto LAB_00011cb6;
  }
LAB_00011c9c:
  _DAT_00fff0d0 = uVar2;
  FUN_00012c36();
  FUN_0000c0da();
LAB_00011cb6:
  FUN_000111d0();
  FUN_00000d94();
  FUN_00011928();
  return;
}


