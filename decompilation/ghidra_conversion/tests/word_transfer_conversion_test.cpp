#include "shadowrun_ghidra/word_transfer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto command = make_vdp_transfer_command(0xFFFFU);
    if (command.low_word != 0x7FFFU || command.high_word != 0x0003U) {
        return 1;
    }

    std::vector<std::uint16_t> commands;
    std::vector<std::uint16_t> words;
    const WordTransferCallbacks callbacks{
        [&commands](const VdpTransferCommand& transfer) {
            commands.push_back(transfer.low_word);
            commands.push_back(transfer.high_word);
        },
        [&words](const std::uint16_t word) { words.push_back(word); },
    };

    write_repeated_words(0x1234U, 0xBEEFU, 3U, callbacks);
    write_word_stream(0x8000U, {1U, 2U, 3U}, callbacks);
    clear_vdp_words(0U, callbacks);
    if (commands != std::vector<std::uint16_t>{0x5234U, 0U,
                                                0x4000U, 2U,
                                                0x4000U, 0U} ||
        words.size() != 22U || words[0] != 0xBEEFU || words[3] != 1U ||
        words[5] != 3U || words[6] != 0U || words.back() != 0U) {
        return 2;
    }

    return 0;
}
