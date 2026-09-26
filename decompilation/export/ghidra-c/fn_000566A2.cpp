// Ghidra headless export
// Entry: 0x000566A2
// Name: copy_action_selection_profile_table
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_action_selection_profile_table(void)

{
  short sVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  puVar5 = &DAT_00fff9fa;
  puVar3 = &DAT_00166ad8;
  sVar1 = 4;
  do {
    sVar2 = 0x10;
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

