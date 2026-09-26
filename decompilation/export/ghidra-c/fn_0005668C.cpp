// Ghidra headless export
// Entry: 0x0005668C
// Name: convert_intro_routine_0x0005668c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017E04 refresh_hud_grid_if_ready
//   0x00056434 advance_menu_selection_service_timer


void convert_intro_routine_0x0005668c(void)

{
    advance_menu_selection_service_timer();
  refresh_hud_grid_if_ready();
  return;
}

