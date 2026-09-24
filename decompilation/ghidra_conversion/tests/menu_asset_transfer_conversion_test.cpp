#include "shadowrun_ghidra/menu_asset_transfer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    MenuGraphicsState prepared{};
    std::vector<int> preparation_order;
    prepare_menu_graphics(
        prepared,
        {
            [&preparation_order](const std::uint16_t command) {
                if (command != 0x8134U) {
                    preparation_order.push_back(-1);
                }
                preparation_order.push_back(1);
            },
            [&preparation_order] { preparation_order.push_back(2); },
            [&preparation_order] { preparation_order.push_back(3); },
            [&preparation_order] { preparation_order.push_back(4); },
        });
    if (prepared.display_command != 0x8134U ||
        !prepared.resource_metadata_ready ||
        !prepared.resource_upload_requested ||
        preparation_order != std::vector<int>{1, 2, 3, 4}) {
        return 1;
    }

    MenuGraphicsState nine_block{};
    std::vector<std::uint16_t> commands;
    std::size_t uploads = 0;
    std::size_t copies = 0;
    transfer_menu_asset_batch(
        nine_block,
        {9U, true, false, true, true, false},
        {
            [&commands] { },
            [&commands](const std::uint16_t command) {
                commands.push_back(command);
            },
            [&copies] { ++copies; },
            [&uploads] { ++uploads; },
        });
    if (nine_block.uploaded_tile_blocks != 10U ||
        !nine_block.optional_tile_uploaded || uploads != 10U || copies != 0U ||
        commands != std::vector<std::uint16_t>{0x8134U, 0x8174U}) {
        return 2;
    }

    MenuGraphicsState eight_block{};
    uploads = 0;
    copies = 0;
    transfer_menu_asset_batch(
        eight_block,
        {8U, false, false, false, false, true},
        {
            [] { },
            [](const std::uint16_t) { },
            [&copies] { ++copies; },
            [&uploads] { ++uploads; },
        });
    if (eight_block.uploaded_tile_blocks != 8U || uploads != 8U ||
        copies != 1U || eight_block.optional_tile_uploaded) {
        return 3;
    }

    return 0;
}
