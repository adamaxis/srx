// Ghidra headless export
// Entry: 0x00019DF2
// Name: run_menu_metadata_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C3C read_menu_entry_profile_eligibility
//   0x00019DD8 refresh_mode_metadata_leaf


void run_menu_metadata_service(void)

{
  read_menu_entry_profile_eligibility();
  refresh_mode_metadata_leaf();
  return;
}


