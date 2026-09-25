#include "shadowrun_ghidra/world_entry_sequence_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void run_timing_cycle(const WorldEntrySequenceCallbacks& callbacks,
                      const std::uint16_t cycles) {
    for (std::uint16_t cycle = 0; cycle <= cycles; ++cycle) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        if (callbacks.advance_hud_sprite_audio_pulse) {
            callbacks.advance_hud_sprite_audio_pulse();
        }
        if (callbacks.advance_menu_random_glitch) {
            callbacks.advance_menu_random_glitch();
        }
    }
}

} // namespace

WorldEntrySequenceAction run_world_entry_sequence(
    WorldEntrySequenceState& state,
    const bool compact_layout,
    const WorldEntrySequenceCallbacks& callbacks) {
    state.animation_marker = -1;
    const auto stream = callbacks.select_command_stream
        ? callbacks.select_command_stream(compact_layout)
        : WorldEntryCommandStream{};
    std::size_t command_cursor = 0U;
    std::size_t delay_cursor = 0U;
    std::uint16_t completed_frames = 0U;

    while (completed_frames < stream.frame_count) {
        run_timing_cycle(callbacks, 2U);
        if (command_cursor >= stream.movement_commands.size()) {
            return WorldEntrySequenceAction::invalid_stream;
        }

        const auto command = stream.movement_commands[command_cursor++];
        if ((command & 0x80U) != 0U) {
            state.world_origin_x -= static_cast<std::int32_t>((command >> 4U) & 7U);
            state.world_origin_y -= static_cast<std::int32_t>(command & 7U);
            continue;
        }

        state.accumulated_x += static_cast<std::int32_t>(command >> 4U);
        if (command_cursor >= stream.movement_commands.size()) {
            return WorldEntrySequenceAction::invalid_stream;
        }
        const auto vertical = stream.movement_commands[command_cursor++];
        if (vertical == 0U) {
            if (delay_cursor >= stream.delays.size()) {
                return WorldEntrySequenceAction::invalid_stream;
            }
            run_timing_cycle(callbacks, stream.delays[delay_cursor++]);
            continue;
        }

        state.accumulated_y += static_cast<std::int32_t>(vertical & 0x0FU);
        if (callbacks.dispatch_frame_buffer) {
            callbacks.dispatch_frame_buffer();
        }
        if (callbacks.write_frame_transfer) {
            callbacks.write_frame_transfer();
        }
        state.display_transfer_pending = true;
        if (callbacks.advance_title_sprite_layers) {
            callbacks.advance_title_sprite_layers();
        }
        if (callbacks.append_world_sprite_records) {
            callbacks.append_world_sprite_records();
        }
        state.sprite_records_pending = true;
        ++completed_frames;
    }
    return WorldEntrySequenceAction::completed;
}

} // namespace shadowrun::ghidra_conversion
