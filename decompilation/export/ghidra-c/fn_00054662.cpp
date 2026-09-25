// Ghidra headless export
// Entry: 0x00054662
// Name: run_mode_action_loop
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::run_mode_action_loop.
// This is the mode-entry action loop reached from the world transition at
// 0x1ADE6 when the alternate runner presenter is inactive. It prepares the
// mode resources, redraws the ten source grid services, handles the action
// gate and unresolved record-kind dispatch, and hands the Start/exit path to
// the converted Matrix loop at 0x1822C. Host presentation and input are typed
// callbacks; no VDP or ROM address arithmetic is runtime behavior.
//
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x00000DBA count_thresholded_random_rolls
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001232 write_vdp_grid
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x00010952 reset_mode_display_buffers
//   0x000176EA render_menu_footer
//   0x000177CC upload_menu_resource_pair
//   0x00017F80 refresh_mode_metadata_wrapper
//   0x0001822C convert_world_routine_0x0001822c (Matrix program loop)
//   0x000532A8 fill_mode_auxiliary_sentinel
//   0x000549E4 write_mode_repeated_grid_service
//   0x000549F6 select_mode_display_status
//   0x00054A28 render_mode_status_summary
//   0x00054A8E advance_random_mode_event
//   0x00055F44 run_mode_action_preamble
//   0x000566F6 refresh_mode_resource_state

// Source order, expressed as native operations:
//
//   initialize_mode_flags();
//   for (;;) {
//       redraw_mode_resources_and_grid();
//       if (advance_random_mode_event()) continue;
//       if (mode_exit_marker_is_negative()) return;
//       render_mode_status_fields();
//       reset_mode_cursor();
//       for (;;) {
//           wait_and_sample_controller();
//           if (directional_edge()) continue;
//           if (action_edge()) {
//               run_mode_action_preamble();
//               if (action_gate_passes())
//                   dispatch_mode_action(record_kind, cursor);
//               else
//                   show_mode_action_fallback();
//               ++mode_random_count;
//               break;
//           }
//           if (start_edge()) {
//               enter_matrix_program_loop();
//               reset_matrix_slot_timers();
//               break;
//           }
//       }
//   }
