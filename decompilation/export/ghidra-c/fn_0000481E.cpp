// Ghidra headless export
// Entry: 0x0000481E
// Name: FUN_0000481e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x00005A62 FUN_00005a62


void FUN_0000481e(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  undefined2 *extraout_A0_00;
  
  puVar2 = &DAT_00ff164c;
  sVar1 = 9;
  do {
    if (((*(byte *)(puVar2 + 8) & 1) != 0) &&
       (FUN_00005a62(), puVar2 = extraout_A0, extraout_A0[6] != 0)) {
      FUN_00001378();
      puVar2 = extraout_A0_00;
    }
    puVar2 = puVar2 + 9;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


