// Ghidra headless export
// Entry: 0x00004618
// Name: FUN_00004618
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004658 FUN_00004658
//   0x00005A56 FUN_00005a56
//   0x00005FDC FUN_00005fdc
//   0x0000600E FUN_0000600e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00004618(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  _DAT_00ffde88 = 1;
  sVar1 = 0xf;
  do {
    if ((*(byte *)(puVar2 + 8) & 1) != 0) {
      FUN_00005a56();
      FUN_00005fdc();
      FUN_0000600e();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00004658();
  _DAT_00ffde88 = 0;
  return;
}


