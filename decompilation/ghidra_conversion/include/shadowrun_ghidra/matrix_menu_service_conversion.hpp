#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kMatrixLoadedSlotCount = 5U;

// ROM 0x19632: clear one loaded-program slot after a command completes.
// The source bytes are kept as semantic state here; the source addresses are
// provenance only and are not runtime storage requirements.
struct MatrixProgramSlotCleanupState {
    std::array<std::uint8_t, kMatrixLoadedSlotCount> program_codes{};
    std::array<std::uint8_t, kMatrixLoadedSlotCount> visual_states{};
    std::uint8_t menu_flags = 0;
    std::uint8_t pending_status = 0;
    std::uint8_t secondary_status = 0;
    std::uint8_t effect_marker = 0;
};

struct MatrixProgramSlotCleanupCallbacks {
    std::function<void()> write_repeated_grid;
    std::function<void()> write_grid;
    std::function<void()> render_rows;
};

bool clear_matrix_program_slot(
    MatrixProgramSlotCleanupState& state,
    std::size_t slot,
    const MatrixProgramSlotCleanupCallbacks& callbacks);

enum class MatrixProgramLoadingEvent {
    none,
    overlay_refreshed,
    slot_advanced,
    slot_completed,
};

// ROM 0x19788: advance the menu overlay timer and one five-slot loading
// animation step. The source uses FFF0ED as five visual states, FFDD48 as the
// signed inter-step delay, and FFF0F2 as the per-slot delay table.
struct MatrixProgramLoadingState {
    std::uint8_t overlay_countdown = 0;
    bool loading_active = false;
    std::int16_t progress_delay = 0;
    std::uint8_t selected_slot = 0;
    std::array<std::uint8_t, kMatrixLoadedSlotCount> visual_states{};
    std::array<std::int16_t, kMatrixLoadedSlotCount> slot_delays{};
};

struct MatrixProgramLoadingCallbacks {
    std::function<void()> render_rows;
    std::function<void()> advance_overlay_timer;
    std::function<void(std::size_t)> write_slot_grid;
    std::function<void(std::size_t)> publish_completed_selection;
    std::function<void()> render_selection_text;
};

MatrixProgramLoadingEvent advance_matrix_program_loading(
    MatrixProgramLoadingState& state,
    const MatrixProgramLoadingCallbacks& callbacks);

enum class HudModeTransitionEvent {
    unchanged,
    rendered,
    audio_published,
    completed_transition,
};

// ROM 0x1989A: normalize the signed -1/0/1 HUD-mode variant and perform the
// source's completion transition when the value is driven upward.
struct HudModeTransitionState {
    std::int8_t mode_variant = 0;       // FFF0A0
    std::int8_t transition_gate = 0;    // FFF0A6
    std::int8_t published_variant = 0; // FFE189
};

struct HudModeTransitionCallbacks {
    std::function<void()> advance_fade;
    std::function<void()> run_world_effects;
    std::function<void()> render_mode_text;
    std::function<void()> wait_for_vblank;
    std::function<void()> wait_for_controller;
    std::function<void(std::int8_t)> publish_audio_variant;
};

HudModeTransitionEvent update_hud_mode_transition(
    HudModeTransitionState& state,
    std::int8_t delta,
    const HudModeTransitionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
