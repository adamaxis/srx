#include "shadowrun_ghidra/field/field_coordinate_conversion.hpp"

namespace shadowrun::ghidra_conversion {

std::uint16_t fold_field_coordinate(std::uint16_t coordinate) {
    auto folded = static_cast<std::uint16_t>(coordinate >> 4U);
    if (folded >= 8U) {
        folded = static_cast<std::uint16_t>(folded - 8U);
    }
    return folded;
}

FieldPlaneFillOrigin prepare_field_plane_fill_origin(
    const std::uint16_t camera_x,
    const std::uint16_t field_y,
    const std::uint16_t destination_index,
    const std::uint16_t destination_base) {
    FieldPlaneFillOrigin origin{
        static_cast<std::uint16_t>((camera_x >> 4U) - 8U),
        static_cast<std::uint16_t>((field_y >> 4U) - 8U),
        destination_index,
        destination_base};
    if (static_cast<std::int16_t>(origin.x) < 0) {
        origin.x = static_cast<std::uint16_t>(origin.x + 8U);
    } else {
        origin.destination_index =
            static_cast<std::uint16_t>(origin.destination_index + 8U);
    }
    if (static_cast<std::int16_t>(origin.y) < 0) {
        origin.y = static_cast<std::uint16_t>(origin.y + 8U);
    } else {
        origin.destination_base =
            static_cast<std::uint16_t>(origin.destination_base - 8U);
    }
    return origin;
}

std::int16_t offset_field_coordinate(std::uint16_t coordinate) {
    const auto folded = static_cast<std::uint16_t>(coordinate >> 4U);
    return static_cast<std::int16_t>(folded - 8U);
}

std::int16_t quantize_field_delta(std::int16_t target,
                                  std::uint16_t origin) {
    const auto delta = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(target) - origin);
    const auto quantized = static_cast<std::int32_t>(delta) >> 7;
    return static_cast<std::int16_t>(quantized << 1);
}

FieldTileSourceLayout calculate_field_tile_source_layout(
    const std::uint16_t x,
    const std::uint16_t y,
    const std::uint16_t row_stride_bytes) {
    const auto x_block = static_cast<std::uint16_t>(x >> 5U);
    const auto y_block = static_cast<std::uint16_t>(y >> 5U);
    const auto x_in_block = static_cast<std::uint16_t>((x >> 3U) & 3U);
    const auto y_in_block = static_cast<std::uint16_t>((y >> 3U) & 3U);

    const auto row_table_offset = static_cast<std::uint16_t>(
        static_cast<std::uint32_t>(y_block) * row_stride_bytes +
        static_cast<std::uint32_t>(x_block) * 2U);
    const auto tile_offset = static_cast<std::uint16_t>(
        ((static_cast<std::uint16_t>(y_in_block << 2U) + x_in_block)
         << 1U));
    return {row_table_offset, tile_offset};
}

std::int32_t compose_field_tile_destination_offset(
    const std::int16_t row,
    const std::uint16_t column) {
    const auto normalized_row = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(-row) & 0x01FFU);
    const auto masked_column = static_cast<std::uint16_t>(column & 0x00F8U);
    const auto word = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(masked_column) << 4U) +
        (normalized_row >> 2U));
    return static_cast<std::int32_t>(static_cast<std::int16_t>(
        static_cast<std::uint16_t>(word & 0xFFFEU)));
}

FieldVerticalOrigin prepare_field_vertical_origin(
    const std::uint16_t camera_y,
    const std::uint16_t field_y) {
    auto row = static_cast<std::int16_t>((field_y >> 4U) - 8);
    auto adjusted_camera_y = camera_y;
    if (row < 0) {
        row = static_cast<std::int16_t>(row + 8);
        return {row, adjusted_camera_y};
    }

    adjusted_camera_y = static_cast<std::uint16_t>(adjusted_camera_y - 8U);
    row = static_cast<std::int16_t>(row - 8);
    if (row < 0) {
        row = static_cast<std::int16_t>(row + 8);
        return {row, adjusted_camera_y};
    }

    adjusted_camera_y = static_cast<std::uint16_t>(adjusted_camera_y - 8U);
    return {row, adjusted_camera_y};
}

FieldHorizontalOrigin prepare_field_horizontal_origin(
    const std::uint16_t camera_x, const std::uint16_t vdp_data_word) {
    auto column = static_cast<std::int16_t>((camera_x >> 4U) - 8);
    auto vdp_data = vdp_data_word;
    if (column < 0) {
        column = static_cast<std::int16_t>(column + 8);
        return {column, vdp_data};
    }

    vdp_data = static_cast<std::uint16_t>(vdp_data + 8U);
    column = static_cast<std::int16_t>(column - 8);
    if (column < 0) {
        column = static_cast<std::int16_t>(column + 8);
        return {column, vdp_data};
    }

    vdp_data = static_cast<std::uint16_t>(vdp_data + 8U);
    return {column, vdp_data};
}

} // namespace shadowrun::ghidra_conversion
