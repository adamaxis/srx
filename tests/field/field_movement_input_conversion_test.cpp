#include "shadowrun_ghidra/field/field_movement_input_conversion.hpp"

#include <array>
#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::uint16_t, 16> source_table{{
        0xFFFFU, 0x0000U, 0x8000U, 0xFFFFU,
        0xC000U, 0xE000U, 0xA000U, 0xFFFFU,
        0x4000U, 0x2000U, 0x6000U, 0xFFFFU,
        0xFFFFU, 0xFFFFU, 0xFFFFU, 0xFFFFU}};

    const auto up = decode_field_movement_input(0x01U, source_table);
    if (!up.valid || up.target_heading != 0x0000U ||
        up.direction_ordinal != 0U || up.direction_slot != 0U) {
        return 1;
    }

    const auto right = decode_field_movement_input(0x08U, source_table);
    if (!right.valid || right.target_heading != 0x4000U ||
        right.direction_ordinal != 2U || right.direction_slot != 4U) {
        return 2;
    }

    const auto diagonal = decode_field_movement_input(0x05U, source_table);
    if (!diagonal.valid || diagonal.target_heading != 0xE000U ||
        diagonal.direction_ordinal != 7U || diagonal.direction_slot != 14U) {
        return 3;
    }

    if (decode_field_movement_input(0x00U, source_table).valid ||
        field_movement_direction_from_heading(0x8000U) != 4U ||
        field_movement_direction_from_heading(0xC000U) != 6U) {
        return 4;
    }
    return 0;
}
