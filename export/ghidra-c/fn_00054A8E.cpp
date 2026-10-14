// Ghidra headless export
// Entry: 0x00054A8E
// Name: advance_random_mode_event
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::advance_random_mode_event.
// The counter comparison is the source event gate. When it fires, the
// routine clears the counter, advances the HUD mode, optionally renders the
// zero-mode acknowledgement/wait path, and always submits the repeated-grid
// refresh before returning to the outer 0x54662 loop.
//
// Calls recovered from direct call instructions:
//   0x00000C24 wait_for_vblank_frames
//   0x00000D3A sample_source_random_bound
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x0001989A update_hud_mode_transition
//   0x000549E4 write_mode_repeated_grid_service
//   0x000549F6 select_mode_display_status
