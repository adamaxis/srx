#include "shadowrun_ghidra/cyberspace/matrix_program_purchase_conversion.hpp"

#include <cstdint>

namespace shadowrun::ghidra_conversion {

namespace {

bool source_quantity_is_rejected(const std::uint8_t quantity) {
    // 0x5751A uses CMPI.B #9 followed by BPL. The source branches for a
    // nonnegative wrapped result; it is not an unsigned quantity >= 9 test.
    return (static_cast<std::uint8_t>(quantity - 9U) & 0x80U) == 0U;
}

} // namespace

void reset_field_action_cost_state(
    std::array<std::int8_t, 5>& loaded_program_slots) {
    loaded_program_slots.fill(static_cast<std::int8_t>(-1));
}

MatrixProgramPurchaseResult install_selected_matrix_program_resource(
    const FieldActionMenuResourceTables& tables,
    MatrixProgramPurchaseState& state,
    const MatrixProgramPurchaseCallbacks& callbacks) {
    if (source_quantity_is_rejected(state.selected_quantity)) {
        state.service_result = -1;
        return MatrixProgramPurchaseResult::invalid_quantity;
    }

    const auto context = prepare_field_action_selection_context(
        tables, state.menu_mode, state.saved_selection, state.primary_cursor,
        state.selected_quantity);
    if (!context || context->resource_record_value >=
                        state.storage.availability_flags.size()) {
        state.service_result = -1;
        return MatrixProgramPurchaseResult::invalid_resource_record;
    }

    if (callbacks.render_action_grid) {
        callbacks.render_action_grid();
    }
    if (state.funds < state.cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
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
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
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
