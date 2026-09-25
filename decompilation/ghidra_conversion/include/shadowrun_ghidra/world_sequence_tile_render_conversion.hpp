#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WorldSequenceTileRenderState {
    std::uint16_t current_row = 0;       // ROM FFDF70
    std::uint16_t vertical_cursor = 0;   // ROM FFDF72
    std::uint16_t horizontal_origin = 0; // ROM FFDF74
};

struct WorldSequenceTileRecord {
    std::int16_t vertical_offset = 0; // record +0x02
    std::uint16_t layout_flags = 0;   // record +0x04
    std::uint16_t tile_index = 0;     // record +0x06, low 11 bits used
};

struct WorldSequenceTileWrite {
    std::size_t record_index = 0;
    std::uint16_t visible_row = 0;
    std::uint16_t random_mask_selector = 0;
    std::uint16_t destination_word = 0;
    std::int32_t source_byte_offset = 0;
    std::uint32_t source_word = 0;
    std::uint32_t nibble_mask = 0;
    std::uint32_t masked_word = 0;
};

struct WorldSequenceTileRenderCallbacks {
    // ROM 0x00000E9C: begin the source transfer for one visible row.
    std::function<void(std::uint16_t destination_word)> begin_row_transfer;
    // Host asset storage supplies source words by logical byte offset.
    std::function<std::uint32_t(std::int32_t source_byte_offset)>
        read_source_word;
    // ROM 0x1D3F4: expand the eight-nibble random mask selected by the
    // source row cadence.
    std::function<std::uint32_t(std::uint16_t mask_selector)>
        expand_random_nibble_mask;
    std::function<void(const WorldSequenceTileWrite&)> write_word;
};

struct WorldSequenceTileRenderResult {
    std::size_t visible_records = 0;
    std::size_t transfer_rows = 0;
    std::size_t written_words = 0;
};

// ROM 0x1D33A: render each visible world-sequence record across the 32-row
// source window. This emits host tile words instead of touching the Genesis
// VDP address; source offsets, row visibility, plane counts, and mask cadence
// remain source-derived and independently testable.
WorldSequenceTileRenderResult render_world_sequence_tile_rows(
    const WorldSequenceTileRenderState& state,
    const std::vector<WorldSequenceTileRecord>& records,
    const WorldSequenceTileRenderCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
