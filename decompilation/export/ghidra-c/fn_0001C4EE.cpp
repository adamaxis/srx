// Ghidra headless export
// Entry: 0x0001C4EE
// Name: copy_eleven_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_eleven_words(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 10;
  puVar2 = &DAT_0001c4da;
  puVar3 = &DAT_00ff1580;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


