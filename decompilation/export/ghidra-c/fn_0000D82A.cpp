// Ghidra headless export
// Entry: 0x0000D82A
// Name: FUN_0000d82a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x0000C0DA render_title_text_source
//   0x0000C216 FUN_0000c216
//   0x0000DA4A FUN_0000da4a
//   0x00012C5A FUN_00012c5a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000d82a(void)

{
  char cVar1;
  short sVar2;
  undefined4 uVar3;
  short sVar4;
  undefined3 uVar5;
  char extraout_D1b;
  uint uVar6;
  ushort uVar7;
  int iVar8;
  char *pcVar9;
  undefined4 extraout_A1;
  undefined4 extraout_A1_00;
  undefined4 uVar10;
  undefined8 uVar11;
  
  DAT_00fff0b3 = 0;
  FUN_00012c5a();
  uVar7 = (ushort)DAT_00fff0d1;
  if ((char)(DAT_00fff0d1 - 4) < '\0') {
    sVar2 = uVar7 + (ushort)DAT_00fff0d0 * 4;
    if ((_DAT_00fff116 < 0) || (-1 < (short)(sVar2 + -6))) goto LAB_0000da00;
    DAT_00fff0b6 = *(char *)(DAT_00fffc04 + 0x77 + (int)sVar2);
    uVar3 = FUN_0000da4a();
    uVar10 = extraout_A1;
    cVar1 = extraout_D1b;
    if ((char)(&DAT_000c22a6)[(int)(short)uVar3 + (uint)*(byte *)(DAT_00fffc04 + 0x9a) * 6] <
        extraout_D1b) {
LAB_0000d9de:
      DAT_00fff0b3 = 0xff;
      render_title_text_source(uVar10);
      goto LAB_0000da00;
    }
  }
  else {
    sVar2 = uVar7 - 4;
    if ((_DAT_00fff116 < 0) ||
       (((ushort)(sVar2 + (ushort)DAT_00fff0d0 * 6) == 0 &&
        (-1 < (char)(*(char *)(DAT_00fffc04 + 0x9b) + -2))))) goto LAB_0000da00;
    uVar11 = FUN_0000da4a();
    uVar3 = (undefined4)((ulonglong)uVar11 >> 0x20);
    iVar8 = DAT_00fffc04 + 0x80;
    sVar4 = (short)((ulonglong)uVar11 >> 0x20);
    if (-1 < (short)(sVar4 + -6)) {
      iVar8 = DAT_00fffc04 + 0x81;
    }
    DAT_00fff0b6 = *(char *)(iVar8 + sVar4);
    uVar5 = (undefined3)((ulonglong)uVar11 >> 8);
    cVar1 = *(char *)(iVar8 + sVar4) + '\x01';
    uVar6 = CONCAT31(uVar5,cVar1);
    uVar10 = extraout_A1_00;
    if (-1 < (char)(*(char *)(iVar8 + sVar4) + -0xc)) goto LAB_0000d9de;
    if ((((DAT_00fff0d0 != 0) || ((short)(uVar7 - 6) < 0)) || (sVar2 == 5)) || (6 < sVar2)) {
      cVar1 = cVar1 * '\x02';
      goto LAB_0000d9a4;
    }
    if (sVar2 == 6) {
      if (DAT_00fff0b6 == '\0') {
        pcVar9 = (char *)(DAT_00fffc04 + 0x85);
        goto LAB_0000d97e;
      }
    }
    else if (DAT_00fff0b6 == '\0') {
      pcVar9 = (char *)(DAT_00fffc04 + 0x81);
LAB_0000d97e:
      DAT_00fff0b6 = *pcVar9;
      uVar6 = CONCAT31(uVar5,DAT_00fff0b6 + '\x01');
    }
    uVar6 = (uVar6 & 0xffff) * 0xf;
    cVar1 = (char)(uVar6 / 10);
    if ((short)(uVar6 % 10) != 0) {
      cVar1 = cVar1 + '\x01';
    }
  }
LAB_0000d9a4:
  DAT_00fff0b8 = cVar1;
  DAT_00fff0b6 = DAT_00fff0b6 + '\x01';
  DAT_00fff0b7 = (undefined1)uVar3;
  FUN_0000c216();
  FUN_0000c216();
LAB_0000da00:
  render_title_text_source();
  write_repeated_vdp_grid();
  FUN_0000c216();
  return;
}


