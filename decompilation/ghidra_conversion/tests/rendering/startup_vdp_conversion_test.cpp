#include "shadowrun_ghidra/rendering/startup_vdp_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::array<std::uint16_t, 64> palette{};
    palette.front() = 0x1234U;
    palette.back() = 0x5678U;
    std::vector<std::uint16_t> controls;
    std::vector<std::uint16_t> palette_words;
    upload_startup_palette(
        palette,
        {[&controls](const std::uint16_t word) { controls.push_back(word); },
         {},
         [&palette_words](const std::uint16_t word) {
             palette_words.push_back(word);
         }});
    if (controls.size() != 128U || controls[0] != 0xC000U ||
        controls[126] != 0xC07EU || palette_words.size() != 64U ||
        palette_words.front() != 0x1234U || palette_words.back() != 0x5678U) {
        return 1;
    }

    StartupVdpTable table{};
    std::array<std::uint16_t, 18> source_table{};
    for (std::size_t index = 0; index < source_table.size(); ++index) {
        source_table[index] = static_cast<std::uint16_t>(0x100U + index);
    }
    controls.clear();
    install_startup_vdp_table(table, source_table,
                              { [&controls](const std::uint16_t word) {
                                    controls.push_back(word);
                                },
                                {},
                                {} });
    if (table.transfer_words[0] != 0x100U ||
        table.transfer_words[3] != 0x103U ||
        table.control_words[0] != 0x104U ||
        table.control_words.back() != 0x111U || controls.size() != 14U ||
        controls.back() != 0x111U) {
        return 2;
    }

    PendingVdpUploadState pending{0x0050U, 7U};
    std::vector<int> pending_order;
    if (service_pending_vdp_upload(
            pending,
            [&pending_order] { pending_order.push_back(1); },
            [&pending_order] { pending_order.push_back(2); }) !=
            PendingVdpUploadAction::stream_block ||
        pending.source_flag != 0U || pending_order != std::vector<int>{1}) {
        return 3;
    }
    pending = {0xFFFFU, 9U};
    pending_order.clear();
    if (service_pending_vdp_upload(
            pending,
            [&pending_order] { pending_order.push_back(1); },
            [&pending_order] { pending_order.push_back(2); }) !=
            PendingVdpUploadAction::reset_upload ||
        pending.source_flag != 0U || pending_order != std::vector<int>{2}) {
        return 4;
    }

    std::vector<VdpTransferCommand> commands;
    std::vector<std::uint16_t> words;
    const WordTransferCallbacks transfer_callbacks{
        [&commands](const VdpTransferCommand& command) {
            commands.push_back(command);
        },
        [&words](const std::uint16_t word) { words.push_back(word); },
    };
    VdpUploadState upload_state{};
    commands.clear();
    words.clear();
    reset_vdp_upload_state(upload_state, 0x2345U, transfer_callbacks);
    if (upload_state.transfer_address != 0x2345U ||
        upload_state.source_stream_cursor != 0U ||
        !upload_state.transfer_enabled || commands.size() != 1U ||
        words != std::vector<std::uint16_t>{0U, 0U, 0U, 0U}) {
        return 5;
    }
    commands.clear();
    words.clear();
    write_transfer_header(0x1234U, 0xAAAAU, 0xBBBBU, transfer_callbacks);
    if (commands.size() != 1U || commands[0].low_word != 0x5234U ||
        words != std::vector<std::uint16_t>{0xAAAAU, 0xBBBBU}) {
        return 6;
    }

    std::uint32_t command_long = 0U;
    std::vector<std::uint16_t> pair;
    write_fixed_transfer_pair(
        0x1111U, 0x2222U,
        {{}, [&command_long](const std::uint32_t value) {
             command_long = value;
         },
         [&pair](const std::uint16_t value) { pair.push_back(value); }});
    if (command_long != 0x40000010UL ||
        pair != std::vector<std::uint16_t>{0x1111U, 0x2222U}) {
        return 7;
    }

    commands.clear();
    words.clear();
    clear_and_fill_vdp_plane(0x2000U, transfer_callbacks);
    if (commands.size() != 2U || words.size() != 16U + 2048U ||
        commands[0].low_word !=
            make_vdp_transfer_command(0x1FE0U).low_word ||
        commands[1].low_word != make_vdp_transfer_command(0x2000U).low_word ||
        words[16] != 0x07FFU || words.back() != 0x07FFU) {
        return 8;
    }

    commands.clear();
    words.clear();
    const auto written = write_startup_text_to_vdp(
        2U, 1U, {0x41U, 0x42U, 0U}, transfer_callbacks);
    const auto text_command = make_vdp_transfer_command(0xC084U);
    if (written != 2U || commands.size() != 1U ||
        commands[0].low_word != text_command.low_word ||
        commands[0].high_word != text_command.high_word ||
        words != std::vector<std::uint16_t>{0x21U, 0x22U}) {
        return 9;
    }
    commands.clear();
    words.clear();
    if (write_startup_text_to_vdp(0U, 0U, {0U}, transfer_callbacks) != 0U ||
        commands.size() != 1U ||
        commands[0].low_word !=
            make_vdp_transfer_command(kStartupTextPlane).low_word ||
        !words.empty()) {
        return 10;
    }

    RuntimeMemoryAndVdpState cold{};
    std::array<std::uint16_t, 64> boot_palette{};
    boot_palette.front() = 0x0EEEU;
    std::size_t cold_longs = 0U;
    std::size_t warm_bytes = 0U;
    std::size_t fill_words = 0U;
    std::size_t audio_inits = 0U;
    std::size_t upload_resets = 0U;
    std::vector<std::uint16_t> boot_controls;
    std::vector<std::uint16_t> boot_data;
    std::vector<std::uint32_t> boot_longs;
    std::vector<VdpTransferCommand> boot_commands;
    const RuntimeMemoryAndVdpCallbacks boot_callbacks{
        [&cold_longs](const std::size_t count) {
            cold_longs = count;
            return static_cast<std::uint16_t>(0xABCDU);
        },
        [&warm_bytes](const std::size_t count) { warm_bytes = count; },
        {[&boot_controls](const std::uint16_t word) {
             boot_controls.push_back(word);
         },
         [&boot_longs](const std::uint32_t value) {
             boot_longs.push_back(value);
         },
         [&boot_data](const std::uint16_t word) { boot_data.push_back(word); }},
        {[&boot_commands](const VdpTransferCommand& command) {
             boot_commands.push_back(command);
         },
         [&fill_words](const std::uint16_t) { ++fill_words; }},
        [&upload_resets] { ++upload_resets; },
        [&audio_inits] { ++audio_inits; },
    };
    clear_runtime_memory_and_vdp(cold, boot_palette, boot_callbacks);
    const auto fill_command = make_vdp_transfer_command(0U);
    const auto post_command =
        make_vdp_transfer_command(kPostTableTransferDestination);
    if (cold_longs != kColdRamLongCount || warm_bytes != 0U ||
        cold.checksum_seed != 0xABCDU || cold.upload_marker != 1U ||
        fill_words != kBootVramFillCount + kPostTableZeroWordCount ||
        audio_inits != kTitleAudioInitCount || upload_resets != 1U ||
        boot_commands.size() != 2U ||
        boot_commands[0].low_word != fill_command.low_word ||
        boot_commands[1].low_word != post_command.low_word ||
        boot_longs != std::vector<std::uint32_t>{kBootVscrollWriteCommand} ||
        boot_data.size() != 66U || boot_data.front() != 0x0EEEU ||
        boot_data[64] != 0U || boot_data[65] != 0U ||
        boot_controls.size() != 128U + 14U ||
        boot_controls[128] != 0x8230U) {
        return 11;
    }

    RuntimeMemoryAndVdpState warm{};
    warm.reset_magic = kRuntimeResetMagic;
    warm.checksum_seed = 0x1111U;
    cold_longs = 0U;
    warm_bytes = 0U;
    fill_words = 0U;
    audio_inits = 0U;
    clear_runtime_memory_and_vdp(warm, boot_palette, boot_callbacks);
    if (cold_longs != 0U || warm_bytes != kWarmRamByteCount ||
        warm.checksum_seed != 0x1111U || warm.upload_marker != 1U ||
        audio_inits != kTitleAudioInitCount) {
        return 12;
    }

    return 0;
}
