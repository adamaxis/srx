#include "shadowrun_ghidra/word_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

VdpTransferCommand make_vdp_transfer_command(
    const std::uint16_t source_address) {
    return {
        static_cast<std::uint16_t>((source_address & 0x3FFFU) | 0x4000U),
        static_cast<std::uint16_t>(source_address >> 14U)};
}

void write_repeated_words(
    const std::uint16_t source_address,
    const std::uint16_t word,
    const std::size_t word_count,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(source_address));
    }
    if (!callbacks.write_word) {
        return;
    }
    for (std::size_t index = 0; index < word_count; ++index) {
        callbacks.write_word(word);
    }
}

void write_word_stream(
    const std::uint16_t source_address,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(source_address));
    }
    if (!callbacks.write_word) {
        return;
    }
    for (const auto word : words) {
        callbacks.write_word(word);
    }
}

void clear_vdp_words(
    const std::uint16_t source_address,
    const WordTransferCallbacks& callbacks) {
    write_repeated_words(source_address, 0U, 16U, callbacks);
}

} // namespace shadowrun::ghidra_conversion
