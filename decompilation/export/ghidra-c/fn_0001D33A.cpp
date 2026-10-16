// Ghidra headless export
// Entry: 0x0001D33A
// Name: render_world_sequence_tile_rows
// Decompile status: source-shaped annotation
//
// The native conversion emits host tile writes through callbacks. It keeps
// the source row window, record visibility test, logical source/destination
// offsets, plane count, and random mask cadence without using a Genesis VDP.
//
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command -> begin_row_transfer boundary
//   0x0001D3F4 expand_random_nibble_mask -> mask callback

void render_world_sequence_tile_rows(void)
{
  // A2 is one eight-byte sequence record:
  //   +0x02 signed vertical offset, +0x04 layout flags, +0x06 tile index.
  // FFDF70 is the first row in the current source window; FFDF72 and FFDF74
  // provide the vertical cursor and horizontal source origin.
// The horizontal source delta is a 16-bit subtract, 16-bit left shift, then
// signed word add to A1; the native conversion preserves that wrap before
// exposing the resulting logical host offset.
  for (uint16_t record_row = 0; record_row < 0x20; ++record_row) {
    const uint16_t visible_row = FFDF70 + record_row;
    const uint16_t record_start = FFDF72 + record->vertical_offset;
    const uint16_t row_width = ((record->layout_flags & 3) + 1) * 8;
    if (visible_row < record_start ||
        visible_row - record_start >= row_width) {
      continue;
    }

    const uint16_t row_delta = visible_row - record_start;
    const uint16_t tile_index = record->tile_index & 0x7FF;
    const int32_t horizontal_delta =
        static_cast<int16_t>(tile_index - FFDF74);
    const uint16_t plane_count = ((record->layout_flags >> 2) & 3) + 1;
    begin_row_transfer(tile_index * 0x20 + row_delta * 4);

    const uint16_t mask_selector =
        7 - ((0x1F - record_row) >> 2);
    const uint32_t nibble_mask = expand_random_nibble_mask(mask_selector);
    for (uint16_t plane = 0; plane < plane_count; ++plane) {
      const int32_t source_byte_offset =
          horizontal_delta * 0x20 + row_delta * 4 +
          plane * row_width * 4;
      const uint16_t destination_word =
          tile_index * 0x20 + row_delta * 4 +
          plane * row_width * 4;
      const uint32_t source_word = read_source_word(source_byte_offset);
      write_word(destination_word, source_word & nibble_mask);
    }
  }
}
