// Ghidra headless export
// Entry: 0x00026A00
// Name: run_display_transition
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::run_display_transition.
// The original masked transfers are named host transition requests. Their
// source data is supplied as extracted assets; this function retains only the
// choreography, display-control boundaries, and input/timing cadence.
//
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001358 copy_source_word_block
//   0x0000195C sample_controller_input
//   0x00010952 reset_mode_display_buffers
//   0x00026BFE write_masked_transfer
//   0x00026C26 write_masked_transfer_with_display_control
//   0x000530DE service_audio_if_enabled

void run_display_transition(void)
{
  DAT_00fff0b1 = -1;
  service_audio_if_enabled(0x29);
  upload_initial_transition_asset();
  reset_mode_display_buffers();
  copy_transition_asset();
  DAT_00ffef14 |= 1;

  write_masked_transfer(initial_background, 40);
  write_masked_transfer_with_display_control(initial_display_panel, 40);
  write_masked_transfer(initial_overlay, 1);

  for (int iteration = 0; iteration < 14; ++iteration) {
    tick_transition_input();
    write_masked_transfer(sweep_primary, 1);
    write_masked_transfer(sweep_secondary, 1);
    write_masked_transfer_with_display_control(sweep_display_primary, 1);
    write_masked_transfer_with_display_control(sweep_display_secondary, 1);
  }
  for (int iteration = 0; iteration < 6; ++iteration) {
    tick_transition_input();
    write_masked_transfer(settle_primary, 1);
    write_masked_transfer(settle_secondary, 1);
  }
  write_masked_transfer(settle_overlay, 1);
  wait_transition_ticks(11);
  write_masked_transfer(clear_one_overlay, 1);
  wait_transition_ticks(6);
  write_masked_transfer(clear_two_overlay, 1);
  wait_transition_ticks(6);
  write_masked_transfer(clear_three_overlay, 1);
  wait_transition_ticks(11);
}
