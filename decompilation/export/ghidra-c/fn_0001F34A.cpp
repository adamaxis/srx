// Ghidra headless export
// Entry: 0x0001F34A
// Name: prepare_mode_resource_variant_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003648 draw_hud_panel
//   0x000566F6 refresh_mode_resource_state


void prepare_mode_resource_variant_b(void)

{
  refresh_mode_resource_state();
  draw_hud_panel();
  return;
}


