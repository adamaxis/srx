#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"

#include <cstdint>

namespace {

bool expect(bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(fold_field_coordinate(0x0070U) == 7U &&
                fold_field_coordinate(0x0080U) == 0U &&
                fold_field_coordinate(0x00F0U) == 7U)) {
        return 1;
    }
    if (!expect(offset_field_coordinate(0x0070U) == -1 &&
                offset_field_coordinate(0x0080U) == 0 &&
                offset_field_coordinate(0x00F0U) == 7)) {
        return 1;
    }
    if (!expect(quantize_field_delta(0x1000, 0U) == 64 &&
                quantize_field_delta(0, 0x1000U) == -64 &&
                quantize_field_delta(0x007FU, 0U) == 0)) {
        return 1;
    }

    const auto layout = calculate_field_tile_source_layout(
        0x0128U, 0x00A8U, 0x0040U);
    if (!expect(layout.row_table_byte_offset == 0x0152U &&
                layout.tile_word_byte_offset == 0x000AU)) {
        return 2;
    }

    if (!expect(compose_field_tile_destination_offset(0, 0x00F8U) ==
                    0x0F80 &&
                compose_field_tile_destination_offset(8, 0x00F8U) ==
                    0x0FFEU &&
                compose_field_tile_destination_offset(-8, 0x0000U) ==
                    0x0002)) {
        return 3;
    }

    const auto first_vertical_origin =
        prepare_field_vertical_origin(0x0100U, 0x0070U);
    if (!expect(first_vertical_origin.row == 7 &&
                first_vertical_origin.camera_y == 0x0100U)) {
        return 4;
    }
    const auto second_vertical_origin =
        prepare_field_vertical_origin(0x0100U, 0x0080U);
    if (!expect(second_vertical_origin.row == 0 &&
                second_vertical_origin.camera_y == 0x00F8U)) {
        return 5;
    }
    const auto third_vertical_origin =
        prepare_field_vertical_origin(0x0100U, 0x0100U);
    if (!expect(third_vertical_origin.row == 0 &&
                third_vertical_origin.camera_y == 0x00F0U)) {
        return 6;
    }

    const auto first_horizontal_origin =
        prepare_field_horizontal_origin(0x0070U, 0x0200U);
    if (!expect(first_horizontal_origin.column == 7 &&
                first_horizontal_origin.vdp_data == 0x0200U)) {
        return 7;
    }
    const auto second_horizontal_origin =
        prepare_field_horizontal_origin(0x0080U, 0x0200U);
    if (!expect(second_horizontal_origin.column == 0 &&
                second_horizontal_origin.vdp_data == 0x0208U)) {
        return 8;
    }
    const auto third_horizontal_origin =
        prepare_field_horizontal_origin(0x0100U, 0x0200U);
    if (!expect(third_horizontal_origin.column == 0 &&
                third_horizontal_origin.vdp_data == 0x0210U)) {
        return 9;
    }
    return 0;
}
