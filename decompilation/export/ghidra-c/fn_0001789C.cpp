// Ghidra headless export
// Entry: 0x0001789C
// Name: prepare_menu_hud_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000178C8 write_menu_hud_grid_batch
//   0x00017998 render_menu_text_progression
//   0x00017A5E render_menu_slot_status
//   0x00017BC0 refresh_menu_number_or_grid
//   0x0001989A update_hud_mode_transition


void prepare_menu_hud_sequence(void)

{
  write_menu_hud_grid_batch();
  write_menu_hud_grid_batch();
  refresh_menu_number_or_grid();
  update_hud_mode_transition();
  render_menu_slot_status();
  render_menu_text_progression();
  return;
}


