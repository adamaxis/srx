#include "shadowrun_ghidra/core/word_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

VdpTransferCommand make_vdp_transfer_command(
    const std::uint16_t source_address) {
    return {
        static_cast<std::uint16_t>((source_address & 0x3FFFU) | 0x4000U),
        static_cast<std::uint16_t>(source_address >> 14U)};
}

void write_single_word(
    const std::uint16_t destination_address,
    const std::uint16_t word,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(destination_address));
    }
    if (callbacks.write_word) {
        callbacks.write_word(word);
    }
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

void write_vdp_word_block_with_source_wrap(
    const std::uint32_t source_address,
    const std::uint16_t destination_address,
    const std::vector<std::uint16_t>& words,
    const std::size_t byte_count,
    const WordTransferCallbacks& callbacks) {
    const auto source_low = source_address & 0xFFFFU;
    const auto bytes_until_wrap = 0x10000U - source_low;
    if (source_low + byte_count <= 0x10000U || bytes_until_wrap == 0U) {
        write_vdp_word_block(destination_address, words, byte_count, callbacks);
        return;
    }

    const auto first_bytes = bytes_until_wrap < byte_count
                                  ? static_cast<std::size_t>(bytes_until_wrap)
                                  : byte_count;
    write_vdp_word_block(destination_address, words, first_bytes, callbacks);
    const auto first_words = first_bytes / 2U;
    const auto remaining_bytes = byte_count - first_bytes;
    if (first_words >= words.size()) {
        return;
    }
    const std::vector<std::uint16_t> remaining_words(
        words.begin() + static_cast<std::ptrdiff_t>(first_words), words.end());
    write_vdp_word_block(
        static_cast<std::uint16_t>(destination_address + first_bytes),
        remaining_words, remaining_bytes, callbacks);
}

void write_four_by_four_transfer_batch(
    const std::function<void()>& write_wrapped_block) {
    if (!write_wrapped_block) {
        return;
    }
    for (std::size_t row = 0; row < 4U; ++row) {
        for (std::size_t column = 0; column < 4U; ++column) {
            write_wrapped_block();
        }
    }
}

void upload_mode_word_block(
    const std::uint32_t source_address,
    const std::uint16_t destination_address,
    const std::vector<std::uint16_t>& words,
    const std::size_t byte_count,
    const WordTransferCallbacks& callbacks) {
    write_vdp_word_block_with_source_wrap(
        source_address, destination_address, words, byte_count, callbacks);
}

void upload_scene_profile_block(
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks) {
    write_vdp_word_block(0xFE00U, words, 0x140U, callbacks);
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

void write_masked_vdp_row(
    const std::uint16_t destination_address,
    const std::size_t width,
    const std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    const bool configure_display_increment,
    const WordTransferCallbacks& callbacks) {
    if (configure_display_increment && callbacks.write_control_word) {
        callbacks.write_control_word(0x8F80U);
    }
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(destination_address));
    }
    if (callbacks.write_word) {
        const auto count = width < words.size() ? width : words.size();
        for (std::size_t index = 0; index < count; ++index) {
            callbacks.write_word(static_cast<std::uint16_t>(
                words[index] | word_mask));
        }
    }
    if (configure_display_increment && callbacks.write_control_word) {
        callbacks.write_control_word(0x8F02U);
    }
}

void write_repeated_vdp_grid(
    const std::uint16_t destination_address,
    const std::size_t width,
    const std::size_t height,
    const std::uint16_t word_mask,
    const std::vector<std::uint16_t>& row_words,
    const WordTransferCallbacks& callbacks) {
    for (std::size_t row = 0; row < height; ++row) {
        const auto row_address = static_cast<std::uint16_t>(
            destination_address + row * 0x80U);
        write_masked_vdp_row(row_address, width, word_mask, row_words, false,
                             callbacks);
    }
}

void write_linear_vdp_grid(
    const std::uint16_t destination_address,
    const std::size_t width,
    const std::size_t height,
    const std::uint16_t word_mask,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks) {
    std::size_t source_index = 0;
    for (std::size_t row = 0; row < height; ++row) {
        const auto row_address = static_cast<std::uint16_t>(
            destination_address + row * 2U);
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
    if (callbacks.write_control_word) {
        callbacks.write_control_word(0x8F02U);
    }
}

void write_repeated_masked_word(
    const std::uint16_t destination_address,
    const std::uint16_t source_word,
    const std::uint16_t word_mask,
    const std::size_t word_count,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(destination_address));
    }
    if (!callbacks.write_word) {
        return;
    }
    const auto word = static_cast<std::uint16_t>(source_word | word_mask);
    for (std::size_t index = 0; index < word_count; ++index) {
        callbacks.write_word(word);
    }
}

void write_masked_transfer_with_display_control(
    const std::uint16_t destination_address,
    const std::uint16_t source_word,
    const std::uint16_t word_mask,
    const std::size_t word_count,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.write_control_word) {
        callbacks.write_control_word(0x8F80U);
    }
    write_repeated_masked_word(
        destination_address, source_word, word_mask, word_count, callbacks);
    if (callbacks.write_control_word) {
        callbacks.write_control_word(0x8F02U);
    }
}

void upload_title_or_field_word_stream(
    const std::uint16_t destination_address,
    const std::uint16_t word_mask,
    const std::size_t row_count,
    const std::size_t extra_source_words,
    const std::vector<std::uint16_t>& words,
    const WordTransferCallbacks& callbacks) {
    auto destination = destination_address;
    std::size_t source_index = 0U;
    for (std::size_t row = 0; row < row_count; ++row) {
        if (source_index >= words.size()) {
            return;
        }
        if (callbacks.begin_transfer) {
            callbacks.begin_transfer(make_vdp_transfer_command(destination));
        }
        if (callbacks.write_word) {
            callbacks.write_word(static_cast<std::uint16_t>(
                words[source_index] | word_mask));
        }
        source_index += 1U + extra_source_words;
        destination = static_cast<std::uint16_t>(destination + 0x80U);
    }
}

} // namespace shadowrun::ghidra_conversion
