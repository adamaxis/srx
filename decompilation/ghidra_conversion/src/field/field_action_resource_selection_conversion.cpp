#include "shadowrun_ghidra/field/field_action_resource_selection_conversion.hpp"

#include <cstddef>

namespace shadowrun::ghidra_conversion {
namespace {

const std::array<std::uint8_t, 0x30>& table_for_family(
    const FieldActionMenuResourceTables& tables,
    const MenuResourceFamily family) {
    return family == MenuResourceFamily::menu_mode_six_records
               ? tables.menu_mode_six_records
               : tables.general_records;
}

} // namespace

std::optional<std::uint8_t> read_menu_resource_value(
    const FieldActionMenuResourceTables& tables,
    const MenuResourceEntry& entry) {
    const auto& table = table_for_family(tables, entry.resource.family);
    const auto byte_offset = static_cast<std::size_t>(entry.resource.record_offset) +
                             static_cast<std::size_t>(entry.word_offset);
    if (byte_offset >= table.size()) {
        return std::nullopt;
    }
    return table[byte_offset];
}

std::optional<FieldActionSelectionContext>
prepare_field_action_selection_context(
    const FieldActionMenuResourceTables& tables,
    const std::uint8_t menu_mode,
    const std::int8_t saved_selection,
    const std::int8_t primary_cursor,
    const std::uint8_t selected_action_quantity) {
    const auto entry = select_menu_resource_entry(
        menu_mode, saved_selection, primary_cursor);
    const auto record_value = read_menu_resource_value(tables, entry);
    if (!record_value) {
        return std::nullopt;
    }

    return FieldActionSelectionContext{
        entry,
        *record_value,
        selected_action_quantity,
        matrix_program_storage_weight(*record_value,
                                      selected_action_quantity),
    };
}

std::optional<FieldActionResourceSelectionResult>
prepare_field_action_resource_selection(
    const MatrixStorageUsage& storage,
    const FieldActionMenuResourceTables& tables,
    const std::uint8_t menu_mode,
    const std::int8_t saved_selection,
    const std::int8_t primary_cursor,
    const std::uint8_t selected_action_quantity) {
    const auto selection = prepare_field_action_selection_context(
        tables, menu_mode, saved_selection, primary_cursor,
        selected_action_quantity);
    if (!selection ||
        selection->resource_record_value >= storage.availability_flags.size() ||
        selection->resource_record_value >= kMatrixProgramFootprints.size()) {
        return std::nullopt;
    }

    const auto capacity = calculate_matrix_storage_capacity(storage);
    const auto selected_program_usage = matrix_program_storage_usage(
        storage, selection->resource_record_value);
    const auto adjusted_available_memory = static_cast<std::uint16_t>(
        capacity.available_memory +
        static_cast<std::uint16_t>(selected_program_usage) -
        static_cast<std::uint16_t>(selection->selected_storage_weight));

    return FieldActionResourceSelectionResult{
        capacity,
        *selection,
        selected_program_usage,
        adjusted_available_memory,
    };
}

} // namespace shadowrun::ghidra_conversion
