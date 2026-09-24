// Ghidra headless export
// Entry: 0x0001789C
// Name: prepare_menu_hud_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000178C8 write_menu_hud_grid_batch
//   0x00017998 FUN_00017998
//   0x00017A5E FUN_00017a5e
//   0x00017BC0 refresh_menu_number_or_grid
//   0x0001989A FUN_0001989a


void prepare_menu_hud_sequence(void)

{
  write_menu_hud_grid_batch();
  write_menu_hud_grid_batch();
  refresh_menu_number_or_grid();
  FUN_0001989a();
  FUN_00017a5e();
  FUN_00017998();
  return;
}


