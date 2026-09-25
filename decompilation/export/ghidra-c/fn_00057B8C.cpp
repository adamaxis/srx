// Ghidra headless export
// Entry: 0x00057B8C
// Name: run_field_action_quantity_chooser
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x000530DE service_audio_if_enabled
//
// This is the quantity/cost prompt entered after an action has passed the
// affordability check. FFF0DC is initialized to the active prompt state;
// movement runs through the common bounded cursor helper and the routine
// exits on the source confirm/cancel edge bits.

undefined8 run_field_action_quantity_chooser(void)
{
  undefined4 in_D0;
  undefined4 in_D1;

  render_encoded_title_text(); // quantity prompt label
  render_encoded_title_text(); // quantity value
  DAT_00fff0dc = 1;
  do {
    service_audio_if_enabled();
    move_wrapped_menu_cursor(); // D2 = -1 or +1, bounded quantity cursor
    while (true) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22 & 0x0100) != 0 ||
          (DAT_00ffdd22 & 0x0200) != 0) {
        break; // Up/down movement: redraw and continue polling
      }
      if ((DAT_00ffdd22 & 0x4000) != 0 ||
          (DAT_00ffdd22 & 0x1000) != 0) {
        service_audio_if_enabled();
        return CONCAT44(in_D0, in_D1);
      }
    }
  } while (true);
}
