#include "shadowrun_ghidra/cyberspace/matrix_program_purchase_conversion.hpp"

#include <cstdint>

namespace shadowrun::ghidra_conversion {

void reset_field_action_cost_state(
    std::array<std::int8_t, 5>& loaded_program_slots) {
    loaded_program_slots.fill(static_cast<std::int8_t>(-1));
}

MatrixProgramPurchaseResult install_selected_matrix_program_resource(
    const FieldActionMenuResourceTables& tables,
    MatrixProgramPurchaseState& state,
    const MatrixProgramPurchaseCallbacks& callbacks) {
    if (state.selected_quantity >= 9U) {
        return MatrixProgramPurchaseResult::invalid_quantity;
    }

    const auto context = prepare_field_action_selection_context(
        tables, state.menu_mode, state.saved_selection, state.primary_cursor,
        state.selected_quantity);
    if (!context || context->resource_record_value >=
                        state.storage.availability_flags.size()) {
        return MatrixProgramPurchaseResult::invalid_resource_record;
    }

    if (callbacks.render_action_grid) {
        callbacks.render_action_grid();
    }
    if (state.funds < state.cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return MatrixProgramPurchaseResult::insufficient_funds;
    }

    const auto capacity = prepare_field_action_resource_selection(
        state.storage, tables, state.menu_mode, state.saved_selection,
        state.primary_cursor, state.selected_quantity);
    if (!capacity ||
        static_cast<std::int16_t>(capacity->adjusted_available_memory) < 0) {
        if (callbacks.show_insufficient_storage) {
            callbacks.show_insufficient_storage();
        }
        return MatrixProgramPurchaseResult::insufficient_storage;
    }

    if (callbacks.confirm_purchase && !callbacks.confirm_purchase()) {
        return MatrixProgramPurchaseResult::cancelled;
    }

    state.funds -= state.cost;
    reset_field_action_cost_state(state.loaded_program_slots);
    state.storage.availability_flags[context->resource_record_value] =
        state.selected_quantity;
    return MatrixProgramPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion
