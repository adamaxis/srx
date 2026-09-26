// Ghidra headless export
// Entry: 0x0001957E
// Name: calculate_loaded_program_capacity
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C matrix_program_storage_usage


void calculate_loaded_program_capacity(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *extraout_A1;
  
  puVar2 = &DAT_00fffbbe;
  sVar1 = 4;
  do {
    if (-1 < (char)puVar2[sVar1 + 0x1a]) {
      matrix_program_storage_usage();
      puVar2 = extraout_A1;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


