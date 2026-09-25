// Ghidra headless export
// Entry: 0x00019E32
// Name: run_menu_selection_service
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x00017E04 refresh_hud_grid_if_ready
//   0x0001B706 prepare_menu_offset_once
//   0x00056434 FUN_00056434


void run_menu_selection_service(void)

{
  set_hud_selection_value();
  render_menu_text_followup();
  if ('\0' < DAT_00fff0d5) {
    prepare_menu_offset_once();
    FUN_00056434();
    refresh_hud_grid_if_ready();
  }
  prepare_menu_offset_once();
  return;
}


