// Ghidra headless export
// Entry: 0x00014446
// Name: FUN_00014446
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00014446(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar2 = &DAT_00ff0100;
  DAT_00ffdfc8 = 0;
  sVar1 = 2;
  puVar3 = &DAT_00ffdfca;
  do {
    puVar4 = puVar3;
    if (((((*(byte *)(puVar2 + 8) & 1) != 0) && ((*(byte *)(puVar2 + 8) & 0x40) == 0)) &&
        ((*(byte *)(puVar2 + 8) & 0x10) == 0)) && (puVar2[6] != 0)) {
      DAT_00ffdfc8 = DAT_00ffdfc8 + 1;
      puVar4 = puVar3 + 1;
      *puVar3 = puVar2;
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
    puVar3 = puVar4;
  } while (sVar1 != -1);
  return;
}


