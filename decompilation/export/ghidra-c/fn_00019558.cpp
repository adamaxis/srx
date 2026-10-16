// Ghidra headless export
// Entry: 0x00019558
// Name: calculate_matrix_storage_capacity
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C matrix_program_storage_usage
//   0x0001954A matrix_loaded_slot_usage


void calculate_matrix_storage_capacity(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    matrix_loaded_slot_usage();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 0xb;
  do {
    matrix_program_storage_usage();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


