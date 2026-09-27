#include "shadowrun_ghidra/field/field_resource_record_purchase_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

const std::array<FieldResourceRecord, 8>& records_for_mode(
    const FieldResourceRecordTables& tables,
    const std::uint8_t menu_mode) {
    return menu_mode == 6U ? tables.menu_mode_six_records
                           : tables.general_records;
}

// 0x573FC uses CMP.B followed by BMI. Preserve the signed bit of the wrapped
// byte subtraction instead of silently replacing it with an unsigned < test.
bool source_byte_difference_is_negative(const std::uint8_t left,
                                        const std::uint8_t right) {
    return static_cast<std::uint8_t>(left - right) & 0x80U;
}

} // namespace

FieldResourceRecordPurchaseResult apply_field_resource_record_purchase(
    const FieldResourceRecordTables& tables,
    FieldResourceRecordPurchaseState& state,
    const FieldResourceRecordPurchaseCallbacks& callbacks) {
    if (callbacks.render_detail_grid) {
        callbacks.render_detail_grid();
    }

    const auto& records = records_for_mode(tables, state.menu_mode);
    if (state.category >= records.size()) {
        return FieldResourceRecordPurchaseResult::invalid_record;
    }
    const auto& record = records[state.category];
    if (record.availability_requirement >= 0x80U) {
        return FieldResourceRecordPurchaseResult::invalid_record;
    }

    if (source_byte_difference_is_negative(
            state.inventory.availability_marker,
            record.availability_requirement)) {
        if (callbacks.show_insufficient_requirement) {
            callbacks.show_insufficient_requirement();
        }
        return FieldResourceRecordPurchaseResult::insufficient_requirement;
    }

    if (state.funds < state.cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return FieldResourceRecordPurchaseResult::insufficient_funds;
    }

    if (callbacks.confirm_purchase && !callbacks.confirm_purchase()) {
        return FieldResourceRecordPurchaseResult::cancelled;
    }

    state.funds -= state.cost;
    state.inventory.installed_item_code = record.installed_item_code;
    state.inventory.installed_item_flags = record.installed_item_flags;
    if (record.primary_capacity >= state.inventory.primary_capacity) {
        state.inventory.primary_capacity = record.primary_capacity;
    }
    if (record.secondary_capacity >= state.inventory.secondary_capacity) {
        state.inventory.secondary_capacity = record.secondary_capacity;
    }
    state.inventory.resource_limit = record.resource_limit;
    state.inventory.resource_progress = record.resource_progress;
    state.inventory.availability_marker = record.availability_requirement;
    return FieldResourceRecordPurchaseResult::applied;
}

} // namespace shadowrun::ghidra_conversion
