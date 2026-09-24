#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

using WordPair = std::array<std::uint16_t, 2>;

struct AlternatingWordTransferState {
    std::uint32_t frame_counter = 0;
};

struct AlternatingWordTransferCallbacks {
    std::function<void()> wait_until_ready;
    std::function<void(const WordPair&)> write_words;
};

// ROM 0x1910: increment the source frame counter, choose the odd/even
// two-word buffer, wait for transfer readiness, and emit that pair.
void write_alternating_word_pair(
    AlternatingWordTransferState& state,
    const WordPair& even_pair,
    const WordPair& odd_pair,
    const AlternatingWordTransferCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
