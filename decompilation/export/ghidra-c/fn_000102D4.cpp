// Ghidra headless export
// Entry: 0x000102D4
// Name: prepare_magic_action_details
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00010260 prepare_menu_grid_then_service
//   0x000107CE adjust_runner_posture
//   0x0001088C render_magic_spell_availability
//
// Source text roles are distinct: C8A4A is the `Set Force & Posture.`
// heading, C89FC is the `Success` label, and C8A3A is the `Force` label.


void prepare_magic_action_details(void)

{
  prepare_menu_grid_then_service();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  adjust_runner_posture();
  render_magic_spell_availability();
  build_grouped_decimal_glyphs();
  return;
}


