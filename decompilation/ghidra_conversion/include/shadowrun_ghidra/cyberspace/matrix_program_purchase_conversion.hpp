#pragma once

#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"
#include "shadowrun_ghidra/field/field_action_resource_selection_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct MatrixProgramPurchaseState {
    MatrixStorageUsage storage{};
    std::array<std::int8_t, 5> loaded_program_slots{}; // FFFBD8..FFFBDC
    std::uint8_t menu_mode = 0; // FFF0B6
    std::int8_t saved_selection = 0; // FFF0B8
    std::int8_t primary_cursor = 0; // FFF0D0
    std::uint8_t selected_quantity = 0; // FFF0D6
    std::uint32_t cost = 0; // FFF0BE
    std::uint32_t funds = 0; // FFFBFE
    std::int8_t service_result = 0; // FFF112
};

enum class MatrixProgramPurchaseResult {
    applied,
    invalid_quantity,
    invalid_resource_record,
    insufficient_funds,
    insufficient_storage,
    cancelled,
};

struct MatrixProgramPurchaseCallbacks {
    std::function<void()> render_action_grid;
    std::function<void()> show_insufficient_funds;
    std::function<void()> show_insufficient_storage;
    std::function<bool()> confirm_purchase;
    std::function<void()> wait_for_controller_event;
};

// ROM 0x5751A. Install the selected Matrix program resource from the field
// action menu. The quantity is limited to eight, the selected resource byte
// identifies the program, and the source's signed D6 capacity gate is kept.
MatrixProgramPurchaseResult install_selected_matrix_program_resource(
    const FieldActionMenuResourceTables& tables,
    MatrixProgramPurchaseState& state,
    const MatrixProgramPurchaseCallbacks& callbacks = {});

// ROM 0x58D7E. Clear all five loaded-program selectors before publishing a
// newly installed program. The source writes FFFBD8..FFFBDC to 0xFF.
void reset_field_action_cost_state(
    std::array<std::int8_t, 5>& loaded_program_slots);

} // namespace shadowrun::ghidra_conversion
