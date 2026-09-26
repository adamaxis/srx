#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

enum class WorldEffectOverlayKind {
    primary,
    secondary,
    background,
};

struct WorldEffectOverlayState {
    bool scene_complete = false;       // FFE084
    std::uint16_t scene_completion_aux = 0; // FFE086

    std::uint16_t primary_started = 0;  // FFE0B6
    std::uint16_t primary_position = 0; // FFE0BE
    std::uint16_t secondary_started = 0; // FFE0B8
    std::uint16_t secondary_position = 0; // FFE0BA
    std::uint16_t background_started = 0; // FFE0C2
    std::uint16_t background_position = 0; // FFE0C6

    std::uint16_t source_x = 0;         // FFE056
    std::uint16_t motion_step = 0;      // FFE0C0
    std::uint16_t background_step = 0;  // FFE0C4
    std::uint16_t frame_delay = 0;      // FFE0B4
    std::uint16_t phase = 0;            // FFE0B2
    std::uint16_t threshold_control = 0; // FFE0C8
    bool intro_marker = false;          // FFF0A3
    bool descriptor_stream_ready = false; // FFE0F4
};

struct WorldEffectSpriteBuild {
    WorldEffectOverlayKind kind = WorldEffectOverlayKind::primary;
    std::uint16_t sprite_key = 0x07A8U;
    std::uint16_t attribute = 0;
    std::uint16_t projected_position = 0;
    std::uint16_t build_flags = 0x2000U;
    // The source selects one longword table entry and passes entry +8 to the
    // title sprite builder. The token is data, not a native ROM address.
    std::uint32_t sprite_data_token = 0;
};

struct WorldEffectOverlayCallbacks {
    // ROM 0x1AC50: returns the source carry result after testing the fixed
    // 0x7140 threshold. A true result also publishes FFE084/FFE086.
    std::function<bool(std::uint16_t source_position)>
        arm_after_threshold;
    // ROM 0x563CC: consume a pending thresholded roll before the primary
    // overlay's frame cadence continues.
    std::function<void()> advance_pending_roll;
    std::function<void()> begin_descriptor_transition;
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    // Resolve the selected phase in ROM tables 0xF1BDE, 0xF1BCE, or 0xF1BBE.
    std::function<std::optional<std::uint32_t>(
        WorldEffectOverlayKind kind, std::uint16_t phase)>
        resolve_sprite_data;
    std::function<void(const WorldEffectSpriteBuild&)> build_sprite;
};

enum class WorldEffectOverlayAction {
    no_action,
    descriptor_transition_started,
    intro_transition_requested,
    sprite_rendered,
    sprite_data_unresolved,
};

// ROM 0x1AB6E: advance the primary effect overlay, including the 0x6B00
// descriptor-transition cutoff, threshold gate, 9-frame cadence, and phase
// table selection.
WorldEffectOverlayAction advance_primary_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks);

// ROM 0x1AC6A: advance the secondary effect overlay. Its threshold result is
// intentionally not used as a branch, matching the source carry handling.
WorldEffectOverlayAction advance_secondary_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks);

// ROM 0x1AD42: advance the four-phase background overlay and use the source
// intro marker to clear its active flag.
WorldEffectOverlayAction advance_background_world_effect_overlay(
    WorldEffectOverlayState& state,
    const WorldEffectOverlayCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
