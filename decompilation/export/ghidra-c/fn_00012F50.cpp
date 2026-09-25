// Ghidra headless export
// Entry: 0x00012F50
// Name: render_entity_action_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00013000 convert_menu_routine_0x00013000


void render_entity_action_menu(void)

{
  convert_menu_routine_0x00013000();
  render_encoded_title_text();
  write_vdp_grid();
  return;
}


