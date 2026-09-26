#include "shadowrun_ghidra/core/word_transfer_conversion.hpp"

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

    write_single_word(0x2468U, 0xCAFEU, callbacks);
    if (commands != std::vector<std::uint16_t>{
                         make_vdp_transfer_command(0x2468U).low_word,
                         make_vdp_transfer_command(0x2468U).high_word} ||
        words != std::vector<std::uint16_t>{0xCAFEU}) {
        return 2;
    }
    commands.clear();
    words.clear();

    write_repeated_words(0x1234U, 0xBEEFU, 3U, callbacks);
    write_word_stream(0x8000U, {1U, 2U, 3U}, callbacks);
    clear_vdp_words(0U, callbacks);
    if (commands != std::vector<std::uint16_t>{0x5234U, 0U,
                                                0x4000U, 2U,
                                                0x4000U, 0U} ||
        words.size() != 22U || words[0] != 0xBEEFU || words[3] != 1U ||
        words[5] != 3U || words[6] != 0U || words.back() != 0U) {
        return 3;
    }

    commands.clear();
    words.clear();
    write_vdp_word_block(0x8000U, {10U, 20U, 30U, 40U}, 6U, callbacks);
    if (commands.size() != 2U || commands[0] != 0x4000U ||
        commands[1] != 2U ||
        words != std::vector<std::uint16_t>{10U, 20U, 30U}) {
        return 4;
    }

    commands.clear();
    words.clear();
    write_vdp_grid(0x1000U, 2U, 2U, 0x8000U,
                   {1U, 2U, 3U, 4U}, callbacks);
    if (commands.size() != 4U ||
        commands[0] != make_vdp_transfer_command(0x1000U).low_word ||
        commands[2] != make_vdp_transfer_command(0x1080U).low_word ||
        words != std::vector<std::uint16_t>{0x8001U, 0x8002U,
                                             0x8003U, 0x8004U}) {
        return 5;
    }

    commands.clear();
    words.clear();
    std::vector<std::uint16_t> wrapped_words(8U, 0U);
    for (std::size_t index = 0; index < wrapped_words.size(); ++index) {
        wrapped_words[index] = static_cast<std::uint16_t>(index + 1U);
    }
    write_vdp_word_block_with_source_wrap(
        0xFFF8U, 0x2000U, wrapped_words, 16U, callbacks);
    if (commands.size() != 4U ||
        commands[0] != make_vdp_transfer_command(0x2000U).low_word ||
        commands[2] != make_vdp_transfer_command(0x2008U).low_word ||
        words != wrapped_words) {
        return 6;
    }

    commands.clear();
    words.clear();
    upload_scene_profile_block(std::vector<std::uint16_t>(160U, 0x55AAU),
                               callbacks);
    if (commands.size() != 2U ||
        commands[0] != make_vdp_transfer_command(0xFE00U).low_word ||
        words.size() != 160U || words.front() != 0x55AAU ||
        words.back() != 0x55AAU) {
        return 7;
    }

    commands.clear();
    words.clear();
    std::vector<std::uint16_t> controls;
    const WordTransferCallbacks row_callbacks{
        [&commands](const VdpTransferCommand& transfer) {
            commands.push_back(transfer.low_word);
            commands.push_back(transfer.high_word);
        },
        [&words](const std::uint16_t word) { words.push_back(word); },
        [&controls](const std::uint16_t word) { controls.push_back(word); },
    };
    write_masked_vdp_row(0x3000U, 3U, 0x4000U, {1U, 2U, 3U, 4U}, true,
                         row_callbacks);
    if (commands.size() != 2U || words != std::vector<std::uint16_t>{0x4001U,
                                                                        0x4002U,
                                                                        0x4003U} ||
        controls != std::vector<std::uint16_t>{0x8F80U, 0x8F02U}) {
        return 8;
    }

    commands.clear();
    words.clear();
    controls.clear();
    write_repeated_vdp_grid(0x4000U, 2U, 2U, 0x2000U, {5U, 6U},
                            row_callbacks);
    if (commands.size() != 4U ||
        commands[0] != make_vdp_transfer_command(0x4000U).low_word ||
        commands[2] != make_vdp_transfer_command(0x4080U).low_word ||
        words != std::vector<std::uint16_t>{0x2005U, 0x2006U,
                                             0x2005U, 0x2006U}) {
        return 9;
    }

    commands.clear();
    words.clear();
    controls.clear();
    write_linear_vdp_grid(0x5000U, 2U, 2U, 0x1000U,
                          {7U, 8U, 9U, 10U}, row_callbacks);
    if (commands.size() != 4U ||
        commands[0] != make_vdp_transfer_command(0x5000U).low_word ||
        commands[2] != make_vdp_transfer_command(0x5002U).low_word ||
        words != std::vector<std::uint16_t>{0x1007U, 0x1008U,
                                             0x1009U, 0x100AU} ||
        controls != std::vector<std::uint16_t>{0x8F02U}) {
        return 10;
    }

    std::size_t transfer_blocks = 0U;
    write_four_by_four_transfer_batch([&transfer_blocks] {
        ++transfer_blocks;
    });
    if (transfer_blocks != 16U) {
        return 11;
    }

    commands.clear();
    words.clear();
    upload_mode_word_block(0x0000U, 0x6000U, {0x1111U, 0x2222U}, 4U,
                           row_callbacks);
    if (commands.size() != 2U || words !=
            std::vector<std::uint16_t>{0x1111U, 0x2222U}) {
        return 12;
    }

    return 0;
}
