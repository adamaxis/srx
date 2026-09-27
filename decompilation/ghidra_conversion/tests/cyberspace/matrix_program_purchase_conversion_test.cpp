#include "shadowrun_ghidra/cyberspace/matrix_program_purchase_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionMenuResourceTables tables{};
    tables.menu_mode_six_records[0x10U + 0x06U] = 2U;

    MatrixProgramPurchaseState state{};
    state.storage.total_memory = 100U;
    state.storage.loaded_slot_usage = {10U, 0U, 0U, 0U, 0U};
    state.storage.availability_flags[2U] = 1U;
    state.loaded_program_slots = {0, 1, 2, 3, 4};
    state.menu_mode = 6U;
    state.saved_selection = 1;
    state.primary_cursor = 3;
    state.selected_quantity = 4U;
    state.cost = 100U;
    state.funds = 500U;
    int grids = 0;
    int poor = 0;
    int no_storage = 0;
    int confirmations = 0;
    const MatrixProgramPurchaseCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++poor; },
        [&] { ++no_storage; },
        [&] {
            ++confirmations;
            return true;
        },
    };

    if (!expect(install_selected_matrix_program_resource(tables, state,
                                                         callbacks) ==
                    MatrixProgramPurchaseResult::applied &&
                grids == 1 && confirmations == 1 && state.funds == 400U &&
                state.storage.availability_flags[2U] == 4U &&
                state.loaded_program_slots ==
                    std::array<std::int8_t, 5>{-1, -1, -1, -1, -1})) {
        return 1;
    }

    state.selected_quantity = 9U;
    if (!expect(install_selected_matrix_program_resource(tables, state,
                                                         callbacks) ==
                    MatrixProgramPurchaseResult::invalid_quantity &&
                grids == 1)) {
        return 2;
    }

    state.selected_quantity = 4U;
    state.storage.total_memory = 1U;
    state.storage.availability_flags.fill(0U);
    state.funds = 400U;
    if (!expect(install_selected_matrix_program_resource(tables, state,
                                                         callbacks) ==
                    MatrixProgramPurchaseResult::insufficient_storage &&
                no_storage == 1 && grids == 2 && state.funds == 400U)) {
        return 3;
    }

    state.storage.total_memory = 100U;
    state.funds = 50U;
    if (!expect(install_selected_matrix_program_resource(tables, state,
                                                         callbacks) ==
                    MatrixProgramPurchaseResult::insufficient_funds &&
                poor == 1 && grids == 3 && state.funds == 50U)) {
        return 4;
    }

    state.funds = 400U;
    const MatrixProgramPurchaseCallbacks cancel_callbacks{
        {}, {}, {}, [] { return false; }};
    if (!expect(install_selected_matrix_program_resource(
                    tables, state, cancel_callbacks) ==
                    MatrixProgramPurchaseResult::cancelled &&
                state.funds == 400U && state.storage.availability_flags[2U] ==
                                         0U)) {
        return 5;
    }

    tables.menu_mode_six_records[0x10U + 0x06U] = 0x80U;
    if (!expect(install_selected_matrix_program_resource(tables, state,
                                                         callbacks) ==
                    MatrixProgramPurchaseResult::invalid_resource_record &&
                grids == 3)) {
        return 6;
    }

    std::array<std::int8_t, 5> slots{0, 1, 2, 3, 4};
    reset_field_action_cost_state(slots);
    if (!expect(slots == std::array<std::int8_t, 5>{-1, -1, -1, -1, -1})) {
        return 7;
    }

    return 0;
}
