// Ghidra headless export
// Entry: 0x0001CB6A
// Name: FUN_0001cb6a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001CBF8 FUN_0001cbf8
//   0x000530DE FUN_000530de


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cb6a(void)

{
  short sVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short sVar3;
  short unaff_D5w;
  short unaff_D6w;
  int extraout_A0;
  undefined *extraout_A1;
  undefined *puVar4;
  int extraout_A1_00;
  
  if (_DAT_00ffe010 == 0) {
    puVar4 = &DAT_00ff159e;
  }
  else {
    FUN_0001cbf8();
    puVar4 = extraout_A1;
  }
  if (puVar4[6] != '\0') {
    return;
  }
  FUN_000530de();
  *(undefined2 *)(extraout_A1_00 + 0x12) = extraout_D1w;
  *(undefined1 *)(extraout_A1_00 + 6) = 0xff;
  *(undefined1 *)(extraout_A1_00 + 10) = 0;
  *(undefined1 *)(extraout_A1_00 + 9) = 0;
  *(undefined1 *)(extraout_A1_00 + 5) = 0;
  *(undefined1 *)(extraout_A1_00 + 4) = 0x14;
  sVar1 = *(short *)(extraout_A0 + 0xe) + unaff_D5w;
  sVar2 = *(short *)(extraout_A0 + 0x10) + unaff_D6w;
  *(short *)(extraout_A1_00 + 0x10) = sVar1 * 0x10;
  *(short *)(extraout_A1_00 + 2) = sVar2 * 0x10;
  sVar3 = DAT_00ff1598 * 0x10;
  *(short *)(extraout_A1_00 + 0xc) = (short)(DAT_00ff1596 * 0x10 + sVar1 * -0x10) / 0x3c;
  *(short *)(extraout_A1_00 + 0xe) = (short)(sVar3 + sVar2 * -0x10) / 0x3c;
  *(undefined1 *)(extraout_A1_00 + 0xb) = 0x3c;
  return;
}


