#include "shadowrun_ghidra/world/world_sequence_tile_render_conversion.hpp"

#include <cstdint>
#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<WorldSequenceTileWrite> writes;
    std::vector<std::uint16_t> transfer_rows;
    const auto result = render_world_sequence_tile_rows(
        {3U, 0U, 100U},
        {{0, 0x0004U, 101U}, {20, 0x0000U, 100U}},
        { [&](const std::uint16_t destination_word) {
              transfer_rows.push_back(destination_word);
          },
          [](const std::int32_t source_byte_offset) {
              return static_cast<std::uint32_t>(source_byte_offset + 0x1000);
          },
          [](const std::uint16_t selector) {
              return selector == 0U ? 0x0F0F0F0FU : 0x00FF00FFU;
          },
          [&](const WorldSequenceTileWrite& write) { writes.push_back(write); } });

    // Record 0 spans rows 3..10 and has two planes. Record 1 spans rows
    // 20..27 and has one plane, for sixteen transfer rows and 24 words.
    // Record 0 has rows 3..7 remaining in the eight-row window and two planes.
    // Record 1 spans rows 20..27 and has one plane, for thirteen transfer rows
    // and eighteen words.
    if (expect(result.visible_records == 2U && result.transfer_rows == 13U &&
               result.written_words == 18U && transfer_rows.size() == 13U &&
               writes.size() == 18U) != 0) {
        return 1;
    }
    if (expect(writes.front().record_index == 0U &&
               writes.front().visible_row == 3U &&
               writes.front().random_mask_selector == 0U &&
               writes.front().destination_word == 0x0CACU &&
               writes.front().source_byte_offset == 0x2CU &&
               writes.front().masked_word ==
                   ((0x102CU) & 0x0F0F0F0FU)) != 0) {
        return 2;
    }
    if (expect(writes[8].record_index == 0U &&
               writes[8].visible_row == 7U &&
               writes[8].random_mask_selector == 1U &&
               writes[8].nibble_mask == 0x00FF00FFU) != 0) {
        return 3;
    }
    if (expect(writes.back().record_index == 1U &&
               writes.back().visible_row == 27U &&
               writes.back().destination_word == 0x0C9CU) != 0) {
        return 4;
    }

    return 0;
}
