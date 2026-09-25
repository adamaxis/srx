// Ghidra headless export
// Entry: 0x0001F3C4
// Name: run_mode_text_prompt
// Decompile status: source-shaped annotation
//
// This is the mode-exit prompt loop. The native conversion lives in
// ghidra_conversion::run_mode_text_prompt; this file preserves the ROM order
// and the source-derived controller/command semantics for cross-checking.
//
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000C24 wait_for_vblank_frames
//   0x00000D94 wait_for_controller_event
//   0x00001232 write_vdp_grid
//   0x0000195C sample_controller_input
//   0x0000C134 normalize_source_text_line
//   0x0000C168 convert_graphics_routine_0x0000c168
//   0x0001F5A8 prepare_mode_text_prompt
//   indirect 0x0012CCB4 mode-command dispatch table

void run_mode_text_prompt(void)
{
  // A0 points at four text entries followed by three longword commands.
  // FFF10C means that the caller has already prepared the prompt display.
  for (;;) {
    if (DAT_00fff10c == 0) {
      prepare_mode_text_prompt();
      render_mode_prompt_text(/* title entry */, /* title layout */);
    }
    DAT_00fff10c = 0;

    if (first_option_text_entry_is_negative()) {
      wait_for_vblank_frames(/* D5 = 0x1E */);
      wait_for_controller_event();
      return_command(/* 0xFB */);
    }

    render_mode_prompt_text(/* first option */, /* first option layout */);
    write_vdp_grid(/* first option grid */);

    const bool second_option_available =
        render_mode_prompt_option_if_present(/* second option */);
    const bool third_option_available =
        render_mode_prompt_option_if_present(/* third option */);

    wait_for_vblank_frames(/* D5 = 0x0A */);
    for (;;) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();

      // btst.b #0xE/#0xC/#0xD on FFDD22 maps to masks 0x40/0x10/0x20.
      // First has priority; the other two require a present option.
      if (controller_mask() & 0x40) {
        break_with_command(/* command 0 */);
      }
      if ((controller_mask() & 0x10) && second_option_available) {
        break_with_command(/* command 1 */);
      }
      if ((controller_mask() & 0x20) && third_option_available) {
        break_with_command(/* command 2 */);
      }
    }

    const uint32_t command = selected_command();
    if (command >= 0xFA) {
      return_command(command);
    }

    // The command table is an explicit native callback boundary. After its
    // handler returns, the source branches back to the prompt entry.
    dispatch_mode_command(command);
  }
}
