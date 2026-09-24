// Ghidra headless export
// Entry: 0x0001A342
// Name: FUN_0001a342
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short FUN_0001a342(void)

{
  short sVar1;
  short sVar2;
  undefined *puVar3;
  
  puVar3 = &DAT_00ff0400;
  sVar2 = 6;
  sVar1 = 3;
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


