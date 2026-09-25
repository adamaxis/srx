// Ghidra headless export
// Entry: 0x0001438C
// Name: convert_entity_routine_0x0001438c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_entity_routine_0x0001438c(void)

{
  short sVar1;
  undefined *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar2 = &DAT_00ff0400;
  DAT_00ffdf94 = 0;
  sVar1 = 6;
  puVar3 = &DAT_00ffdf96;
  do {
    puVar4 = puVar3;
    if (((((puVar2[0x10] & 1) != 0) && ((puVar2[0x10] & 0x40) == 0)) && ((puVar2[0x10] & 0x10) == 0)
        ) && ((((puVar2[0xb0] & 0x80) == 0 && ((puVar2[0x9e] & 0x20) == 0)) &&
              (((puVar2[0x9e] & 8) == 0 && (*(short *)(puVar2 + 0xc) != 0)))))) {
      DAT_00ffdf94 = DAT_00ffdf94 + 1;
      puVar4 = puVar3 + 1;
      *puVar3 = puVar2;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
    puVar3 = puVar4;
  } while (sVar1 != -1);
  return;
}


