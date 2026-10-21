#include "shadowrun_ghidra/world/world_entry_sequence_conversion.hpp"

#include <algorithm>

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
    const auto plan = callbacks.select_sequence_plan
        ? callbacks.select_sequence_plan(compact_layout)
        : WorldEntrySequencePlan{};
    const auto render_frames = static_cast<std::uint16_t>(std::count_if(
        plan.steps.begin(), plan.steps.end(), [](const WorldEntryStep& step) {
            return step.kind == WorldEntryStepKind::render_frame;
        }));
    if (render_frames != plan.frame_count) {
        return WorldEntrySequenceAction::invalid_plan;
    }

    state.animation_marker = -1;
    std::uint16_t completed_frames = 0U;
    bool begin_service_cycle = true;

    for (const auto& step : plan.steps) {
        if (begin_service_cycle) {
            run_timing_cycle(callbacks, 2U);
        }
        begin_service_cycle = true;

        if (step.kind == WorldEntryStepKind::camera_adjustment) {
            state.world_origin_x -= step.camera_step_x;
            state.world_origin_y -= step.camera_step_y;
            // ROM 0x26834 clears D5 and branches to 0x267F0, so DBF waits
            // once before the next command. It skips the 0x267EE setup that
            // would start another three-VBlank cycle.
            run_timing_cycle(callbacks, 0U);
            begin_service_cycle = false;
            continue;
        }

        if (step.kind == WorldEntryStepKind::delay) {
            run_timing_cycle(callbacks, step.additional_wait_count);
            // ROM 0x26848 branches to 0x267F0, past the moveq that starts
            // the normal three-VBlank service cycle. Once this delay expires,
            // the next command is consumed immediately.
            begin_service_cycle = false;
            continue;
        }

        // Offline ROM analysis resolves each packed sample into these typed
        // counts. Runtime rendering consumes the plan without decoding ROM
        // command bytes.
        state.accumulated_x += step.sample_count_x;
        state.accumulated_y += step.sample_count_y;
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
    return completed_frames == plan.frame_count
        ? WorldEntrySequenceAction::completed
        : WorldEntrySequenceAction::invalid_plan;
}

} // namespace shadowrun::ghidra_conversion
