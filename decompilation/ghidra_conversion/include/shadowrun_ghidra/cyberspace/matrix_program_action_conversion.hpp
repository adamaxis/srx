#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class MatrixProgramActionEvent {
    ignored,
    action_dispatched,
    pending_action_cleared,
    selection_published,
    started_runner_action,
};

// ROM 0x19976: source state used by the selected Matrix program-action
// service. These are semantic host fields, not an emulated RAM image.
struct MatrixProgramActionState {
    std::array<std::int8_t, 5> program_codes{{-1, -1, -1, -1, -1}}; // FFBBD8
    std::array<std::uint8_t, 5> visual_states{};                     // FFF0ED
    std::array<std::uint8_t, 5> selection_values{};                   // FFF0E8
    std::uint8_t selected_slot = 0;                                  // FFF0D4
    std::uint8_t overlay_stage = 0;                                  // FFF0E4
    std::int8_t action_mode = 0;                                     // FFF0B7
    bool pending_action = false;                                      // FFF0B6
    std::uint16_t input_flags = 0;                                   // FFDD22
    std::uint8_t runner_record_field = 0;                             // FF1113+13
    std::array<std::uint8_t, 12> availability_flags{};               // FFBCC
    std::int8_t hud_mode_variant = 0;                                 // FFF0A0
};

struct MatrixProgramActionCallbacks {
    // The indirect table at ROM 0xE2E0A is intentionally unresolved.
    std::function<void(std::int8_t, std::int8_t)> dispatch_program_action;
    std::function<void()> reset_overlay;
    std::function<void(std::size_t, std::uint8_t)> publish_selection;
    std::function<void()> render_selection_text;
    std::function<void()> render_rows;
    std::function<void()> update_hud_mode_transition;
    std::function<void(std::uint8_t, std::uint8_t)> initialize_world_effect;
};

MatrixProgramActionEvent service_matrix_program_action(
    MatrixProgramActionState& state,
    const MatrixProgramActionCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
