// Ghidra headless export
// Entry: 0x000053C4
// Name: FUN_000053c4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00005468 FUN_00005468
//   0x00005494 FUN_00005494
//   0x00005622 FUN_00005622
//   0x0000AFF2 FUN_0000aff2
//   0x0001A304 FUN_0001a304


void FUN_000053c4(void)

{
  short sVar1;
  int extraout_A0;
  undefined4 extraout_A0_00;
  int iVar2;
  
  if (-1 < DAT_00ffdff6) {
    DAT_00ffdff6 = DAT_00ffdff6 + -1;
    if (-1 < DAT_00ffdff6) {
      return;
    }
    DAT_00ffdff8 = DAT_00ffdff8 + -1;
  }
  FUN_00005494();
  if (*(char *)(extraout_A0 + 0xb2) != '\0') {
    return;
  }
  sVar1 = 8;
  iVar2 = extraout_A0;
  do {
    if (8 < *(short *)(iVar2 + 0x13e)) break;
    if (((*(char *)(iVar2 + 0x1de) == '\r') && ((*(byte *)(iVar2 + 0x110) & 1) != 0)) &&
       ((*(byte *)(iVar2 + 0x110) & 0x40) == 0)) {
      FUN_00005468();
      FUN_0000aff2(extraout_A0_00);
      return;
    }
    sVar1 = sVar1 + -1;
    iVar2 = iVar2 + 0x100;
  } while (sVar1 != -1);
  if (-1 < DAT_00ffdff8) {
    DAT_00ffdff6 = 0x1e;
    FUN_00005468();
    return;
  }
  FUN_0001a304();
  FUN_00005622();
  return;
}


