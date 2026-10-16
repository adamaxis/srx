// Ghidra headless export
// Entry: 0x00053704
// Name: run_runner_selection_screen
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::run_runner_selection_screen.
// This is the runner-selection controller reached from the world/title mode
// transition. It initializes the screen, refreshes the selected runner's
// sprite records, polls edge-triggered controller events, moves through the
// roster, publishes the selected runner and redraws the HUD, or returns to
// the menu footer. The host owns input, roster records, and presentation.
//
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000176EA render_menu_footer
//   0x00017708 render_field_hud
//   0x000177CC upload_menu_resource_pair
//   0x000532BC initialize_runner_selection_screen
//   0x000537FA refresh_runner_selection_sprites

int run_runner_selection_screen(void)
{
  initialize_runner_selection_screen();
  refresh_runner_selection_sprites();
  for (;;) {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    if (previous_runner_pressed()) {
      if (move_to_previous_runner()) {
        refresh_runner_selection_sprites();
      }
      continue;
    }
    if (runner_confirm_pressed()) {
      publish_runner_selection();
      if (!alternate_presenter_active()) {
        render_field_hud();
      }
      return -1;
    }
    if (next_runner_pressed()) {
      move_to_next_runner();
      refresh_runner_selection_sprites();
      continue;
    }
    if (return_to_menu_pressed() && !alternate_presenter_active()) {
      upload_menu_resource_pair();
      return render_menu_footer();
    }
  }
}
