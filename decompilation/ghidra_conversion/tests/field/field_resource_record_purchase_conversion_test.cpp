#include "shadowrun_ghidra/field/field_resource_record_purchase_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldResourceRecordTables tables{};
    tables.general_records[2U].availability_requirement = 4U;
    tables.general_records[2U].installed_item_code = 0x21U;
    tables.general_records[2U].installed_item_flags = 0x80U;
    tables.general_records[2U].primary_capacity = 40U;
    tables.general_records[2U].secondary_capacity = 60U;
    tables.general_records[2U].resource_limit = 9U;
    tables.general_records[2U].resource_progress = 3U;
    tables.menu_mode_six_records[2U].availability_requirement = 5U;
    tables.menu_mode_six_records[2U].installed_item_code = 0x31U;
    tables.menu_mode_six_records[2U].installed_item_flags = 0x40U;
    tables.menu_mode_six_records[2U].primary_capacity = 55U;
    tables.menu_mode_six_records[2U].secondary_capacity = 70U;
    tables.menu_mode_six_records[2U].resource_limit = 11U;
    tables.menu_mode_six_records[2U].resource_progress = 6U;

    FieldResourceRecordPurchaseState state{};
    state.menu_mode = 6U;
    state.category = 2U;
    state.cost = 125U;
    state.funds = 500U;
    state.inventory.availability_marker = 5U;
    state.inventory.primary_capacity = 40U;
    state.inventory.secondary_capacity = 80U;
    int grids = 0;
    int requirement_failures = 0;
    int fund_failures = 0;
    int confirmations = 0;
    int waits = 0;
    const FieldResourceRecordPurchaseCallbacks callbacks{
        [&] { ++grids; },
        [&] { ++requirement_failures; },
        [&] { ++fund_failures; },
        [&] {
            ++confirmations;
            return true;
        },
        [&] { ++waits; },
    };

    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::applied &&
                grids == 1 && confirmations == 1 && state.funds == 375U &&
                state.inventory.installed_item_code == 0x31U &&
                state.inventory.installed_item_flags == 0x40U &&
                state.inventory.primary_capacity == 55U &&
                state.inventory.secondary_capacity == 80U &&
                state.inventory.resource_limit == 11U &&
                state.inventory.resource_progress == 6U &&
                state.inventory.availability_marker == 5U)) {
        return 1;
    }

    state.inventory.availability_marker = 4U;
    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::insufficient_requirement &&
                requirement_failures == 1 && waits == 1 &&
                state.funds == 375U)) {
        return 2;
    }

    state.inventory.availability_marker = 5U;
    state.funds = 10U;
    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::insufficient_funds &&
                fund_failures == 1 && waits == 2 && state.funds == 10U)) {
        return 3;
    }

    state.funds = 500U;
    const FieldResourceRecordPurchaseCallbacks cancel_callbacks{
        {}, {}, {}, [] { return false; }};
    if (!expect(apply_field_resource_record_purchase(
                    tables, state, cancel_callbacks) ==
                    FieldResourceRecordPurchaseResult::cancelled &&
                state.funds == 500U)) {
        return 4;
    }

    tables.menu_mode_six_records[2U].availability_requirement = 0xFFU;
    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::invalid_record)) {
        return 5;
    }

    // The ROM's CMP.B/BMI gate is a signed wrapped-byte predicate.  A current
    // byte of 200 and requirement 127 produces a positive wrapped difference
    // and is accepted even though an unsigned comparison would reject it.
    tables.menu_mode_six_records[2U].availability_requirement = 127U;
    state.inventory.availability_marker = 200U;
    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::applied &&
                state.inventory.installed_item_code == 0x31U)) {
        return 6;
    }

    state.menu_mode = 4U;
    state.category = 2U;
    state.inventory.availability_marker = 4U;
    state.funds = 500U;
    if (!expect(apply_field_resource_record_purchase(tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::applied &&
                state.inventory.installed_item_code == 0x21U &&
                state.inventory.resource_limit == 9U)) {
        return 7;
    }

    FieldResourceRecordTables word_flag_tables = tables;
    word_flag_tables.general_records[2U].primary_capacity = 0xFFFFU;
    state.menu_mode = 4U;
    state.category = 2U;
    state.inventory.availability_marker = 4U;
    state.inventory.primary_capacity = 40U;
    state.inventory.secondary_capacity = 80U;
    state.funds = 500U;
    if (!expect(apply_field_resource_record_purchase(
                    word_flag_tables, state, callbacks) ==
                    FieldResourceRecordPurchaseResult::applied &&
                // 0xFFFF - 40 has bit 15 set, so the ROM's BMI skips the
                // primary-capacity replacement despite the unsigned value.
                state.inventory.primary_capacity == 40U)) {
        return 8;
    }

    return 0;
}
