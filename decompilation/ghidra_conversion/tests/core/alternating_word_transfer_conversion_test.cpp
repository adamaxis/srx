#include "shadowrun_ghidra/core/alternating_word_transfer_conversion.hpp"

#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const WordPair even{0x1000U, 0x1001U};
    const WordPair odd{0x2000U, 0x2001U};
    AlternatingWordTransferState state{0U};
    std::vector<int> order;
    std::vector<WordPair> writes;
    const AlternatingWordTransferCallbacks callbacks{
        [&order] { order.push_back(1); },
        [&order, &writes](const WordPair& pair) {
            order.push_back(2);
            writes.push_back(pair);
        }};

    write_alternating_word_pair(state, even, odd, callbacks);
    write_alternating_word_pair(state, even, odd, callbacks);
    if (state.frame_counter != 2U || order != std::vector<int>{1, 2, 1, 2} ||
        writes != std::vector<WordPair>{odd, even}) {
        return 1;
    }

    return 0;
}
