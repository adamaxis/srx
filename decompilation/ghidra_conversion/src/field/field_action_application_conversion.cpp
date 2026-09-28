#include "shadowrun_ghidra/field/field_action_application_conversion.hpp"

#include <optional>

namespace shadowrun::ghidra_conversion {

FieldActionApplicationResult apply_selected_field_action(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& state,
    const FieldActionBaseCostLookup& base_cost_lookup,
    const FieldActionApplicationCallbacks& callbacks) {
    // 0x57962 uses a signed-byte test before any panel or cost work.  A high
    // bit in the record marker is the source's disabled/invalid sentinel.
    if (record.type >= 0x80U) {
        state.service_result = -1;
        return FieldActionApplicationResult::invalid_record;
    }

    if (callbacks.refresh_action_detail_panel) {
        callbacks.refresh_action_detail_panel();
    }

    const bool is_special_action = record.type == static_cast<std::uint8_t>('E');
    std::size_t inventory_slot = 0;
    if (!is_special_action) {
        while (inventory_slot < state.runner.action_codes.size() &&
               state.runner.action_codes[inventory_slot] != 0U) {
            ++inventory_slot;
        }
        if (inventory_slot == state.runner.action_codes.size()) {
            if (callbacks.show_no_inventory_slot) {
                callbacks.show_no_inventory_slot();
            }
            if (callbacks.wait_for_controller_event) {
                callbacks.wait_for_controller_event();
            }
            return FieldActionApplicationResult::no_inventory_slot;
        }
    }

    const auto preview_cost = is_special_action
                                  ? apply_field_action_cost_discounts(
                                        record.quantity, cost_context)
                                  : calculate_field_action_record_cost(
                                        record, cost_context, base_cost_lookup);
    if (state.funds < preview_cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        if (callbacks.wait_for_controller_event) {
            callbacks.wait_for_controller_event();
        }
        return FieldActionApplicationResult::insufficient_funds;
    }

    const bool confirmed = callbacks.confirm_purchase
                               ? callbacks.confirm_purchase()
                               : true;
    if (!confirmed) {
        return FieldActionApplicationResult::cancelled;
    }

    if (is_special_action) {
        // 0x57A08 increments first, then rejects 0x15 and restores the
        // visible count to 0x14. Preserve that source mutation order before
        // exposing the capacity-limit presentation boundary.
        ++state.runner.special_action_count;
        if (state.runner.special_action_count >= 0x15U) {
            state.runner.special_action_count = 0x14U;
            if (callbacks.refresh_action_detail_panel) {
                callbacks.refresh_action_detail_panel();
            }
            if (callbacks.show_special_action_limit) {
                callbacks.show_special_action_limit();
            }
            if (callbacks.wait_for_controller_event) {
                callbacks.wait_for_controller_event();
            }
            return FieldActionApplicationResult::special_action_limit;
        }
    }
    else {
        // The source stores the record marker and immediately increments the
        // stored byte.  Preserve that observable result as marker + 1, while
        // the companion byte is copied unchanged from record byte 1.
        state.runner.action_codes[inventory_slot] =
            static_cast<std::uint8_t>(record.type + 1U);
        state.runner.companion_values[inventory_slot] = record.value;
    }

    state.funds -= preview_cost;
    return FieldActionApplicationResult::applied;
}

FieldActionApplicationResult apply_selected_field_action(
    const FieldActionCostRecord& record,
    const FieldActionCostContext& cost_context,
    FieldActionPurchaseState& state,
    const UsItemCatalog& item_catalog,
    const FieldActionApplicationCallbacks& callbacks) {
    // Preserve the source high-bit disabled marker before applying the
    // zero-based D4540 identity check. This keeps the typed path's result
    // distinct from a valid-looking record with missing source data.
    if (record.type >= 0x80U) {
        return apply_selected_field_action(record, cost_context, state,
                                           FieldActionBaseCostLookup{},
                                           callbacks);
    }

    const auto special_action = record.type == static_cast<std::uint8_t>('E');
    if (!special_action) {
        if (record.type >= item_catalog.size()) {
            return FieldActionApplicationResult::missing_source_data;
        }
        const auto& item = item_catalog[record.type];
        if (item.item_code != static_cast<std::uint8_t>(record.type + 1U)) {
            return FieldActionApplicationResult::missing_source_data;
        }
    }

    const FieldActionBaseCostLookup lookup =
        [&item_catalog](const std::uint8_t record_type) -> std::uint32_t {
        return item_catalog[record_type].base_cost;
    };
    return apply_selected_field_action(record, cost_context, state, lookup,
                                       callbacks);
}

} // namespace shadowrun::ghidra_conversion
