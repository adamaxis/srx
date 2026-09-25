// Ghidra headless export
// Entry: 0x00026938
// Name: copy_two_word_rows
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_two_word_rows(void)

{
  short sVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  puVar5 = (undefined2 *)&DAT_00fffbbe;
  puVar3 = &DAT_0014c1f4;
  sVar1 = 0;
  do {
    sVar2 = 0x16;
    puVar4 = puVar3;
    puVar6 = puVar5;
    do {
      puVar3 = puVar4 + 1;
      puVar5 = puVar6 + 1;
      *puVar6 = *puVar4;
      sVar2 = sVar2 + -1;
      puVar4 = puVar3;
      puVar6 = puVar5;
    } while (sVar2 != -1);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


