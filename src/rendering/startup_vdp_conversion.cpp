#include "shadowrun_ghidra/rendering/startup_vdp_conversion.hpp"

namespace shadowrun::ghidra_conversion {

PendingVdpUploadAction service_pending_vdp_upload(
    PendingVdpUploadState& state,
    const VdpUploadBoundary& write_block,
    const VdpUploadBoundary& reset_upload) {
    if (state.transfer_count > 0x4FU) {
        state.source_flag = 0U;
    }
    if (state.transfer_count != 0xFFFFU) {
        if (write_block) {
            write_block();
        }
        return PendingVdpUploadAction::stream_block;
    }
    if (reset_upload) {
        reset_upload();
    }
    return PendingVdpUploadAction::reset_upload;
}

void install_startup_vdp_table(
    StartupVdpTable& state,
    const std::array<std::uint16_t, 18>& source_words,
    const RawVdpWriteCallbacks& callbacks) {
    for (std::size_t index = 0; index < state.transfer_words.size(); ++index) {
        state.transfer_words[index] = source_words[index];
    }
    for (std::size_t index = 0; index < state.control_words.size(); ++index) {
        state.control_words[index] = source_words[index + 4U];
        if (callbacks.write_control_word) {
            callbacks.write_control_word(state.control_words[index]);
        }
    }
}

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

std::size_t write_startup_text_to_vdp(
    const std::uint8_t column,
    const std::uint8_t row,
    const std::vector<std::uint8_t>& text,
    const WordTransferCallbacks& callbacks) {
    const auto destination = static_cast<std::uint16_t>(
        kStartupTextPlane + (static_cast<std::uint16_t>(row) << 7U) +
        (static_cast<std::uint16_t>(column) << 1U));
    if (callbacks.begin_transfer) {
        callbacks.begin_transfer(make_vdp_transfer_command(destination));
    }
    std::size_t written = 0U;
    for (const auto glyph : text) {
        if (glyph == 0U) {
            break;
        }
        if (callbacks.write_word) {
            callbacks.write_word(static_cast<std::uint16_t>(
                static_cast<std::uint8_t>(glyph - 0x20U)));
        }
        ++written;
    }
    return written;
}

void clear_runtime_memory_and_vdp(
    RuntimeMemoryAndVdpState& state,
    const std::array<std::uint16_t, 64>& palette,
    const RuntimeMemoryAndVdpCallbacks& callbacks) {
    if (state.reset_magic == kRuntimeResetMagic) {
        if (callbacks.clear_warm_ram) {
            callbacks.clear_warm_ram(kWarmRamByteCount);
        }
    } else if (callbacks.clear_cold_ram) {
        state.checksum_seed = callbacks.clear_cold_ram(kColdRamLongCount);
    }

    upload_startup_palette(palette, callbacks.vdp);
    write_repeated_words(
        0U, kBootVramFillWord, kBootVramFillCount, callbacks.transfer);
    state.upload_marker = 1U;
    if (callbacks.reset_vdp_upload_state) {
        callbacks.reset_vdp_upload_state();
    }

    StartupVdpTable table{};
    install_startup_vdp_table(table, kStartupVdpTableWords, callbacks.vdp);

    if (callbacks.transfer.begin_transfer) {
        callbacks.transfer.begin_transfer(
            make_vdp_transfer_command(kPostTableTransferDestination));
    }
    if (callbacks.transfer.write_word) {
        for (std::size_t index = 0; index < kPostTableZeroWordCount; ++index) {
            callbacks.transfer.write_word(0U);
        }
    }
    if (callbacks.vdp.write_control_long) {
        callbacks.vdp.write_control_long(kBootVscrollWriteCommand);
    }
    if (callbacks.vdp.write_data_word) {
        callbacks.vdp.write_data_word(0U);
        callbacks.vdp.write_data_word(0U);
    }
    if (callbacks.initialize_title_audio_assets) {
        for (std::size_t index = 0; index < kTitleAudioInitCount; ++index) {
            callbacks.initialize_title_audio_assets();
        }
    }
}

} // namespace shadowrun::ghidra_conversion
