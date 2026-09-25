#include "shadowrun_ghidra/world_sequence_tile_render_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

WorldSequenceTileRenderResult render_world_sequence_tile_rows(
    const WorldSequenceTileRenderState& state,
    const std::vector<WorldSequenceTileRecord>& records,
    const WorldSequenceTileRenderCallbacks& callbacks) {
    WorldSequenceTileRenderResult result{};

    for (std::size_t record_index = 0; record_index < records.size();
         ++record_index) {
        const auto& record = records[record_index];
        const auto tile_index = static_cast<std::uint16_t>(
            record.tile_index & 0x07FFU);
        const auto row_width = static_cast<std::uint16_t>(
            ((record.layout_flags & 0x0003U) + 1U) * 8U);
        const auto plane_count = static_cast<std::uint16_t>(
            ((record.layout_flags >> 2U) & 0x0003U) + 1U);
        const auto record_start = static_cast<std::uint16_t>(
            state.vertical_cursor + static_cast<std::uint16_t>(
                                       record.vertical_offset));
        const auto horizontal_delta = static_cast<std::int32_t>(
            static_cast<std::int16_t>(tile_index - state.horizontal_origin));

        bool record_visible = false;
        for (std::uint16_t frame_slot = 0; frame_slot < 32U; ++frame_slot) {
            const auto visible_row = static_cast<std::uint16_t>(
                state.current_row + frame_slot);
            if (visible_row < record_start) {
                continue;
            }
            const auto row_delta = static_cast<std::uint16_t>(
                visible_row - record_start);
            if (row_delta >= row_width) {
                continue;
            }

            record_visible = true;
            ++result.transfer_rows;
            if (callbacks.begin_row_transfer) {
                callbacks.begin_row_transfer(static_cast<std::uint16_t>(
                    tile_index * 0x20U + row_delta * 4U));
            }

            const auto source_row_base = static_cast<std::int32_t>(
                horizontal_delta * 0x20 + row_delta * 4U);
            const auto destination_row_base = static_cast<std::uint16_t>(
                tile_index * 0x20U + row_delta * 4U);
            const auto source_mask_selector = static_cast<std::uint16_t>(
                7U - ((31U - frame_slot) >> 2U));
            const auto nibble_mask = callbacks.expand_random_nibble_mask
                                         ? callbacks.expand_random_nibble_mask(
                                               source_mask_selector)
                                         : 0xFFFFFFFFU;

            for (std::uint16_t plane = 0; plane < plane_count; ++plane) {
                const auto source_byte_offset = static_cast<std::int32_t>(
                    source_row_base + plane * row_width * 4U);
                const auto destination_word = static_cast<std::uint16_t>(
                    destination_row_base + plane * row_width * 4U);
                const auto source_word = callbacks.read_source_word
                                             ? callbacks.read_source_word(
                                                   source_byte_offset)
                                             : 0U;
                const auto masked_word = source_word & nibble_mask;

                if (callbacks.write_word) {
                    callbacks.write_word({record_index,
                                          visible_row,
                                          source_mask_selector,
                                          destination_word,
                                          source_byte_offset,
                                          source_word,
                                          nibble_mask,
                                          masked_word});
                }
                ++result.written_words;
            }
        }
        if (record_visible) {
            ++result.visible_records;
        }
    }
    return result;
}

} // namespace shadowrun::ghidra_conversion
