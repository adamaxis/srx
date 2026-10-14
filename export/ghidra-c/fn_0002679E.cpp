// Ghidra headless export
// Entry: 0x0002679E
// Name: run_world_entry_sequence
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::run_world_entry_sequence.
// The two source command tables are extracted as host streams. Timing,
// signed movement corrections, delay records, and frame publication order
// remain observable policy; VDP pointers do not.
//
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000F98 write_vdp_word_block
//   0x00017CC8 advance_hud_sprite_audio_pulse
//   0x00017FDA advance_menu_random_glitch
//   0x0001C3FA advance_title_sprite_layers
//   0x00024E20 dispatch_indexed_handler
//   0x000268E6 append_world_sprite_records

void run_world_entry_sequence(void)
{
  DAT_00fff0e1 = -1;
  const auto command_stream = select_extracted_world_entry_stream();
  std::size_t command_cursor = 0;
  std::size_t delay_cursor = 0;
  std::uint16_t completed_frames = 0;
  while (completed_frames < command_stream.frame_count) {
    run_three_source_timing_cycles();
    const auto command = command_stream.movement_commands[command_cursor++];
    if ((command & 0x80) != 0) {
      FFDD28 -= (command >> 4) & 7;
      FFDD2A -= command & 7;
      continue;
    }
    FFE1A2 += command >> 4;
    const auto vertical = command_stream.movement_commands[command_cursor++];
    if (vertical == 0) {
      run_source_delay_cycles(command_stream.delays[delay_cursor++]);
      continue;
    }
    FFE1A4 += vertical & 0x0F;
    dispatch_indexed_handler(frame_buffer_stage);
    write_vdp_word_block();
    FFDD02 = 1;
    advance_title_sprite_layers();
    append_world_sprite_records();
    FFD980 = 1;
    DAT_00ffef14 |= 2;
    ++completed_frames;
  }
}
