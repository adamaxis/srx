#include "shadowrun_ghidra/startup_vdp_conversion.hpp"

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

    return 0;
}
