// Ghidra headless export
// Entry: 0x00015A8C
// Name: FUN_00015a8c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short FUN_00015a8c(void)

{
  short sVar1;
  short sVar2;
  undefined *puVar3;
  
  puVar3 = &DAT_00ff0b00;
  sVar2 = 5;
  sVar1 = 9;
  do {
    if ((puVar3[0x10] & 1) == 0) {
      return sVar1;
    }
    puVar3 = puVar3 + 0x100;
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return sVar1;
}


