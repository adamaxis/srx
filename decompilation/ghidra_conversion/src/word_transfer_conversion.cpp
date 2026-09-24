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

void write_vdp_word_block(
    const std::uint16_t destination_address,
    const std::vector<std::uint16_t>& words,
    const std::size_t byte_count,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(destination_address));
    }
    if (!callbacks.write_word) {
        return;
    }
    const auto requested_words = byte_count / 2U;
    const auto word_count =
        requested_words < words.size() ? requested_words : words.size();
    for (std::size_t index = 0; index < word_count; ++index) {
        callbacks.write_word(words[index]);
    }
}

void write_vdp_grid(
    const std::uint16_t destination_address,
    const std::size_t width,
    const std::size_t height,
    const std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks) {
    std::size_t source_index = 0;
    for (std::size_t row = 0; row < height; ++row) {
        const auto row_address = static_cast<std::uint16_t>(
            destination_address + row * 0x80U);
        if (callbacks.begin_transfer) {
            callbacks.begin_transfer(make_vdp_transfer_command(row_address));
        }
        for (std::size_t column = 0; column < width; ++column) {
            if (source_index >= words.size()) {
                return;
            }
            if (callbacks.write_word) {
                callbacks.write_word(static_cast<std::uint16_t>(
                    words[source_index] | word_mask));
            }
            ++source_index;
        }
    }
}

} // namespace shadowrun::ghidra_conversion
