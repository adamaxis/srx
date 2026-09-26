// Ghidra headless export
// Entry: 0x000575F8
// Name: prepare_field_action_resource_selection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C matrix_program_storage_usage
//   0x00019558 calculate_matrix_storage_capacity
//   0x00057610 prepare_field_action_selection_context


void prepare_field_action_resource_selection(void)

{
  calculate_matrix_storage_capacity();
  prepare_field_action_selection_context();
  matrix_program_storage_usage();
  return;
}


