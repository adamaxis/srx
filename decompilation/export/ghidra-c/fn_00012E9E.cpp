// Ghidra headless export
// Entry: 0x00012E9E
// Name: service_entity_action_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00012148 read_entity_action_code
//   0x00012F50 render_entity_action_menu


void service_entity_action_menu(void)

{
  read_entity_action_code();
  render_entity_action_menu();
  return;
}


