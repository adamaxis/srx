#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x41B6: fold the high nibble into the source's eight-value field band.
std::uint16_t fold_field_coordinate(std::uint16_t coordinate);

// ROM 0x4216: subtract eight from the high-nibble coordinate.
std::int16_t offset_field_coordinate(std::uint16_t coordinate);

// ROM 0x423C: quantize a wrapped 16-bit coordinate delta in 128-unit steps.
std::int16_t quantize_field_delta(std::int16_t target,
                                  std::uint16_t origin);

struct FieldTileSourceLayout {
    // 0x3FD8 D3: byte offset into the active row-offset table.
    std::uint16_t row_table_byte_offset = 0;
    // 0x3FD8 D1: byte offset for the tile inside its four-by-four block.
    std::uint16_t tile_word_byte_offset = 0;
};

// ROM 0x3FD8: split a field coordinate into its coarse row-table location
// and the two-dimensional tile offset within the selected map block.
FieldTileSourceLayout calculate_field_tile_source_layout(
    std::uint16_t x,
    std::uint16_t y,
    std::uint16_t row_stride_bytes);

// ROM 0x4000: compose the even display destination used by the field tile
// edge/row writers from the signed row and the source column value.
std::int32_t compose_field_tile_destination_offset(std::int16_t row,
                                                   std::uint16_t column);

struct FieldVerticalOrigin {
    std::int16_t row = 0;
    std::uint16_t camera_y = 0;
};

// ROM 0x41F0: consume the camera Y and field Y globals in three eight-unit
// stages, returning the row origin and the adjusted camera coordinate.
FieldVerticalOrigin prepare_field_vertical_origin(std::uint16_t camera_y,
                                                  std::uint16_t field_y);

} // namespace shadowrun::ghidra_conversion
