// Ghidra headless export
// Entry: 0x000566C2
// Name: copy_mode_selection_state_table
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_mode_selection_state_table(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  sVar1 = 0x17;
  puVar2 = &DAT_0018ac56;
  puVar3 = &DAT_00ffd91e;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  return;
}


