// Ghidra headless export
// Entry: 0x00019DE8
// Name: run_menu_service_then_prepare
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001789C prepare_menu_hud_sequence
//   0x00019DF2 run_menu_metadata_service


void run_menu_service_then_prepare(void)

{
  run_menu_metadata_service();
  prepare_menu_hud_sequence();
  return;
}


