#include "shadowrun_ghidra/field/field_action_resource_selection_conversion.hpp"

#include <array>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldActionMenuResourceTables tables{};
    tables.menu_mode_six_records[0x10U + 0x06U] = 2U;
    tables.general_records[0x10U + 0x04U] = 3U;

    const auto context = prepare_field_action_selection_context(
        tables, 6U, 1, 3, 4U);
    if (!expect(context &&
                context->entry.resource.family ==
                    MenuResourceFamily::menu_mode_six_records &&
                context->entry.resource.record_offset == 0x10U &&
                context->entry.word_offset == 0x06U &&
                context->resource_record_value == 2U &&
                context->selected_action_quantity == 4U &&
                context->selected_storage_weight == 64U)) {
        return 1;
    }

    const auto general = prepare_field_action_selection_context(
        tables, 4U, 1, 2, 5U);
    if (!expect(general &&
                general->entry.resource.family ==
                    MenuResourceFamily::general_menu_records &&
                general->entry.resource.record_offset == 0x10U &&
                general->entry.word_offset == 0x04U &&
                general->resource_record_value == 3U &&
                general->selected_storage_weight == 75U)) {
        return 2;
    }

    if (!expect(!prepare_field_action_selection_context(
                     tables, 6U, -1, 0, 1U))) {
        return 3;
    }

    MatrixStorageUsage storage{};
    storage.total_memory = 100U;
    storage.loaded_slot_usage = {10U, 0U, 0U, 0U, 0U};
    storage.availability_flags[2U] = 1U;
    const auto result = prepare_field_action_resource_selection(
        storage, tables, 6U, 1, 3, 4U);
    if (!expect(result && result->capacity.available_memory == 86U &&
                result->selected_program_storage_usage == 4U &&
                result->adjusted_available_memory == 26U)) {
        return 4;
    }

    // 0x575F8 uses ADD.W and SUB.W on D6.  A replacement whose old weight is
    // larger than the available word must remain a wrapped source word.
    storage.total_memory = 1U;
    storage.loaded_slot_usage = {};
    storage.availability_flags.fill(0U);
    const auto wrapped = prepare_field_action_resource_selection(
        storage, tables, 6U, 1, 3, 4U);
    if (!expect(wrapped && wrapped->adjusted_available_memory == 0xFFC1U)) {
        return 5;
    }

    tables.menu_mode_six_records[0x10U + 0x06U] = 12U;
    if (!expect(!prepare_field_action_selection_context(
                     tables, 6U, 1, 3, 4U))) {
        return 6;
    }

    return 0;
}
