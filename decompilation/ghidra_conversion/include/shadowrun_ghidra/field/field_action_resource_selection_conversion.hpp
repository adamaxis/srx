#pragma once

#include "shadowrun_ghidra/cyberspace/matrix_storage_conversion.hpp"
#include "shadowrun_ghidra/field/field_mode_selection_presentation_conversion.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The source table at 0x1AB276 contains two 0x30-byte menu-resource tables.
// The native conversion keeps the extracted bytes as data and lets the
// selector return a typed record/word location; no ROM pointer is retained.
struct FieldActionMenuResourceTables {
    std::array<std::uint8_t, 0x30> general_records{};
    std::array<std::uint8_t, 0x30> menu_mode_six_records{};
};

// The value read through A0 at 0x57614 is the byte used as D7 by
// matrix_program_storage_weight and matrix_program_storage_usage.  The
// quantity is FFF0D6, which is loaded into D1 before the weight helper.
struct FieldActionSelectionContext {
    MenuResourceEntry entry{};
    std::uint8_t resource_record_value = 0;
    std::uint8_t selected_action_quantity = 0;
    std::uint32_t selected_storage_weight = 0;
};

struct FieldActionResourceSelectionResult {
    MatrixStorageCapacity capacity{};
    FieldActionSelectionContext selection{};
    std::uint32_t selected_program_storage_usage = 0;
    std::uint16_t adjusted_available_memory = 0;
};

// Read the byte that the source obtains from the selected menu-resource word.
// An absent value means that the host has not supplied a complete extracted
// table for the selected record; callers must not invent a fallback.
std::optional<std::uint8_t> read_menu_resource_value(
    const FieldActionMenuResourceTables& tables,
    const MenuResourceEntry& entry);

// ROM 0x57610. Select the menu-resource word, read its record byte, and pass
// that byte plus FFF0D6 through 0x19538.
std::optional<FieldActionSelectionContext>
prepare_field_action_selection_context(
    const FieldActionMenuResourceTables& tables,
    std::uint8_t menu_mode,
    std::int8_t saved_selection,
    std::int8_t primary_cursor,
    std::uint8_t selected_action_quantity);

// ROM 0x575F8. Calculate available Matrix storage, then replace the selected
// program's current storage weight with its newly selected weight:
// D6 = capacity_available + matrix_program_storage_usage(D7)
//      - matrix_program_storage_weight(record_byte, FFF0D6).
// The result preserves the source's word arithmetic at the final adjustment.
std::optional<FieldActionResourceSelectionResult>
prepare_field_action_resource_selection(
    const MatrixStorageUsage& storage,
    const FieldActionMenuResourceTables& tables,
    std::uint8_t menu_mode,
    std::int8_t saved_selection,
    std::int8_t primary_cursor,
    std::uint8_t selected_action_quantity);

} // namespace shadowrun::ghidra_conversion
