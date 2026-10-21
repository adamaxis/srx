#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class WorldEntryStepKind {
    camera_adjustment,
    delay,
    render_frame,
};

struct WorldEntryStep {
    WorldEntryStepKind kind = WorldEntryStepKind::render_frame;
    std::int32_t camera_step_x = 0;
    std::int32_t camera_step_y = 0;
    std::uint16_t sample_count_x = 0;
    std::uint16_t sample_count_y = 0;
    std::uint16_t additional_wait_count = 0;
};

struct WorldEntrySequencePlan {
    std::uint16_t frame_count = 0;
    std::vector<WorldEntryStep> steps;
};

struct WorldEntrySequenceState {
    std::int8_t animation_marker = -1; // FFF0E1
    std::int32_t world_origin_x = 0; // FFDD28
    std::int32_t world_origin_y = 0; // FFDD2A
    std::int32_t accumulated_x = 0; // FFE1A2
    std::int32_t accumulated_y = 0; // FFE1A4
    bool display_transfer_pending = false; // FFDD02/FFEF14
    bool sprite_records_pending = false; // FFD980
};

struct WorldEntrySequenceCallbacks {
    std::function<WorldEntrySequencePlan(bool compact_layout)>
        select_sequence_plan;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> advance_hud_sprite_audio_pulse;
    std::function<void()> advance_menu_random_glitch;
    std::function<void()> dispatch_frame_buffer;
    std::function<void()> write_frame_transfer;
    std::function<void()> advance_title_sprite_layers;
    std::function<void()> append_world_sprite_records;
};

enum class WorldEntrySequenceAction {
    completed,
    invalid_plan,
};

// ROM 0x2679E: apply a source-derived typed world-entry plan while preserving
// source timing and dispatch/transfer/sprite publication order.
WorldEntrySequenceAction run_world_entry_sequence(
    WorldEntrySequenceState& state,
    bool compact_layout,
    const WorldEntrySequenceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
