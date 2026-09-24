#include "shadowrun_ghidra/startup_vdp_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_vdp_upload_state(
    VdpUploadState& state,
    const std::uint16_t transfer_address,
    const WordTransferCallbacks& callbacks) {
    state.transfer_address = transfer_address;
    state.source_stream_cursor = 0U;
    state.transfer_enabled = true;
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(transfer_address));
    }
    if (callbacks.write_word) {
        for (std::size_t word = 0; word < 4U; ++word) {
            callbacks.write_word(0U);
        }
    }
}

void upload_startup_palette(
    const std::array<std::uint16_t, 64>& words,
    const RawVdpWriteCallbacks& callbacks) {
    for (std::size_t index = 0; index < words.size(); ++index) {
        if (callbacks.write_control_word) {
            callbacks.write_control_word(static_cast<std::uint16_t>(
                0xC000U + index * 2U));
            callbacks.write_control_word(0U);
        }
        if (callbacks.write_data_word) {
            callbacks.write_data_word(words[index]);
        }
    }
}

void write_transfer_header(
    const std::uint16_t source_address,
    const std::uint16_t first_word,
    const std::uint16_t second_word,
    const WordTransferCallbacks& callbacks) {
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(source_address));
    }
    if (callbacks.write_word) {
        callbacks.write_word(first_word);
        callbacks.write_word(second_word);
    }
}

void write_fixed_transfer_pair(
    const std::uint16_t first_word,
    const std::uint16_t second_word,
    const RawVdpWriteCallbacks& callbacks) {
    if (callbacks.write_control_long) {
        callbacks.write_control_long(0x40000010UL);
    }
    if (callbacks.write_data_word) {
        callbacks.write_data_word(first_word);
        callbacks.write_data_word(second_word);
    }
}

void clear_and_fill_vdp_plane(
    const std::uint16_t plane_address,
    const WordTransferCallbacks& callbacks) {
    clear_vdp_words(static_cast<std::uint16_t>(plane_address - 0x20U),
                    callbacks);
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(plane_address));
    }
    if (!callbacks.write_word) {
        return;
    }
    for (std::size_t write = 0; write < 0x400U; ++write) {
        callbacks.write_word(0x07FFU);
        callbacks.write_word(0x07FFU);
    }
}

} // namespace shadowrun::ghidra_conversion
