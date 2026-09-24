// Ghidra headless export
// Entry: 0x00010260
// Name: prepare_menu_grid_then_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000101FE FUN_000101fe
//   0x00012C5A write_menu_repeated_grid_with_metadata


void prepare_menu_grid_then_service(void)

{
  write_menu_repeated_grid_with_metadata();
  FUN_000101fe();
  return;
}


