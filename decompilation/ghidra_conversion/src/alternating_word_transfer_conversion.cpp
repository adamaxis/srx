#include "shadowrun_ghidra/alternating_word_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void write_alternating_word_pair(
    AlternatingWordTransferState& state,
    const WordPair& even_pair,
    const WordPair& odd_pair,
    const AlternatingWordTransferCallbacks& callbacks) {
    ++state.frame_counter;
    const auto& selected_pair = (state.frame_counter & 1U) == 0U
                                    ? even_pair
                                    : odd_pair;
    if (callbacks.wait_until_ready) {
        callbacks.wait_until_ready();
    }
    if (callbacks.write_words) {
        callbacks.write_words(selected_pair);
    }
}

} // namespace shadowrun::ghidra_conversion
